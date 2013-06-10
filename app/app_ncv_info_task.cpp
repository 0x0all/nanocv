#include "ncv.h"
#include <boost/program_options.hpp>

int main(int argc, char *argv[])
{
        ncv::init();

        const ncv::strings_t task_ids = ncv::task_manager_t::instance().ids();

        // parse the command line
        boost::program_options::options_description po_desc("", 160);
        po_desc.add_options()("help,h", "help message");
        po_desc.add_options()("task",
                boost::program_options::value<ncv::string_t>(),
                ("tasks to choose from: " + ncv::text::concatenate(task_ids, ", ")).c_str());
        po_desc.add_options()("task-dir",
                boost::program_options::value<ncv::string_t>(),
                "directory to load task data from");
	
        boost::program_options::variables_map po_vm;
        boost::program_options::store(
                boost::program_options::command_line_parser(argc, argv).options(po_desc).run(),
                po_vm);
        boost::program_options::notify(po_vm);
        		
        // check arguments and options
        if (	po_vm.empty() ||
                !po_vm.count("task") ||
                !po_vm.count("task-dir") ||
                po_vm.count("help"))
        {
                std::cout << po_desc;
                return EXIT_FAILURE;
        }

        const ncv::string_t cmd_task = po_vm["task"].as<ncv::string_t>();
        const ncv::string_t cmd_task_dir = po_vm["task-dir"].as<ncv::string_t>();

        ncv::timer_t timer;

        // create task
        ncv::rtask_t rtask = ncv::task_manager_t::instance().get(cmd_task, "");
        if (!rtask)
        {
                ncv::log_error() << "<<< failed to load task <" << cmd_task << ">!";
                return EXIT_FAILURE;
        }

        // load task data
        timer.start();
        if (!rtask->load(cmd_task_dir))
        {
                ncv::log_error() << "<<< failed to load task <" << cmd_task
                                 << "> from directory <" << cmd_task_dir << ">!";
                return EXIT_FAILURE;
        }
        else
        {
                ncv::log_info() << "<<< loaded task in " << timer.elapsed() << ".";
        }

        // describe task
        ncv::log_info() << "images: " << rtask->n_images() << ".";
        ncv::log_info() << "sample: #rows = " << rtask->n_rows()
                        << ", #cols = " << rtask->n_cols()
                        << ", #outputs = " << rtask->n_outputs()
                        << ", #folds = " << rtask->n_folds() << ".";

        for (ncv::size_t f = 0; f < rtask->n_folds(); f ++)
        {
                const ncv::fold_t train_fold = std::make_pair(f, ncv::protocol::train);
                const ncv::fold_t test_fold = std::make_pair(f, ncv::protocol::test);

                ncv::log_info() << "fold [" << (f + 1) << "/" << rtask->n_folds()
                                << "]: #train samples = " << rtask->samples(train_fold).size()
                                << ", #test samples = " << rtask->samples(test_fold).size() << ".";
        }
		
        // OK
        ncv::log_info() << ncv::done;
        return EXIT_SUCCESS;
}