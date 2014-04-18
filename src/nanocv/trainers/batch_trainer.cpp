#include "batch_trainer.h"
#include "common/math.hpp"
#include "common/logger.h"
#include "common/usampler.hpp"
#include "common/random.hpp"
#include "text.h"

namespace ncv
{
        /////////////////////////////////////////////////////////////////////////////////////////

        batch_trainer_t::batch_trainer_t(const string_t& parameters)
                :       trainer_t(parameters,
                                  "batch trainer, parameters: opt=lbfgs[,cgd,gd],iters=1024[4,4096],eps=1e-6[1e-8,1e-3]")
        {
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        bool batch_trainer_t::train(
                const task_t& task, const fold_t& fold, const loss_t& loss, size_t nthreads,
                model_t& model) const
        {
                if (fold.second != protocol::train)
                {
                        log_error() << "batch trainer: cannot only train models with training samples!";
                        return false;
                }

                // initialize the model
                model.resize(task, true);
                model.random_params();

                // prune training & validation data
                const samples_t samples = ncv::prune_annotated(task, task.samples(fold));
                if (samples.empty())
                {
                        log_error() << "batch trainer: no annotated training samples!";
                        return false;
                }

                samples_t tsamples, vsamples;
                ncv::uniform_split(samples, size_t(90), random_t<size_t>(0, samples.size()), tsamples, vsamples);

                // parameters
                const string_t optimizer = text::from_params<string_t>(parameters(), "opt", "lbfgs");
                const size_t iterations = math::clamp(text::from_params<size_t>(parameters(), "iters", 1024), 4, 4096);
                const scalar_t epsilon = math::clamp(text::from_params<scalar_t>(parameters(), "eps", 1e-6), 1e-8, 1e-3);
                const size_t batchsize = 0;

                const scalars_t l2_weights = { 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1.0 };

                // L2-regularize the loss
                trainer_state_t state(model.n_parameters());
                for (scalar_t l2_weight : l2_weights)
                {
                        trainer_t::train(task, tsamples, vsamples, batchsize, nthreads,
                                         loss, l2_weight, optimizer, iterations, epsilon,
                                         model, state);
                }

                model.load_params(state.m_params);

                // OK
                return true;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
}