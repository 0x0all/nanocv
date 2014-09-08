#ifndef NANOCV_LOSS_CLASS_NLL_HPP
#define NANOCV_LOSS_CLASS_NLL_HPP

#include "loss.h"
#include <cassert>

namespace ncv
{
        ///
        /// \brief multi-class negative log-likelihood loss
        ///
        class classnll_loss_t : public loss_t
        {
        public:

                NANOCV_MAKE_CLONABLE(classnll_loss_t)

                // constructor
                classnll_loss_t(const string_t& = string_t())
                        :       loss_t(string_t(), "class negative-log-likelihood loss")
                {
                }

                // compute the error value
                virtual scalar_t error(const vector_t& targets, const vector_t& scores) const
                {
                        return mclass_argmax_error(targets, scores);
                }

                // compute the loss value & derivatives
                virtual scalar_t value(const vector_t& targets, const vector_t& scores) const
                {
                        return _value(targets, scores);
                }
                virtual vector_t vgrad(const vector_t& targets, const vector_t& scores) const
                {
                        return _vgrad(targets, scores);
                }

        private:

                scalar_t _value(const vector_t& targets, const vector_t& scores) const
                {
                        assert(targets.size() == scores.size());

                        const vector_t escores = scores.array().exp();
                        
                        return std::log(escores.array().sum()) - 0.5 * (1.0 + targets.array()).matrix().dot(scores);
                }

                vector_t _vgrad(const vector_t& targets, const vector_t& scores) const
                {
                        assert(targets.size() == scores.size());
                        
                        const vector_t escores = scores.array().exp();
                        
                        return escores.array() / escores.sum() - 0.5 * (1.0 + targets.array());
                }
        };
}

#endif // NANOCV_LOSS_CLASS_NLL_HPP
