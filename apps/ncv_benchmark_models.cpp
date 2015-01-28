#include "libnanocv/nanocv.h"
#include "libnanocv/sampler.h"
#include "libnanocv/accumulator.h"
#include "libnanocv/util/timer.h"
#include "libnanocv/util/logger.h"
#include "libnanocv/util/thread_pool.h"
#include "libnanocv/util/tabulator.h"
#include "libnanocv/tasks/task_syn_dots.h"
#include <boost/program_options.hpp>

int main(int argc, char *argv[])
{
        ncv::init();

        using namespace ncv;

        // parse the command line
        boost::program_options::options_description po_desc("", 160);
        po_desc.add_options()("help,h", "benchmark models");
        po_desc.add_options()("samples,s",
                boost::program_options::value<size_t>()->default_value(10000),
                "number of samples to use [1000, 100000]");
        po_desc.add_options()("forward",
                "evaluate the \'forward\' pass (output)");
        po_desc.add_options()("backward",
                "evaluate the \'backward' pass (gradient)");

        boost::program_options::variables_map po_vm;
        boost::program_options::store(
                boost::program_options::command_line_parser(argc, argv).options(po_desc).run(),
                po_vm);
        boost::program_options::notify(po_vm);

        // check arguments and options
        if (	po_vm.empty() ||
                po_vm.count("help"))
        {
                std::cout << po_desc;
                return EXIT_FAILURE;
        }

        const size_t cmd_samples = math::clamp(po_vm["samples"].as<size_t>(), 1000, 100 * 1000);
        const bool cmd_forward = po_vm.count("forward");
        const bool cmd_backward = po_vm.count("backward");

        if (!cmd_forward && !cmd_backward)
        {
                std::cout << po_desc;
                return EXIT_FAILURE;
        }

        const size_t cmd_rows = 28;
        const size_t cmd_cols = 28;
        const size_t cmd_outputs = 10;
        const size_t cmd_min_nthreads = 1;
        const size_t cmd_max_nthreads = ncv::n_threads();

        syn_dots_task_t task("rows=" + text::to_string(cmd_rows) + "," +
                             "cols=" + text::to_string(cmd_cols) + "," +
                             "dims=" + text::to_string(cmd_outputs) + "," +
                             "color=luma" + "," +
                             "size=" + text::to_string(cmd_samples));
        task.load("");

        const string_t lmodel0;
        const string_t lmodel1 = lmodel0 + "linear:dims=100;act-snorm;";
        const string_t lmodel2 = lmodel1 + "linear:dims=100;act-snorm;";
        const string_t lmodel3 = lmodel2 + "linear:dims=100;act-snorm;";
        const string_t lmodel4 = lmodel3 + "linear:dims=100;act-snorm;";
        const string_t lmodel5 = lmodel4 + "linear:dims=100;act-snorm;";
        
        string_t cmodel100;
        cmodel100 = cmodel100 + "conv:dims=16,rows=9,cols=9,mask=100;act-snorm;pool-max;";
        cmodel100 = cmodel100 + "conv:dims=32,rows=5,cols=5,mask=100;act-snorm;pool-max;";
        cmodel100 = cmodel100 + "conv:dims=64,rows=3,cols=3,mask=100;act-snorm;";

        string_t cmodel50;
        cmodel50 = cmodel50 + "conv:dims=16,rows=9,cols=9,mask=50;act-snorm;pool-max;";
        cmodel50 = cmodel50 + "conv:dims=32,rows=5,cols=5,mask=50;act-snorm;pool-max;";
        cmodel50 = cmodel50 + "conv:dims=64,rows=3,cols=3,mask=50;act-snorm;";

        string_t cmodel25;
        cmodel25 = cmodel25 + "conv:dims=16,rows=9,cols=9,mask=25;act-snorm;pool-max;";
        cmodel25 = cmodel25 + "conv:dims=32,rows=5,cols=5,mask=25;act-snorm;pool-max;";
        cmodel25 = cmodel25 + "conv:dims=64,rows=3,cols=3,mask=25;act-snorm;";

        const string_t outlayer = "linear:dims=" + text::to_string(cmd_outputs) + ";";

        strings_t cmd_networks =
        {
                lmodel0 + outlayer,
                lmodel1 + outlayer,
                lmodel2 + outlayer,
                lmodel3 + outlayer,
                lmodel4 + outlayer,
                lmodel5 + outlayer,

                cmodel100 + outlayer,
                cmodel50 + outlayer,
                cmodel25 + outlayer
        };

        strings_t cmd_names =
        {
                "lmodel0",
                "lmodel1",
                "lmodel2",
                "lmodel3",
                "lmodel4",
                "lmodel5",

                "cmodel100",
                "cmodel50",
                "cmodel25"
        };

        const rloss_t loss = loss_manager_t::instance().get("logistic");
        assert(loss);

        // construct tables to compare models
        tabulator_t ftable("model-forward\\threads");
        tabulator_t btable("model-backward\\threads");

        for (size_t nthreads = cmd_min_nthreads; nthreads <= cmd_max_nthreads; nthreads ++)
        {
                ftable.header() << (text::to_string(nthreads) + "xCPU [ms]");
                btable.header() << (text::to_string(nthreads) + "xCPU [ms]");
        }

        // evaluate models
        for (size_t im = 0; im < cmd_networks.size(); im ++)
        {
                const string_t cmd_network = cmd_networks[im];
                const string_t cmd_name = cmd_names[im];

                tabulator_t::row_t& frow = ftable.append(cmd_name);
                tabulator_t::row_t& brow = btable.append(cmd_name);

                log_info() << "<<< running network [" << cmd_network << "] ...";

                // create feed-forward network
                const rmodel_t model = model_manager_t::instance().get("forward-network", cmd_network);
                assert(model);
                model->resize(task, true);

                // select random samples
                samples_t samples;
                {
                        const ncv::timer_t timer;

                        sampler_t sampler(task);
                        sampler.setup(sampler_t::stype::uniform, cmd_samples);
                        sampler.setup(sampler_t::atype::annotated);

                        samples = sampler.get();

                        log_info() << "<<< selected [" << samples.size()
                                   << "] random samples in " << timer.elapsed() << ".";
                }

                // simulate parameter loading & saving
                {
                        const ncv::timer_t timer;

                        vector_t params(model->psize());
                        vector_t paramsx(model->psize());

                        const size_t tests = 128;
                        for (size_t t = 0; t < tests; t ++)
                        {
                                params.setRandom();

                                model->load_params(params);
                                model->save_params(paramsx);

                                // also check parameter loading & saving
                                const scalar_t eps = 1e-12;
                                const scalar_t err = (params - paramsx).lpNorm<Eigen::Infinity>();
                                if (err > eps)
                                {
                                        log_error() << "loading & saving parameters failed: error = "
                                                    << err << "/" << eps << "!";
                                }
                        }

                        log_info() << "<<< loaded & saved " << tests << " x " << model->psize()
                                   << " parameters in " << timer.elapsed() << ".";
                }

                // process the samples
                for (size_t nthreads = cmd_min_nthreads; nthreads <= cmd_max_nthreads; nthreads ++)
                {
                        if (cmd_forward)
                        {
                                accumulator_t ldata(*model, nthreads, "l2n-reg", criterion_t::type::value, 0.1);

                                const ncv::timer_t timer;
                                ldata.update(task, samples, *loss);

                                log_info() << "<<< processed [" << ldata.count()
                                           << "] forward samples in " << timer.elapsed() << ".";

                                frow << timer.miliseconds();
                        }

                        if (cmd_backward)
                        {
                                accumulator_t gdata(*model, nthreads, "l2n-reg", criterion_t::type::vgrad, 0.1);

                                const ncv::timer_t timer;
                                gdata.update(task, samples, *loss);

                                log_info() << "<<< processed [" << gdata.count()
                                           << "] backward samples in " << timer.elapsed() << ".";

                                brow << timer.miliseconds();
                        }
                }

                log_info();
        }

        // print results
        if (cmd_forward)
        {
                ftable.print(std::cout);
        }
        log_info();
        if (cmd_backward)
        {
                btable.print(std::cout);
        }

        // OK
        log_info() << done;
        return EXIT_SUCCESS;
}