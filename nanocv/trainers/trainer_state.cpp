#include "trainer_state.h"
#include "nanocv/text.h"
#include <fstream>
#include <limits>
#include <cmath>

namespace ncv
{
        trainer_state_t::trainer_state_t()
                :       trainer_state_t(
                        std::numeric_limits<scalar_t>::max(),
                        std::numeric_limits<scalar_t>::max(),
                        std::numeric_limits<scalar_t>::max(),
                        std::numeric_limits<scalar_t>::max(),
                        std::numeric_limits<scalar_t>::max(),
                        std::numeric_limits<scalar_t>::max())
        {
        }

        trainer_state_t::trainer_state_t(
                        scalar_t tvalue,
                        scalar_t terror_avg,
                        scalar_t terror_var,
                        scalar_t vvalue,
                        scalar_t verror_avg,
                        scalar_t verror_var)
                :       m_tvalue(tvalue),
                        m_terror_avg(terror_avg),
                        m_terror_var(terror_var),
                        m_vvalue(vvalue),
                        m_verror_avg(verror_avg),
                        m_verror_var(verror_var)
        {
        }

        bool operator<(const trainer_state_t& one, const trainer_state_t& two)
        {
                const scalar_t v1 = std::isfinite(one.m_verror_avg) ? one.m_verror_avg : std::numeric_limits<scalar_t>::max();
                const scalar_t v2 = std::isfinite(two.m_verror_avg) ? two.m_verror_avg : std::numeric_limits<scalar_t>::max();
                return v1 < v2;
        }

        bool save(const string_t& path, const trainer_states_t& states)
        {
                std::ofstream ofs(path.c_str(), std::ofstream::out);
                if (!ofs.is_open())
                {
                        return false;
                }
                
                const string_t delim = "\t";
                const size_t colsize = 24;
                
                // header
                ofs 
                << text::resize("train-loss", colsize) << delim
                << text::resize("train-error-average", colsize) << delim
                << text::resize("train-error-variance", colsize) << delim
                << text::resize("valid-loss", colsize) << delim
                << text::resize("valid-error-average", colsize) << delim
                << text::resize("valid-error-variance", colsize) << delim
                << std::endl;

                // optimization states
                for (const trainer_state_t& state : states)
                {
                        ofs 
                        << text::resize(text::to_string(state.m_tvalue), colsize) << delim
                        << text::resize(text::to_string(state.m_terror_avg), colsize) << delim
                        << text::resize(text::to_string(state.m_terror_var), colsize) << delim
                        << text::resize(text::to_string(state.m_vvalue), colsize) << delim
                        << text::resize(text::to_string(state.m_verror_avg), colsize) << delim
                        << text::resize(text::to_string(state.m_verror_var), colsize) << delim
                        << std::endl;
                }

                return ofs.good();
        }
}
	
