#include "layer_softmax.h"

namespace ncv
{
        /////////////////////////////////////////////////////////////////////////////////////////

        template
        <
                typename tscalar,
                typename tsize
        >
        static void _forward(
                const tscalar* idata, tsize size,
                tscalar* wdata, tscalar* odata)
        {
                auto wmap = tensor::make_vector(wdata, size);
                auto omap = tensor::make_vector(odata, size);
                auto imap = tensor::make_vector(idata, size);

                wmap = imap.array().exp();

                const tscalar sumw = wmap.sum();
                const tscalar isumw = 1 / (sumw);

                omap = imap * isumw;
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        template
        <
                typename tscalar,
                typename tsize
        >
        static void _backward(
                tscalar* idata, tsize size,
                const tscalar* wdata, const tscalar* gdata)
        {
                auto wmap = tensor::make_vector(wdata, size);
                auto gmap = tensor::make_vector(gdata, size);
                auto imap = tensor::make_vector(idata, size);

                const tscalar sumw = wmap.sum();
                const tscalar isumw2 = 1 / (sumw * sumw);

                for (tsize i = 0; i < size; i ++)
                {
                        imap(i) = gmap(i) * wmap(i) * (sumw - wmap(i)) * isumw2;
                }
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        size_t softmax_layer_t::resize(const tensor_t& tensor)
        {
                const size_t dims = tensor.dims();
                const size_t rows = tensor.rows();
                const size_t cols = tensor.cols();

                m_idata.resize(dims, rows, cols);
                m_odata.resize(dims, rows, cols);
                m_wdata.resize(dims, rows, cols);

                return 0;
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        const tensor_t& softmax_layer_t::forward(const tensor_t& input)
        {
                assert(dims() == input.dims());
                assert(rows() == input.rows());
                assert(cols() == input.cols());

                m_idata.copy_from(input);

                _forward(m_idata.data(), m_idata.size(),
                         m_wdata.data(),
                         m_odata.data());

                return m_odata;
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        const tensor_t& softmax_layer_t::backward(const tensor_t& gradient)
        {
                assert(dims() == gradient.dims());
                assert(rows() == gradient.rows());
                assert(cols() == gradient.cols());

                _backward(m_idata.data(), m_idata.size(),
                          m_wdata.data(),
                          gradient.data());

                return m_idata;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
}


