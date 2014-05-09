#ifndef NANOCV_POOL_SOFTABS_LAYER_H
#define NANOCV_POOL_SOFTABS_LAYER_H

#include "layer.h"

namespace ncv
{
        ///
        /// soft-abs pooling layer:
        ///      down-sample by 2 from a 3x3 neighbouring region using a soft-max weighting.
        ///      weight ~ absolute input value.
        ///
        class pool_softabs_layer_t : public layer_t
        {
        public:

                // constructor
                pool_softabs_layer_t(const string_t& parameters = string_t())
                        :       layer_t(parameters, "soft-abs pooling layer")
                {
                }

                // create an object clone
                virtual rlayer_t clone(const string_t& parameters) const
                {
                        return rlayer_t(new pool_softabs_layer_t(parameters));
                }

                // resize to process new tensors of the given type
                virtual size_t resize(const tensor_t& tensor);

                // reset parameters
                virtual void zero_params() {}
                virtual void random_params(scalar_t min, scalar_t max) {}

                // serialize parameters & gradients
                virtual ovectorizer_t& save_params(ovectorizer_t& s) const { return s; }
                virtual ovectorizer_t& save_grad(ovectorizer_t& s) const { return s; }
                virtual ivectorizer_t& load_params(ivectorizer_t& s) { return s; }

                // process inputs (compute outputs & gradients)
                virtual const tensor_t& forward(const tensor_t& input);
                virtual const tensor_t& backward(const tensor_t& gradient);

                // access functions
                virtual const tensor_t& input() const { return m_idata; }
                virtual const tensor_t& output() const { return m_odata; }
                virtual size_t psize() const { return 0; }

        private:

                size_t idims() const { return m_idata.dims(); }
                size_t irows() const { return m_idata.rows(); }
                size_t icols() const { return m_idata.cols(); }

                size_t odims() const { return m_odata.dims(); }
                size_t orows() const { return m_odata.rows(); }
                size_t ocols() const { return m_odata.cols(); }

        private:

                // attributes
                tensor_t                m_idata;        ///< input buffer
                tensor_t                m_odata;        ///< output buffer

                tensor_t                m_wdata;        ///< pooling weights
                tensor_t                m_sdata;        ///< nominator
                tensor_t                m_tdata;        ///< denominator
        };
}

#endif // NANOCV_POOL_SOFTABS_LAYER_H
