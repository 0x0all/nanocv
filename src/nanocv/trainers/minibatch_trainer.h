#ifndef MINIBATCH_TRAINER_H
#define MINIBATCH_TRAINER_H

#include "trainer.h"

namespace ncv
{
        ///
        /// mini-batch trainer: each gradient update is computed for all samples.
        ///
        /// parameters:
        ///      batch=1024[256,8192]    - mini-batch size (#samples)
        ///      iters=1024[4,4096]      - maximum number of iterations
        ///      eps=1e-6[1e-8,1e-3]     - convergence
        ///
        class minibatch_trainer_t : public trainer_t
        {
        public:

                // constructor
                minibatch_trainer_t(const string_t& parameters = string_t());

                // create an object clone
                virtual rtrainer_t clone(const string_t& parameters) const
                {
                        return rtrainer_t(new minibatch_trainer_t(parameters));
                }

                // train the model
                virtual bool train(const task_t&, const fold_t&, const loss_t&, size_t nthreads, model_t&) const;
        };
}

#endif // MINIBATCH_TRAINER_H