#include "layer_convolution.h"
#include "common/logger.h"
#include "common/math.hpp"
#include "common/random.hpp"
#include "common/conv2d.hpp"
#include "common/iconv2d.hpp"
#include "common/sampling.hpp"
#include "tensor/serialize.hpp"

namespace ncv
{
        template
        <
                typename tscalar,
                typename tsize
        >
        static void _output(
                const tscalar* idata, tsize idims,
                const tscalar* kdata, tsize krows, tsize kcols,
                tscalar* odata, tsize odims, tsize orows, tsize ocols)
        {
                const tsize irows = orows + krows - 1;
                const tsize icols = ocols + kcols - 1;
                const tsize isize = irows * icols;

                const tsize osize = orows * ocols;                
                const tsize ksize = krows * kcols;

                // output
                for (tsize o = 0; o < odims; o ++)
                {
                        auto omap = tensor::make_matrix(odata + o * osize, orows, ocols);

                        omap.setZero();
                        for (tsize i = 0; i < idims; i ++)
                        {
                                auto imap = tensor::make_matrix(idata + i * isize, irows, icols);
                                auto kmap = tensor::make_matrix(kdata + (o * idims + i) * ksize, krows, kcols);
                                
                                ncv::conv2d_dot(imap, kmap, omap);
                        }
                }
        }

        template
        <
                typename tscalar,
                typename tsize
        >
        static void _igrad(
                tscalar* gidata, tsize idims,
                const tscalar* kdata, tsize krows, tsize kcols,
                const tscalar* odata, tsize odims, tsize orows, tsize ocols)
        {
                const tsize irows = orows + krows - 1;
                const tsize icols = ocols + kcols - 1;
                const tsize isize = irows * icols;

                const tsize osize = orows * ocols;
                const tsize ksize = krows * kcols;
                
                // input gradient
                tensor::make_vector(gidata, idims * isize).setZero();
                for (tsize o = 0; o < odims; o ++)
                {
                        auto omap = tensor::make_matrix(odata + o * osize, orows, ocols);
                        
                        for (tsize i = 0; i < idims; i ++)
                        {
                                auto gimap = tensor::make_matrix(gidata + i * isize, irows, icols);
                                auto kmap = tensor::make_matrix(kdata + (o * idims + i) * ksize, krows, kcols);                                     

                                ncv::iconv2d_mad(omap, kmap, gimap);
                        }
                }
        }

        template
        <
                typename tscalar,
                typename tsize
        >
        static void _pgrad(
                const tscalar* idata, tsize idims,
                tscalar* gkdata, tsize krows, tsize kcols,
                const tscalar* odata, tsize odims, tsize orows, tsize ocols)
        {
                const tsize irows = orows + krows - 1;
                const tsize icols = ocols + kcols - 1;
                const tsize isize = irows * icols;

                const tsize osize = orows * ocols;
                const tsize ksize = krows * kcols;

                for (tsize o = 0; o < odims; o ++)
                {
                        auto omap = tensor::make_matrix(odata + o * osize, orows, ocols);

                        for (tsize i = 0; i < idims; i ++)
                        {
                                auto imap = tensor::make_matrix(idata + i * isize, irows, icols);
                                auto gkmap = tensor::make_matrix(gkdata + (o * idims + i) * ksize, krows, kcols);

                                gkmap.setZero();
                                ncv::conv2d_dot(imap, omap, gkmap);
                        }
                }
        }

        conv_layer_t::conv_layer_t(const string_t& parameters)
                :       layer_t(parameters),
                        m_type(type::full)
        {
        }

        conv_layer_t::~conv_layer_t()
        {
        }

