#pragma once

#include "criterion.h"

namespace ncv
{        
        ///
        /// \brief average loss
        ///
        class avg_criterion_t : public criterion_t
        {
        public:
                
                NANOCV_MAKE_CLONABLE(avg_criterion_t, "average loss")

                ///
                /// \brief constructor
                ///
                avg_criterion_t(const string_t& configuration = string_t());
                
                ///
                /// \brief destructor
                ///
                virtual ~avg_criterion_t() {}
                                
                ///
                /// \brief reset statistics and settings
                ///
                virtual void reset() override;

                ///
                /// \brief cumulated loss value
                ///
                virtual scalar_t value() const override;

                ///
                /// \brief cumulated gradient
                ///
                virtual vector_t vgrad() const override;

                ///
                /// \brief check if the criterion has a regularization term to tune
                ///
                virtual bool can_regularize() const override;

        protected:

                ///
                /// \brief update statistics with the loss value/error/gradient for a sample
                ///
                virtual void accumulate(scalar_t value, scalar_t error) override;
                virtual void accumulate(const vector_t& vgrad, scalar_t value, scalar_t error) override;

                ///
                /// \brief update statistics with cumulated samples
                ///
                virtual void accumulate(const criterion_t& other) override;

        protected:

                // attributes
                scalar_t                m_value;        ///< cumulated loss value
                vector_t                m_vgrad;        ///< cumulated gradient                
        };
}

