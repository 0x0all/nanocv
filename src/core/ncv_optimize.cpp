#include "ncv_optimize.h"
#include "ncv_math.h"
#include <deque>
#include <cassert>

namespace ncv
{
        namespace optimize
        {
                /////////////////////////////////////////////////////////////////////////////////////////////
                // checks the convergence:
                //      the gradient is relatively small.
                /////////////////////////////////////////////////////////////////////////////////////////////

                namespace impl
                {
                        inline bool converged(scalar_t epsilon, const state_t& st)
                        {
                                return st.g.lpNorm<Eigen::Infinity>() < epsilon * (1.0 + std::fabs(st.f));
                        }
                }

                /////////////////////////////////////////////////////////////////////////////////////////////
                // backtracking line-search searches for the scalar
                // that reduces the function value (the most) along the direction d:
                //
                //      argmin(t) f(x + t * d).
                /////////////////////////////////////////////////////////////////////////////////////////////

                namespace impl
                {
                        scalar_t ls_armijo(
                                const problem_t& problem, state_t& st, scalar_t t0,
                                scalar_t alpha = 0.2, scalar_t beta = 0.7)
                        {
                                // Check if descent direction
                                scalar_t dg = st.d.dot(st.g);
                                if (dg > std::numeric_limits<scalar_t>::min())
                                {
                                        st.d = -st.g;
                                        dg = st.d.dot(st.g);
                                }

                                // Armijo (sufficient decrease) condition
                                scalar_t t = t0;
                                while (problem.f(st.x + t * st.d) > st.f + t * alpha * dg)
                                {
                                        t = beta * t;
                                }

                                return t;
                        }

                        scalar_t ls_strong_wolfe(
                                const problem_t& problem, state_t& st, scalar_t t0,
                                scalar_t c1 = 1e-4, scalar_t c2 = 0.9)
                        {
                                // Check if descent direction
                                scalar_t dg = st.d.dot(st.g);
                                if (dg > std::numeric_limits<scalar_t>::min())
                                {
                                        st.d = -st.g;
                                        dg = st.d.dot(st.g);
                                }

                                vector_t gt;

                                // strong Wolfe (sufficient decrease and curvature) conditions
                                scalar_t t = t0;
                                while (problem.f(st.x + t * st.d, gt) > st.f + t * c1 * dg ||
                                       std::fabs(gt.dot(st.d)) < c2 * dg)
                                {
                                        t = c2 * t;
                                }

                                return t;
                        }
                }
        }

        //-------------------------------------------------------------------------------------------------

        optimize::result_t::result_t(size_t size)
                :       m_optimum(size),
                        m_iterations(0)
        {
        }

        //-------------------------------------------------------------------------------------------------

        void optimize::result_t::update(const state_t& st)
        {
                if (m_iterations > 0)
                {
                        const scalar_t df = std::fabs(m_optimum.f - st.f);
                        m_conv_speed.add(df / std::max(1.0, std::fabs(m_optimum.f)));
                }

                m_iterations ++;
                if (st < m_optimum)
                {
                        m_optimum = st;
                }
        }

        //-------------------------------------------------------------------------------------------------

        optimize::problem_t::problem_t(
                const op_size_t& op_size,
                const op_fval_t& op_fval,
                const op_fval_grad_t& op_fval_grad)
                :       m_op_size(op_size),
                        m_op_fval(op_fval),
                        m_op_fval_grad(op_fval_grad)
        {
        }

        //-------------------------------------------------------------------------------------------------

        optimize::problem_t::problem_t(
                const op_size_t& op_size,
                const op_fval_t& op_fval)
                :       m_op_size(op_size),
                        m_op_fval(op_fval),
                        m_op_fval_grad()
        {
        }

        //-------------------------------------------------------------------------------------------------

        size_t optimize::problem_t::size() const
        {
                return m_op_size();
        }

        //-------------------------------------------------------------------------------------------------

        scalar_t optimize::problem_t::f(const vector_t& x) const
        {
                return m_op_fval(x);
        }

        //-------------------------------------------------------------------------------------------------

        scalar_t optimize::problem_t::f(const vector_t x, vector_t& g) const
        {
                if (m_op_fval_grad)
                {
                        return m_op_fval_grad(x, g);
                }
                else
                {
                        eval_grad(x, g);
                        return f(x);
                }
        }

        //-------------------------------------------------------------------------------------------------

        void optimize::problem_t::eval_grad(const vector_t x, vector_t& g) const
        {
                const size_t n = size();
                const scalar_t d = 1e-6;

                vector_t xp = x, xn = x;

                g.resize(n);
                for (size_t i = 0; i < n; i ++)
                {
                        if (i > 0)
                        {
                                xp(i - 1) -= d;
                                xn(i - 1) += d;
                        }

                        xp(i) += d;
                        xn(i) -= d;
                        g(i) = f(xp) - f(xn);
                }

                g /= 2.0 * d;
        }

        //-------------------------------------------------------------------------------------------------