        size_t conv_layer_t::resize(const tensor_t& tensor)
        {
                const size_t idims = tensor.dims();
                const size_t irows = tensor.rows();
                const size_t icols = tensor.cols();

                const size_t odims = math::clamp(text::from_params<size_t>(configuration(), "dims", 16), 1, 256);
                const size_t krows = math::clamp(text::from_params<size_t>(configuration(), "rows", 8), 1, 32);
                const size_t kcols = math::clamp(text::from_params<size_t>(configuration(), "cols", 8), 1, 32);

                // check connection type
                const string_t t = text::from_params<string_t>(configuration(), "type", "full");
                if (t == "full")
                {
                        m_type = type::full;
                }
                else if (t == "rand")
                {
                        m_type = type::rand;
                }
                else if (t == "mask")
                {
                        m_type = type::mask;
                }
                else
                {
                        const string_t message =
                                "invalid connection type (" + t + ")! expecting [full, rand, mask].";

                        log_error() << "convolution layer: " << message;
                        throw std::runtime_error("convolution layer: " + message);
                }

                // check convolution size
                if (irows < krows || icols < kcols)
                {
                        const string_t message =
                                "invalid size (" + text::to_string(idims) + "x" + text::to_string(irows) +
                                 "x" + text::to_string(icols) + ") -> (" + text::to_string(odims) + "x" +
                                 text::to_string(krows) + "x" + text::to_string(kcols) + ")";

                        log_error() << "convolution layer: " << message;
                        throw std::runtime_error("convolution layer: " + message);
                }

                const size_t orows = irows - krows + 1;
                const size_t ocols = icols - kcols + 1;

                // resize buffers
                m_idata.resize(idims, irows, icols);
                m_odata.resize(odims, orows, ocols);
                m_kdata.resize(odims * idims, krows, kcols);
                m_mdata.resize(odims, idims);

                return psize();
        }

        void conv_layer_t::zero_params()
        {
                m_kdata.zero();

                generate_mask();
        }

        void conv_layer_t::random_params(scalar_t min, scalar_t max)
        {
                m_kdata.random(random_t<scalar_t>(min, max));

                generate_mask();
        }

        void conv_layer_t::generate_mask()
        {
                switch (m_type)
                {
                case type::full:
                        m_mdata.setOnes();
                        break;

                case type::rand:
                        m_mdata.setOnes();
                        break;

                case type::mask:
                        m_mdata.setZero();
                        for (size_t o = 0; o < odims(); o ++)
                        {
                                const indices_t indices = uniform_indices(idims(), std::max(size_t(1), idims() / 2));
                                for (size_t i : indices)
                                {
                                        m_mdata(o, i) = 1.0;
                                }

                                string_t mask;
                                for (size_t i = 0; i < idims(); i ++)
                                {
                                        mask.append(m_mdata(o, i) > 0.5 ? "1" : "0");
                                        if (i + 1 < idims())
                                        {
                                                mask.append(", ");
                                        }
                                }

                                log_info() << "mask [" << (o + 1) << "/" << odims() << "]: " << mask;
                        }
                        break;
                }
        }

        scalar_t* conv_layer_t::save_params(scalar_t* params) const
        {
                switch (m_type)
                {
                case type::full:
                case type::mask:
                        params = tensor::save(m_kdata, params);
                        break;

                case type::rand:
                        break;
                }

                return params;
        }

        const scalar_t* conv_layer_t::load_params(const scalar_t* params)
        {
                switch (m_type)
                {
                case type::full:
                case type::mask:
                        params = tensor::load(m_kdata, params);
                        break;

                case type::rand:
                        break;
                }

                return params;
        }

        size_t conv_layer_t::psize() const
        {
                switch (m_type)
                {
                case type::full:
                case type::mask:
                        return m_kdata.size();

                case type::rand:
                default:
                        return 0;
                }
        }

        const tensor_t& conv_layer_t::output(const tensor_t& input)
        {
                assert(idims() == input.dims());
                assert(irows() == input.rows());
                assert(icols() == input.cols());

                m_idata.copy_from(input);

                _output(m_idata.data(), idims(),
                        m_kdata.data(), krows(), kcols(),
                        m_odata.data(), odims(), orows(), ocols());

                return m_odata;
        }        

        const tensor_t& conv_layer_t::igrad(const tensor_t& output)
        {
                assert(odims() == output.dims());
                assert(orows() == output.rows());
                assert(ocols() == output.cols());

                m_odata.copy_from(output);

                _igrad(m_idata.data(), idims(),
                       m_kdata.data(), krows(), kcols(),
                       m_odata.data(), odims(), orows(), ocols());

                return m_idata;
        }

        void conv_layer_t::pgrad(const tensor_t& output, scalar_t* gradient)
        {
                assert(odims() == output.dims());
                assert(orows() == output.rows());
                assert(ocols() == output.cols());

                m_odata.copy_from(output);

                switch (m_type)
                {
                case type::full:
                case type::mask:
                        _pgrad(m_idata.data(), idims(),
                               gradient, krows(), kcols(),
                               m_odata.data(), odims(), orows(), ocols());
                        break;

                case type::rand:
                        break;
                }
        }
}


