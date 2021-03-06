#pragma once

#include "nanocv/arch.h"
#include "nanocv/string.h"
#include "nanocv/scalar.h"

namespace ncv
{
        ///
        /// \brief training state
        ///
        struct NANOCV_PUBLIC trainer_state_t
        {
                ///
                /// \brief constructor
                ///
                trainer_state_t();

                ///
                /// \brief constructor
                ///
                trainer_state_t(scalar_t tvalue, scalar_t terror_avg, scalar_t terror_var,
                                scalar_t vvalue, scalar_t verror_avg, scalar_t verror_var);
                
                // attributes
                scalar_t                m_tvalue;       ///< train loss value
                scalar_t                m_terror_avg;   ///< train error (average)
                scalar_t                m_terror_var;   ///< train error (variance)
                scalar_t                m_vvalue;       ///< validation loss value
                scalar_t                m_verror_avg;   ///< validation error (average)
                scalar_t                m_verror_var;   ///< validation error (variance)
        };

        typedef std::vector<trainer_state_t> trainer_states_t;

        ///
        /// \brief compare two training states
        ///
        NANOCV_PUBLIC bool operator<(const trainer_state_t& one, const trainer_state_t& two);

        ///
        /// \brief save optimization states to text file
        ///
        NANOCV_PUBLIC bool save(const string_t& path, const trainer_states_t& states);
}

