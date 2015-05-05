#pragma once

#include "vector.hpp"
#include "matrix.hpp"
#include <cassert>
#include <boost/serialization/access.hpp>

namespace ncv
{
        namespace tensor
        {
                ///
                /// 3D tensor stored as ::dims() 2D planes of size ::rows() x ::cols()
                ///
                template
                <
                        typename tscalar_,
                        typename tsize_
                >
                class tensor_t
                {
                public:

                        typedef tscalar_                                        Scalar; // Eigen compatible
                        typedef tscalar_                                        tscalar;
                        typedef tsize_                                          tindex;
                        typedef tsize_                                          tsize;
                        typedef typename vector_types_t<tscalar>::tvector       tvector;
                        typedef typename matrix_types_t<tscalar>::tmatrix       tmatrix;

                        ///
                        /// \brief constructor
                        ///
                        tensor_t(tsize dims = 0, tsize rows = 0, tsize cols = 0)
                        {
                                resize(dims, rows, cols);
                        }

                        ///
                        /// \brief resize to new dimensions
                        ///
                        tsize resize(tsize dims, tsize rows, tsize cols)
                        {
                                m_dims = dims;
                                m_rows = rows;
                                m_cols = cols;

                                m_data.resize(m_dims * m_rows * m_cols);
                                m_data.setZero();

                                return size();
                        }

                        ///
                        /// \brief set all elements to zero
                        ///
                        void setZero()
                        {
                                m_data.setZero();
                        }

                        ///
                        /// \brief set all elements to constant
                        ///
                        void setConstant(tscalar val)
                        {
                                m_data.setConstant(val);
                        }

                        ///
                        /// \brief set all elements to random values using the given generator
                        ///
                        template
                        <
                                typename tgenerator
                        >
                        void random(tgenerator gen)
                        {
                                gen(m_data.data(), m_data.data() + size());
                        }

                        ///
                        /// \brief dimensions
                        ///
                        tsize size() const { return m_data.size(); }
                        tsize dims() const { return m_dims; }
                        tsize rows() const { return m_rows; }
                        tsize cols() const { return m_cols; }                        
                        tsize plane_size() const { return rows() * cols(); }

                        ///
                        /// \brief access the tensor as a vector (size() x 1)
                        ///
                        const tvector& vector() const { return m_data; }

                        ///
                        /// \brief access the whole tensor data
                        ///
                        const tscalar* data() const { return m_data.data(); }
                        tscalar* data() { return m_data.data(); }

                        ///
                        /// \brief access an element of the tensor in the range [0, size())
                        ///
                        tscalar data(size_t i) const { return m_data(i); }
                        tscalar& data(size_t i) { return m_data(i); }

                        ///
                        /// \brief access the 2D planes
                        ///
                        const tscalar* plane_data(tsize i) const
                        {
                                return data() + i * plane_size();
                        }
                        tscalar* plane_data(tsize i)
                        {
                                return data() + i * plane_size();
                        }

                        decltype(auto) plane_matrix(tsize i = 0)
                        {
                                return tensor::map_matrix(plane_data(i), rows(), cols());
                        }
                        decltype(auto) plane_matrix(tsize i = 0) const
                        {
                                return tensor::map_matrix(plane_data(i), rows(), cols());
                        }

                        decltype(auto) plane_vector(tsize i = 0)
                        {
                                return tensor::map_vector(plane_data(i), plane_size());
                        }
                        decltype(auto) plane_vector(tsize i = 0) const
                        {
                                return tensor::map_vector(plane_data(i), plane_size());
                        }

                        ///
                        /// \brief copy to/from another tensor (of the same size)
                        ///
                        void copy_from(const tensor_t& t)
                        {
                                assert(size() == t.size());
                                copy_from(t.data());
                        }
                        void copy_to(tensor_t& t) const
                        {
                                assert(size() == t.size());
                                copy_to(t.data());
                        }

                        void copy_from(const tscalar* d)
                        {
                                m_data = tensor::map_vector(d, size());
                        }
                        void copy_to(tscalar* d) const
                        {                                
                                tensor::map_vector(d, size()) = m_data;
                        }

                        ///
                        /// \brief copy plane to/from another tensor (of the same size)
                        ///
                        void copy_plane_from(tsize i, const tensor_t& t)
                        {
                                assert(plane_size() == static_cast<tsize>(t.size()));
                                assert(i < dims());
                                copy_plane_from(i, t.data());
                        }
                        void copy_plane_to(tsize i, tensor_t& t) const
                        {
                                assert(plane_size() == t.size());
                                assert(i < dims());
                                copy_plane_to(i, t.data());
                        }

                        void copy_plane_from(tsize i, const tscalar* d)
                        {
                                tensor::map_vector(plane_data(i), plane_size()) = tensor::map_vector(d, plane_size());
                        }
                        void copy_plane_to(tsize i, tscalar* d) const
                        {
                                tensor::map_vector(d, plane_size()) = tensor::map_vector(plane_data(i), plane_size());
                        }

                private:

                        ///
                        /// serialize
                        ///
                        friend class boost::serialization::access;
                        template
                        <
                                class tarchive
                        >
                        void serialize(tarchive & ar, const unsigned int version)
                        {
                                ar & m_dims;
                                ar & m_rows;
                                ar & m_cols;
                                ar & m_data;
                        }

                private:

                        // attributes
                        tsize                   m_dims;         ///< #dimensions
                        tsize                   m_rows;         ///< #rows (for each dimension)
                        tsize                   m_cols;         ///< #cols (for each dimension)
                        tvector                 m_data;         ///< storage (1D vector)
                };
        }
}
