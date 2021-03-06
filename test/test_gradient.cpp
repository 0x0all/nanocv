#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test_gradient"

#include <boost/test/unit_test.hpp>
#include "nanocv/nanocv.h"
#include "nanocv/logger.h"
#include "nanocv/optimizer.h"
#include "nanocv/accumulator.h"
#include "nanocv/thread/pool.h"
#include "nanocv/math/close.hpp"
#include "nanocv/math/random.hpp"
#include "nanocv/math/epsilon.hpp"
#include "nanocv-test/test_models.h"

namespace test
{
        using namespace ncv;

        thread_pool_t::mutex_t mutex;

        size_t n_checks = 0;
        size_t n_failures1 = 0;
        size_t n_failures2 = 0;
        size_t n_failures3 = 0;

        void test_grad_params(const string_t& header, const string_t& loss_id, const model_t& model,
                accumulator_t& acc_params)
        {
                random_t<size_t> rand(8, 16);

                const size_t n_tests = 16;
                const size_t n_samples = rand();

                const rloss_t rloss = ncv::get_losses().get(loss_id);
                const loss_t& loss = *rloss;

                const size_t psize = model.psize();
                const size_t isize = model.isize();
                const size_t osize = model.osize();

                vector_t params(psize);
                vectors_t targets(n_samples, vector_t(osize));
                tensors_t inputs(n_samples, tensor_t(model.idims(), model.irows(), model.icols()));

                // optimization problem (wrt parameters & inputs): size
                auto fn_params_size = [&] ()
                {
                        return psize;
                };

                // optimization problem (wrt parameters & inputs): function value
                auto fn_params_fval = [&] (const vector_t& x)
                {
                        acc_params.set_params(x);
                        acc_params.update(inputs, targets, loss);

                        return acc_params.value();
                };

                // optimization problem (wrt parameters & inputs): function value & gradient
                auto fn_params_grad = [&] (const vector_t& x, vector_t& gx)
                {
                        acc_params.set_params(x);
                        acc_params.update(inputs, targets, loss);

                        gx = acc_params.vgrad();
                        return acc_params.value();
                };

                // construct optimization problem: analytic gradient and finite difference approximation
                const opt_problem_t problem(fn_params_size, fn_params_fval, fn_params_grad);

                for (size_t t = 0; t < n_tests; t ++)
                {
                        random_t<scalar_t> prgen(-0.1, +0.1);
                        random_t<scalar_t> irgen(-0.1, +0.1);
                        random_t<size_t> trgen(0, osize - 1);

                        prgen(params.data(), params.data() + psize);
                        for (vector_t& target : targets)
                        {
                                target = ncv::class_target(trgen(), osize);
                        }
                        for (tensor_t& input : inputs)
                        {
                                irgen(input.data(), input.data() + isize);
                        }

                        const scalar_t delta = problem.grad_accuracy(params);

                        // update statistics
                        const thread_pool_t::lock_t lock(test::mutex);

                        n_checks ++;
                        if (!math::close(delta, scalar_t(0), math::epsilon1<scalar_t>()))
                        {
                                n_failures1 ++;
                        }
                        if (!math::close(delta, scalar_t(0), math::epsilon2<scalar_t>()))
                        {
                                n_failures2 ++;
                        }
                        if (!math::close(delta, scalar_t(0), math::epsilon3<scalar_t>()))
                        {
                                n_failures3 ++;

                                BOOST_CHECK_LE(delta, math::epsilon3<scalar_t>());

                                log_error() << header << ": error = " << delta << "/" << math::epsilon3<scalar_t>() << "!";
                        }
                }
        }

        void test_grad_params(const string_t& header, const string_t& loss_id, const model_t& model)
        {
                // check all criteria
                const strings_t criteria = ncv::get_criteria().ids();
                for (const string_t& criterion : criteria)
                {
                        random_t<size_t> rand(2, 16);
                        const size_t n_threads = 1 + (rand() % 2);

                        accumulator_t acc_params(model, n_threads, criterion, criterion_t::type::vgrad, 0.1);
                        test_grad_params(header + "[criterion = " + criterion + "]", loss_id, model, acc_params);
                }
        }