        optimize::result_t optimize::gradient_descent(
                const problem_t& problem, const vector_t& x0,
                size_t max_iterations, scalar_t epsilon,
                const op_updated_t& op_updated)
        {
                assert(problem.size() == math::cast<size_t>(x0.size()));

                result_t result(problem.size());
                state_t cstate(problem, x0);
                scalar_t t = 1.0, pdt = -1.0;

                // iterate until convergence
                for (size_t i = 0; i < max_iterations; i ++)
                {
                        result.update(cstate);
                        if (op_updated)
                        {
                                op_updated(result);
                        }

                        // check convergence
                        if (impl::converged(epsilon, cstate))
                        {
                                break;
                        }

                        // descent direction
                        cstate.d = -cstate.g;

                        // update solution
                        const scalar_t dt = cstate.g.dot(cstate.d);
                        if (i > 0)
                        {
                                t *= pdt / dt;
                        }

                        t = impl::ls_armijo(problem, cstate, t);
                        cstate.update(problem, t);
                }

                return result;
        }

        //-------------------------------------------------------------------------------------------------

        optimize::result_t optimize::conjugate_gradient_descent(
                const problem_t& problem, const vector_t& x0,
                size_t max_iterations, scalar_t epsilon,
                const op_updated_t& op_updated)
        {
                assert(problem.size() == math::cast<size_t>(x0.size()));

                result_t result(problem.size());
                state_t cstate(problem, x0), pstate = cstate;

                // iterate until convergence
                for (size_t i = 0; i < max_iterations; i ++)
                {
                        result.update(cstate);
                        if (op_updated)
                        {
                                op_updated(result);
                        }

                        // check convergence
                        if (impl::converged(epsilon, cstate))
                        {
                                break;
                        }

                        // descent direction (Polak–Ribière updates)
                        if (i == 0)
                        {
                                cstate.d = -cstate.g;
                        }
                        else
                        {
                                const scalar_t beta = cstate.g.dot(cstate.g - pstate.g) /
                                                      pstate.g.dot(pstate.g);
                                cstate.d = -cstate.g +
                                           std::max(0.0, beta) * pstate.d;
                        }

                        // update solution
                        const scalar_t t = impl::ls_strong_wolfe(problem, cstate, 1.0);
                        pstate = cstate;
                        cstate.update(problem, t);
                }

                return result;
        }

        //-------------------------------------------------------------------------------------------------

        optimize::result_t optimize::lbfgs(
                const problem_t& problem, const vector_t& x0,
                size_t max_iterations, scalar_t epsilon, size_t history_size,
                const op_updated_t& op_updated)
        {
                assert(problem.size() == math::cast<size_t>(x0.size()));

                result_t result(problem.size());
                std::deque<vector_t> ss, ys;
                state_t cstate(problem, x0), pstate = cstate;

                vector_t q, r;

                // iterate until convergence
                for (size_t i = 0; i < max_iterations; i ++)
                {
                        result.update(cstate);
                        if (op_updated)
                        {
                                op_updated(result);
                        }

                        // check convergence
                        if (impl::converged(epsilon, cstate))
                        {
                                break;
                        }

                        // descent direction
                        //      (LBFGS - Nocedal & Wright (numerical optimization 2nd) notations @ p.178)
                        q = cstate.g;

                        std::deque<vector_t>::const_reverse_iterator itr_s = ss.rbegin();
                        std::deque<vector_t>::const_reverse_iterator itr_y = ys.rbegin();
                        scalars_t alphas;
                        for (size_t j = 1; j <= history_size && i >= j; j ++)
                        {
                                const vector_t& s = (*itr_s ++);
                                const vector_t& y = (*itr_y ++);

                                const scalar_t alpha = s.dot(q) / s.dot(y);
                                q.noalias() -= alpha * y;
                                alphas.push_back(alpha);
                        }

                        if (i == 0)
                        {
                                r = q;
                        }
                        else
                        {
                                const vector_t& s = *ss.rbegin();
                                const vector_t& y = *ys.rbegin();
                                r = s.dot(y) / y.dot(y) * q;
                        }

                        std::deque<vector_t>::const_iterator it_s = ss.begin();
                        std::deque<vector_t>::const_iterator it_y = ys.begin();
                        scalars_t::const_reverse_iterator itr_alpha = alphas.rbegin();
                        for (size_t j = 1; j <= history_size && i >= j; j ++)
                        {
                                const vector_t& s = (*it_s ++);
                                const vector_t& y = (*it_y ++);

                                const scalar_t alpha = *(itr_alpha ++);
                                const scalar_t beta = y.dot(r) / s.dot(y);
                                r.noalias() += s * (alpha - beta);
                        }

                        cstate.d = -r;

                        // update solution
                        const scalar_t t = impl::ls_strong_wolfe(problem, cstate, 1.0);
                        pstate = cstate;
                        cstate.update(problem, t);

                        ss.push_back(cstate.x - pstate.x);
                        ys.push_back(cstate.g - pstate.g);
                        if (ss.size() > history_size)
                        {
                                ss.pop_front();
                                ys.pop_front();
                        }
                }

                return result;
        }

        //-------------------------------------------------------------------------------------------------
}