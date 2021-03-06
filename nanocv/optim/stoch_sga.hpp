#pragma once

#include "stoch_params.hpp"
#include "nanocv/math/average_vector.hpp"
#include <cassert>

namespace ncv
{
        namespace optim
        {
                ///
                /// \brief stochastic gradient average gradient (descent)
                ///     see "Minimizing Finite Sums with the Stochastic Average Gradient",
                ///     by Mark Schmidth, Nicolas Le Roux, Francis Bach
                ///
                template
                <
                        typename tproblem               ///< optimization problem
                >
                struct stoch_sga_t : public stoch_params_t<tproblem>
                {
                        typedef stoch_params_t<tproblem>        base_t;

                        typedef typename base_t::tscalar        tscalar;
                        typedef typename base_t::tsize          tsize;
                        typedef typename base_t::tvector        tvector;
                        typedef typename base_t::tstate         tstate;
                        typedef typename base_t::twlog          twlog;
                        typedef typename base_t::telog          telog;
                        typedef typename base_t::tulog          tulog;

                        ///
                        /// \brief constructor
                        ///
                        stoch_sga_t(    tsize epochs,
                                        tsize epoch_size,
                                        tscalar alpha0,
                                        tscalar decay,
                                        const twlog& wlog = twlog(),
                                        const telog& elog = telog(),
                                        const tulog& ulog = tulog())
                                :       base_t(epochs, epoch_size, alpha0, decay, wlog, elog, ulog)
                        {
                        }

                        ///
                        /// \brief minimize starting from the initial guess x0
                        ///
                        tstate operator()(const tproblem& problem, const tvector& x0) const
                        {
                                assert(problem.size() == static_cast<tsize>(x0.size()));

                                // current state
                                tstate cstate(problem, x0);

                                // running-weighted-averaged gradient
                                average_vector_t<tscalar, tvector> gavg(x0.size());

                                for (tsize e = 0, k = 1; e < base_t::m_epochs; e ++)
                                {
                                        for (tsize i = 0; i < base_t::m_epoch_size; i ++, k ++)
                                        {
                                                // learning rate
                                                const tscalar alpha = base_t::alpha(k);

                                                // descent direction
                                                gavg.update(cstate.g, base_t::weight(k));
                                                cstate.d = -gavg.value();

                                                // update solution
                                                cstate.update(problem, alpha);
                                        }

                                        base_t::ulog(cstate);
                                }

                                return cstate;
                        }
                };
        }
}

