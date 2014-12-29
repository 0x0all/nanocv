#pragma once

#include "types.h"

namespace ncv
{
        ///
        /// \brief batch optimization
        ///
        opt_state_t minimize(
                const opt_opsize_t& fn_size,
                const opt_opfval_t& fn_fval,
                const opt_opgrad_t& fn_fval_grad,
                const opt_opwlog_t& fn_wlog,
                const opt_opelog_t& fn_elog,
                const opt_opulog_t& fn_ulog,
                const vector_t& x0,
                batch_optimizer, size_t iterations, scalar_t epsilon, scalar_t history_size = 6);

        ///
        /// \brief stochastic optimization
        ///
        opt_state_t minimize(
                const opt_opsize_t& fn_size,
                const opt_opfval_t& fn_fval,
                const opt_opgrad_t& fn_fval_grad,
                const opt_opwlog_t& fn_wlog,
                const opt_opelog_t& fn_elog,
                const opt_opulog_t& fn_ulog,
                const vector_t& x0,
                stochastic_optimizer, size_t epochs, size_t epoch_size, scalar_t alpha0, scalar_t decay = 0.50);
}