        void test_grad_inputs(const string_t& header, const string_t& loss_id, const model_t& model)
        {
                const rmodel_t rmodel_inputs = model.clone();

                const size_t n_tests = 16;

                const rloss_t rloss = ncv::get_losses().get(loss_id);
                const loss_t& loss = *rloss;

                const size_t psize = model.psize();
                const size_t isize = model.isize();
                const size_t osize = model.osize();

                vector_t params(psize);
                vector_t target(osize);
                tensor_t input(model.idims(), model.irows(), model.icols());

                // optimization problem (wrt parameters & inputs): size
                auto fn_inputs_size = [&] ()
                {
                        return isize;
                };

                // optimization problem (wrt parameters & inputs): function value
                auto fn_inputs_fval = [&] (const vector_t& x)
                {
                        rmodel_inputs->load_params(params);

                        const vector_t output = rmodel_inputs->output(x).vector();

                        return loss.value(target, output);
                };

                // optimization problem (wrt parameters & inputs): function value & gradient
                auto fn_inputs_grad = [&] (const vector_t& x, vector_t& gx)
                {
                        rmodel_inputs->load_params(params);

                        const vector_t output = rmodel_inputs->output(x).vector();

                        gx = rmodel_inputs->ginput(loss.vgrad(target, output)).vector();
                        return loss.value(target, output);
                };

                // construct optimization problem: analytic gradient and finite difference approximation
                const opt_problem_t problem_analytic_inputs(fn_inputs_size, fn_inputs_fval, fn_inputs_grad);
                const opt_problem_t problem_aproxdif_inputs(fn_inputs_size, fn_inputs_fval);

                for (size_t t = 0; t < n_tests; t ++)
                {
                        random_t<scalar_t> prgen(-1.0, +1.0);
                        random_t<scalar_t> irgen(-0.1, +0.1);
                        random_t<size_t> trgen(0, osize - 1);

                        prgen(params.data(), params.data() + psize);
                        target = ncv::class_target(trgen(), osize);
                        irgen(input.data(), input.data() + isize);

                        vector_t analytic_inputs_grad, aproxdif_inputs_grad;

                        problem_analytic_inputs(input.vector(), analytic_inputs_grad);
                        problem_aproxdif_inputs(input.vector(), aproxdif_inputs_grad);

                        const scalar_t delta = (analytic_inputs_grad - aproxdif_inputs_grad).lpNorm<Eigen::Infinity>();

                        // update statistics
                        const thread_pool_t::lock_t lock(test::mutex);

                        n_checks ++;
                        if (!math::close(delta, scalar_t(0), math::epsilon1<scalar_t>()))
                        {
                                n_failures1 ++;
                        }
                        if (!math::close(delta, scalar_t(0), math::epsilon2<scalar_t>()))
                        {
                                n_failures2 ++;
                        }
                        if (!math::close(delta, scalar_t(0), math::epsilon3<scalar_t>()))
                        {
                                n_failures3 ++;

                                BOOST_CHECK_LE(delta, math::epsilon3<scalar_t>());

                                log_error() << header << ": error = " << delta << "/" << math::epsilon3<scalar_t>() << "!";
                        }
                }
        }
}

BOOST_AUTO_TEST_CASE(test_gradient)
{
        using namespace ncv;

        ncv::init();

        size_t cmd_irows;
        size_t cmd_icols;
        size_t cmd_outputs;
        color_mode cmd_color;

        auto configs = test::make_grad_configs(cmd_irows, cmd_icols, cmd_outputs, cmd_color);

        // test each configuration
        thread_pool_t pool;
        for (auto config : configs)
        {
                pool.enqueue([=] ()
                {
                        const string_t desc = config.first;
                        const string_t loss_id = config.second;                        

                        {
                                const thread_pool_t::lock_t lock(test::mutex);
                                log_info() << desc;
                        }

                        // create model
                        const rmodel_t model = ncv::get_models().get("forward-network", desc);
                        BOOST_CHECK_EQUAL(model.operator bool(), true);
                        {
                                const thread_pool_t::lock_t lock(test::mutex);

                                model->resize(cmd_irows, cmd_icols, cmd_outputs, cmd_color, false);
                                BOOST_CHECK_EQUAL(model->irows(), cmd_irows);
                                BOOST_CHECK_EQUAL(model->icols(), cmd_icols);
                                BOOST_CHECK_EQUAL(model->osize(), cmd_outputs);
                                BOOST_CHECK_EQUAL(static_cast<int>(model->color()), static_cast<int>(cmd_color));
                        }

                        // check with the given loss
                        test::test_grad_params("param [loss = " + loss_id + "]", loss_id, *model);
                        test::test_grad_inputs("input [loss = " + loss_id + "]", loss_id, *model);
                });
        };

        pool.wait();

        // print statistics
        const scalar_t eps1 = math::epsilon1<scalar_t>();
        const scalar_t eps2 = math::epsilon2<scalar_t>();
        const scalar_t eps3 = math::epsilon3<scalar_t>();

        log_info() << "failures: level1 = " << test::n_failures1 << "/" << test::n_checks << ", epsilon = " << eps1;
        log_info() << "failures: level2 = " << test::n_failures2 << "/" << test::n_checks << ", epsilon = " << eps2;
        log_info() << "failures: level3 = " << test::n_failures3 << "/" << test::n_checks << ", epsilon = " << eps3;

        BOOST_CHECK_LE(test::n_failures1 * 100, 100 * test::n_checks);
        BOOST_CHECK_LE(test::n_failures2 * 100, 5 * test::n_checks);
        BOOST_CHECK_LE(test::n_failures3 * 100, 0 * test::n_checks);
}
