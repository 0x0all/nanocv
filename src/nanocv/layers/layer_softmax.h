#ifndef NANOCV_SOFTMAX_LAYER_H
#define NANOCV_SOFTMAX_LAYER_H

#include "layer.h"

namespace ncv
{
        ///
        /// softmax layer.
        ///
        class softmax_layer_t : public layer_t
        {
        public:

                // constructor
                softmax_layer_t(const string_t& parameters = string_t())
                        :       layer_t(parameters, "soft-max layer")
                {
                }

                // create an object clone
                virtual rlayer_t clone(const string_t& parameters) const
                {
                        return rlayer_t(new softmax_layer_t(parameters));
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

                virtual const tensor_t& input() const { return m_data; }
                virtual const tensor_t& output() const { return m_data; }
                virtual size_t psize() const { return 0; }

        private:

                size_t dims() const { return m_data.dims(); }
                size_t rows() const { return m_data.rows(); }
                size_t cols() const { return m_data.cols(); }

        private:

                // attributes
                tensor_t                m_data;         ///< input-output buffer
        };
}

#endif // NANOCV_SOFTMAX_LAYER_H

