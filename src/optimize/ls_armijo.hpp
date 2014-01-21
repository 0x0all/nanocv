#ifndef NANOCV_OPTIMIZE_LINESEARCH_ARMIJO_HPP
#define NANOCV_OPTIMIZE_LINESEARCH_ARMIJO_HPP

#include "descent.hpp"

namespace ncv
{
        namespace optimize
        {
                /////////////////////////////////////////////////////////////////////////////////////////
                // line-search method to find the scalar that reduces
                //      the function value (the most) along the direction d: argmin(t) f(x + t * d).
                //      using the Armijo (sufficient decrease) condition
                /////////////////////////////////////////////////////////////////////////////////////////

                template
                <
                        typename tproblem,

                        // dependent types
                        typename tscalar = typename tproblem::tscalar,
                        typename tsize = typename tproblem::tsize,
                        typename tvector = typename tproblem::tvector,
                        typename tstate = typename tproblem::tstate,

                        typename twlog = typename tproblem::twlog,
                        typename telog = typename tproblem::telog,
                        typename tulog = typename tproblem::tulog
                >
                tscalar ls_armijo(const tproblem& problem, tstate& st, const twlog& wlog,
                        tscalar alpha = 0.2, tscalar beta = 0.7, tsize max_iters = 64)
                {
                        const tscalar dg = descent(st, wlog);

                        tscalar t = 1;
                        for (tsize i = 0; i < max_iters; i ++, t = beta * t)
                        {
                                if (problem(st.x + t * st.d) < st.f + t * alpha * dg)
                                {
                                        return t;
                                }
                        }

                        return 0;
                }
        }
}

#endif // NANOCV_OPTIMIZE_LINESEARCH_ARMIJO_HPP