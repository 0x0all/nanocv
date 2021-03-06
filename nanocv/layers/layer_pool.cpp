#include "layer_pool.h"
#include "pooling.hpp"
#include "nanocv/text.h"
#include "nanocv/math/clamp.hpp"

namespace ncv
{
        pool_layer_t::pool_layer_t(const string_t& parameters)
                :       layer_t(parameters),
                        m_alpha(math::clamp(text::from_params<scalar_t>(parameters, "dims", 0.1), -100.0, +100.0))
        {
        }

        size_t pool_layer_t::resize(const tensor_t& tensor)
        {
                const size_t idims = tensor.dims();
                const size_t irows = tensor.rows();
                const size_t icols = tensor.cols();

                const size_t odims = idims;
                const size_t orows = (irows + 1) / 2;
                const size_t ocols = (icols + 1) / 2;

                m_idata.resize(idims, irows, icols);
                m_odata.resize(odims, orows, ocols);

                m_wdata.resize(idims, irows, icols);
                m_sdata.resize(odims, orows, ocols);
                m_cdata.resize(odims, orows, ocols);

                return 0;
        }

        const tensor_t& pool_layer_t::output(const tensor_t& input)
        {
                assert(idims() == input.dims());
                assert(irows() <= input.rows());
                assert(icols() <= input.cols());

                m_idata = input;

                for (size_t o = 0; o < odims(); o ++)
                {
                        pooling::output(
                                m_idata.matrix(o), m_alpha,
                                m_wdata.matrix(o),
                                m_sdata.matrix(o),
                                m_cdata.matrix(o),
                                m_odata.matrix(o));
                }

                return m_odata;
        }

        const tensor_t& pool_layer_t::ginput(const tensor_t& output)
        {
                assert(odims() == output.dims());
                assert(orows() == output.rows());
                assert(ocols() == output.cols());

                m_odata = output;

                for (size_t o = 0; o < odims(); o ++)
                {
                        pooling::ginput(
                                m_idata.matrix(o),
                                m_wdata.matrix(o),
                                m_sdata.matrix(o),
                                m_cdata.matrix(o),
                                m_odata.matrix(o));
                }

                return m_idata;
        }

        void pool_layer_t::gparam(const tensor_t& output, scalar_t* gradient)
        {
                NANOCV_UNUSED1(gradient);
                NANOCV_UNUSED1_RELEASE(output);

                assert(odims() == output.dims());
                assert(orows() == output.rows());
                assert(ocols() == output.cols());
        }
}


