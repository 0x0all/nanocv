#ifndef NANOCV_AVG_VAR_CRITERION_H
#define NANOCV_AVG_VAR_CRITERION_H

#include "avg_criterion.h"

namespace ncv
{        
        ///
        /// \brief variational regularized loss
        /// (e.g. penalize high variance across training samples),
        ///     like in EBBoost/VadaBoost: http://www.cs.columbia.edu/~jebara/papers/vadaboost.pdf
        ///     or my PhD thesis: http://infoscience.epfl.ch/record/177356?ln=fr
        ///
        class avg_var_criterion_t : public avg_criterion_t
        {
        public:
                
                NANOCV_MAKE_CLONABLE(avg_var_criterion_t)

                ///
                /// \brief constructor
                ///
                avg_var_criterion_t(const string_t& = string_t());
                
                ///
                /// \brief reset statistics and settings
                ///
                virtual void reset();

                ///
                /// \brief cumulate statistics
                ///
                virtual criterion_t& operator+=(const criterion_t&);

                ///
                /// \brief average loss value
                ///
                virtual scalar_t value() const;

                ///
                /// \brief average error value
                ///
                virtual scalar_t error() const;

                ///
                /// \brief average gradient
                ///
                virtual vector_t vgrad() const;

                ///
                /// \brief check if the criterion has a regularization term to tune
                ///
                virtual bool can_regularize() const;

        protected:

                ///
                /// \brief update statistics with a new sample
                ///
                virtual void accumulate(const vector_t& input, const vector_t& target, const loss_t& loss);
                
        private:
                
                // attributes
                scalar_t        m_value2;        ///< cumulated squared loss value
                vector_t        m_vgrad2;        ///< cumulated loss value multiplied with the gradient
        };
}

#endif // NANOCV_AVG_VAR_CRITERION_H