#pragma once

#include <algorithm>
#include "linesearch.h"
#include "linesearch_step.hpp"
#include "linesearch_cgdescent_secant2.hpp"

namespace ncv
{
        namespace optimize
        {
                template
                <
                        typename tstep,
                        typename tscalar = typename tstep::tscalar,
                        typename tsize = typename tstep::tsize
                >
                tstep ls_cgdescent(
                        const ls_strategy, const tscalar c1, const tscalar c2,
                        const tstep& step0, const tscalar t0, const tsize max_iters = 128)
                {
                        const tscalar epsilon = tscalar(1e-6);
                        const tscalar theta = tscalar(0.5);
                        const tscalar gamma = tscalar(0.66);

                        tstep a(step0);
                        tstep b(step0);
                        b.reset_with_grad(t0);

                        // CG_DESCENT (Hager & Zhang 2005, p. 15)
                        for (tsize i = 0; i < max_iters && (b.alpha() - a.alpha()) > a.minimum(); i ++)
                        {
                                // check Armijo+Wolfe or approximate Wolfe condition
                                if (b.phi() < a.phi())
                                {
                                        if (    (b.has_armijo(c1) && b.has_wolfe(c2)) ||
                                                (b.has_approx_wolfe(c1, c2, epsilon)))
                                        {
                                                return b.setup();
                                        }
                                }

                                else
                                {
                                        if (    (a.has_armijo(c1) && a.has_wolfe(c2)) ||
                                                (a.has_approx_wolfe(c1, c2, epsilon)))
                                        {
                                                 return a.setup();
                                        }
                                }

                                // secant interpolation
                                tstep A(a), B(a);
                                std::tie(A, B) = cgdescent_secant2(a, b, epsilon, theta);

                                // update search interval
                                if ((B.alpha() - A.alpha()) > gamma * (b.alpha() - a.alpha()))
                                {
                                        tstep c(a);
                                        c.reset_with_grad((A.alpha() + B.alpha()) / 2);
                                        std::tie(a, b) = cgdescent_update(A, B, c, epsilon, theta);
                                }
                                else
                                {
                                        a = A;
                                        b = B;
                                }
                        }

                        // NOK, give up
                        return std::min(std::min(a, b), step0);
                }
        }
}
