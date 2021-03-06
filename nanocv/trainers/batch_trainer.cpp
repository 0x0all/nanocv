#include "batch_trainer.h"
#include "nanocv/model.h"
#include "nanocv/logger.h"
#include "nanocv/sampler.h"
#include "nanocv/math/numeric.hpp"
#include "batch.h"

namespace ncv
{
        batch_trainer_t::batch_trainer_t(const string_t& parameters)
                :       trainer_t(parameters)
        {
        }

        trainer_result_t batch_trainer_t::train(
                const task_t& task, const fold_t& fold, const loss_t& loss, size_t nthreads, const string_t& criterion,
                model_t& model) const
        {
                if (fold.second != protocol::train)
                {
                        log_error() << "batch trainer: can only train models with training samples!";
                        return trainer_result_t();
                }

                // initialize the model
                model.resize(task, true);
                model.random_params();

                // prune training & validation data
                sampler_t tsampler(task);
                tsampler.setup(fold).setup(sampler_t::atype::annotated);

                sampler_t vsampler(task);
                tsampler.split(80, vsampler);

                if (tsampler.empty() || vsampler.empty())
                {
                        log_error() << "batch trainer: no annotated training samples!";
                        return trainer_result_t();
                }

                // parameters
                const size_t iterations = math::clamp(text::from_params<size_t>(configuration(), "iters", 1024), 4, 4096);
                const scalar_t epsilon = math::clamp(text::from_params<scalar_t>(configuration(), "eps", 1e-4), 1e-8, 1e-3);

                const optim::batch_optimizer optimizer = text::from_string<optim::batch_optimizer>
                        (text::from_params<string_t>(configuration(), "opt", "lbfgs"));

                // train the model
                const trainer_result_t result = ncv::batch_train(
                        model, task, tsampler, vsampler, nthreads,
                        loss, criterion, optimizer, iterations, epsilon);

                const trainer_state_t state = result.optimum_state();

                log_info() << "optimum [train = " << state.m_tvalue << "/" << state.m_terror_avg
                           << ", valid = " << state.m_vvalue << "/" << state.m_verror_avg
                           << ", epoch = " << result.optimum_epoch()
                           << ", config = " << text::concatenate(result.optimum_config(), "/")
                           << "].";

                // OK
                if (result.valid())
                {
                        model.load_params(result.optimum_params());
                }
                return result;
        }
}
