#include "image.h"
#include "common/bilinear.hpp"
#include "tensor/transform.hpp"
#include <fstream>

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL

#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/tiff_io.hpp>
#include <boost/gil/extension/io/png_io.hpp>

namespace ncv
{
        // use Magick++ (cmake support) instead of GIL
        //      supports more formats, can read from memory buffers

        // or DevIL (cmake support):
        //      https://github.com/DentonW/DevIL/tree/master/DevIL-docs

        enum class imagetype : int
        {
                jpeg,
                png,
                tif,
                pgm,
                unknown
        };

        static imagetype decode_image_type(const string_t& path)
        {
                if (text::iends_with(path, ".jpg") || text::iends_with(path, ".jpeg"))
                {
                        return imagetype::jpeg;
                }

                else if (text::iends_with(path, ".png"))
                {
                        return imagetype::png;
                }

                else if (text::iends_with(path, ".tif") || text::iends_with(path, ".tiff"))
                {
                        return imagetype::tif;
                }

                else if (text::iends_with(path, ".pgm"))
                {
                        return imagetype::pgm;
                }

                else
                {
                        return imagetype::unknown;
                }
        }

        static bool load_image(const string_t& path,
                color_mode mode, rgba_matrix_t& rgba, luma_matrix_t& luma)
        {
                const imagetype itype = decode_image_type(path);
                switch (itype)
                {
                case imagetype::png:    // boost::gil decoding
                case imagetype::jpeg:
                case imagetype::tif:
                        {
                                boost::gil::argb8_image_t image;

                                if (itype == imagetype::png)
                                {
                                        boost::gil::png_read_and_convert_image(path, image);
                                }
                                else if (itype == imagetype::jpeg)
                                {
                                        boost::gil::jpeg_read_and_convert_image(path, image);
                                }
                                else
                                {
                                        boost::gil::tiff_read_and_convert_image(path, image);
                                }

                                const int rows = static_cast<int>(image.height());
                                const int cols = static_cast<int>(image.width());

                                const boost::gil::argb8_image_t::const_view_t view = boost::gil::const_view(image);
                                switch (mode)
                                {
                                case color_mode::luma:
                                        luma.resize(rows, cols);
                                        for (int r = 0; r < rows; r ++)
                                        {
                                                for (int c = 0; c < cols; c ++)
                                                {
                                                        const boost::gil::argb8_pixel_t pix = view(c, r);
                                                        luma(r, c) = color::make_luma(pix[1], pix[2], pix[3]);
                                                }
                                        }
                                        break;

                                case color_mode::rgba:
                                        rgba.resize(rows, cols);
                                        for (int r = 0; r < rows; r ++)
                                        {
                                                for (int c = 0; c < cols; c ++)
                                                {
                                                        const boost::gil::argb8_pixel_t pix = view(c, r);
                                                        rgba(r, c) = color::make_rgba(pix[1], pix[2], pix[3], pix[0]);
                                                }
                                        }
                                        break;
                                }
                        }
                        return true;

                case imagetype::pgm:    // PGM binary decoding
                        {
                                std::ifstream is(path);

                                // read header
                                string_t line_type, line_size, line_maxv;
                                if (    !is.is_open() ||
                                        !std::getline(is, line_type) ||
                                        !std::getline(is, line_size) ||
                                        !std::getline(is, line_maxv) ||
                                        line_type != "P5" ||
                                        line_maxv != "255")
                                {
                                        return false;
                                }

                                strings_t tokens;
                                text::split(tokens, line_size, text::is_any_of(" "));

                                int rows = -1, cols = -1;
                                if (    tokens.size() != 2 ||
                                        (cols = text::from_string<int>(tokens[0])) < 1 ||
                                        (rows = text::from_string<int>(tokens[1])) < 1)
                                {
                                        return false;
                                }

                                // read pixels
                                std::vector<uint8_t> grays(rows * cols);
                                if (!is.read((char*)(grays.data()), grays.size()))
                                {
                                        return false;
                                }

                                switch (mode)
                                {
                                case color_mode::luma:
                                        luma.resize(rows, cols);
                                        tensor::transform(tensor::make_matrix(grays.data(), rows, cols),
                                                          luma, [] (luma_t g) { return g; });
                                        break;

                                case color_mode::rgba:
                                        rgba.resize(rows, cols);
                                        tensor::transform(tensor::make_matrix(grays.data(), rows, cols),
                                                          rgba, [] (luma_t g) { return color::make_rgba(g, g, g); });
                                        break;
                                }
                        }
                        return true;

                case imagetype::unknown:
                default:
                        return false;
                }
        }

        static bool save_image(const string_t& path,
                color_mode mode, const rgba_matrix_t& rgba, const luma_matrix_t& luma)
        {
                const int rows = static_cast<int>(mode == color_mode::rgba ? rgba.rows() : luma.rows());
                const int cols = static_cast<int>(mode == color_mode::rgba ? rgba.cols() : luma.cols());

                const imagetype itype = decode_image_type(path);
                switch (itype)
                {
                case imagetype::png: // boost::gil RGBA encoding
                        {
                                boost::gil::argb8_image_t image(cols, rows);

                                boost::gil::argb8_image_t::view_t view = boost::gil::view(image);
                                switch (mode)
                                {
                                case color_mode::luma:
                                        for (int r = 0; r < rows; r ++)
                                        {
                                                for (int c = 0; c < cols; c ++)
                                                {
                                                        boost::gil::argb8_pixel_t& pixel = view(c, r);
                                                        pixel[0] = 255;
                                                        pixel[1] = luma(r, c);
                                                        pixel[2] = luma(r, c);
                                                        pixel[3] = luma(r, c);
                                                }
                                        }
                                        break;

                                case color_mode::rgba:
                                        for (int r = 0; r < rows; r ++)
                                        {
                                                for (int c = 0; c < cols; c ++)
                                                {
                                                        boost::gil::argb8_pixel_t& pixel = view(c, r);
                                                        pixel[0] = color::make_alpha(rgba(r, c));
                                                        pixel[1] = color::make_red(rgba(r, c));
                                                        pixel[2] = color::make_green(rgba(r, c));
                                                        pixel[3] = color::make_blue(rgba(r, c));
                                                }
                                        }
                                }

                                boost::gil::png_write_view(path, view);
                        }
                        return true;

                case imagetype::jpeg: // boost::gil RGB encoding
                case imagetype::tif:
                        {
                                boost::gil::rgb8_image_t image(cols, rows);

                                boost::gil::rgb8_image_t::view_t view = boost::gil::view(image);
                                switch (mode)
                                {
                                case color_mode::luma:
                                        for (int r = 0; r < rows; r ++)
                                        {
                                                for (int c = 0; c < cols; c ++)
                                                {
                                                        boost::gil::rgb8_pixel_t& pixel = view(c, r);
                                                        pixel[0] = luma(r, c);
                                                        pixel[1] = luma(r, c);
                                                        pixel[2] = luma(r, c);
                                                }
                                        }
                                        break;

                                case color_mode::rgba:
                                        for (int r = 0; r < rows; r ++)
                                        {
                                                for (int c = 0; c < cols; c ++)
                                                {
                                                        boost::gil::rgb8_pixel_t& pixel = view(c, r);
                                                        pixel[0] = color::make_red(rgba(r, c));
                                                        pixel[1] = color::make_green(rgba(r, c));
                                                        pixel[2] = color::make_blue(rgba(r, c));
                                                }
                                        }
                                        break;
                                }

                                if (itype == imagetype::jpeg)
                                {
                                        boost::gil::jpeg_write_view(path, view);
                                }
                                else
                                {
                                        boost::gil::tiff_write_view(path, view);
                                }
                        }
                        return true;

                case imagetype::pgm:    // PGM binary encoding
                        {
                                std::ofstream os(path);

                                // write header
                                if (    !os.is_open() ||
                                        !(os << "P5" << std::endl) ||
                                        !(os << cols << " " << rows << std::endl) ||
                                        !(os << "255" << std::endl))
                                {
                                        return false;
                                }

                                // write pixels
                                luma_matrix_t grays(rows, cols);
                                switch (mode)
                                {
                                case color_mode::luma:
                                        grays = luma;
                                        break;

                                case color_mode::rgba:
                                        tensor::transform(rgba, grays, [] (rgba_t c) { return color::make_luma(c); });
                                        break;
                                }

                                return os.write(reinterpret_cast<const char*>(grays.data()),
                                                static_cast<std::streamsize>(grays.size()));
                        }
                        return true;

                case imagetype::unknown:
                default:
                        return false;
                }
        }

        image_t::image_t(coord_t rows, coord_t cols, color_mode mode)
                :       m_rows(rows),
                        m_cols(cols),
                        m_mode(mode)
        {
                resize(rows, cols, mode);
        }

        void image_t::resize(coord_t rows, coord_t cols, color_mode mode)
        {
                m_mode = mode;
                m_rows = rows;
                m_cols = cols;

                switch (m_mode)
                {
                case color_mode::luma:
                        m_luma.resize(rows, cols);
                        m_rgba.resize(0, 0);
                        break;

                case color_mode::rgba:
                        m_luma.resize(0, 0);
                        m_rgba.resize(rows, cols);
                        break;
                }
        }

        bool image_t::setup_rgba()
        {
                m_luma.resize(0, 0);
                m_mode = color_mode::rgba;
                m_rows = static_cast<coord_t>(m_rgba.rows());
                m_cols = static_cast<coord_t>(m_rgba.cols());
                return true;
        }

        bool image_t::setup_luma()
        {
                m_rgba.resize(0, 0);
                m_mode = color_mode::luma;
                m_rows = static_cast<coord_t>(m_luma.rows());
                m_cols = static_cast<coord_t>(m_luma.cols());
                return true;
        }

        bool image_t::load_rgba(const string_t& path)
        {
                return  load_image(path, color_mode::rgba, m_rgba, m_luma) &&
                        setup_rgba();
        }

        bool image_t::load_luma(const string_t& path)
        {
                return  load_image(path, color_mode::luma, m_rgba, m_luma) &&
                        setup_luma();
        }

        bool image_t::load_luma(const char* buffer, coord_t rows, coord_t cols)
        {
                const coord_t size = rows * cols;

                m_luma.resize(rows, cols);
                tensor::transform(tensor::make_vector(buffer, size),
                                  m_luma, [] (char luma) { return static_cast<luma_t>(luma); });

                return setup_luma();
        }

        bool image_t::load_rgba(const char* buffer, coord_t rows, coord_t cols, coord_t stride)
        {
                const coord_t size = rows * cols;

                m_rgba.resize(rows, cols);
                tensor::transform(tensor::make_vector(buffer + 0 * stride, size),
                                  tensor::make_vector(buffer + 1 * stride, size),
                                  tensor::make_vector(buffer + 2 * stride, size),
                                  m_rgba, [] (char r, char g, char b) { return color::make_rgba(r, g, b); });

                return setup_rgba();
        }

        bool image_t::load_rgba(const rgba_matrix_t& data)
        {
                m_rgba = data;

                return setup_rgba();
        }

        bool image_t::load_luma(const rgba_matrix_t& data)
        {
                m_luma.resize(data.rows(), data.cols());
                tensor::transform(data, m_luma, [] (rgba_t c) { return color::make_luma(c); });

                return setup_luma();
        }

        bool image_t::load_luma(const luma_matrix_t& data)
        {
                m_luma = data;

                return setup_luma();
        }

        bool image_t::load(const tensor_t& data)
        {
                const scalar_t scale = 255.0;

                if (data.dims() == 1)
                {
                        const auto gmap = data.plane_matrix(0);

                        m_luma.resize(data.rows(), data.cols());
                        tensor::transform(gmap, m_luma, [=] (scalar_t l)
                        {
                                const rgba_t luma = static_cast<rgba_t>(l * scale) & 0xFF;
                                return static_cast<luma_t>(luma);
                        });

                        return setup_luma();
                }

                else if (data.dims() == 3)
                {
                        const auto rmap = data.plane_matrix(0);
                        const auto gmap = data.plane_matrix(1);
                        const auto bmap = data.plane_matrix(2);

                        m_rgba.resize(data.rows(), data.cols());
                        tensor::transform(rmap, gmap, bmap, m_rgba, [=] (scalar_t r, scalar_t g, scalar_t b)
                        {
                                const rgba_t rr = static_cast<rgba_t>(r * scale) & 0xFF;
                                const rgba_t gg = static_cast<rgba_t>(g * scale) & 0xFF;
                                const rgba_t bb = static_cast<rgba_t>(b * scale) & 0xFF;
                                return color::make_rgba(rr, gg, bb);
                        });

                        return setup_rgba();
                }

                else
                {
                        return false;
                }
        }

        bool image_t::save(const string_t& path) const
        {
                return save_image(path, m_mode, m_rgba, m_luma);
        }

        tensor_t image_t::to_tensor() const
        {
                return to_tensor(rect_t(0, 0, cols(), rows()));
        }

        tensor_t image_t::to_tensor(const rect_t& region) const
        {
                const coord_t top = region.top();
                const coord_t left = region.left();
                const coord_t rows = region.rows();
                const coord_t cols = region.cols();

                const scalar_t scale = 1.0 / 255.0;

                switch (m_mode)
                {
                case color_mode::luma:
                        {
                                tensor_t data(1, rows, cols);
                                auto gmap = data.plane_matrix(0);

                                tensor::transform(m_luma.block(top, left, rows, cols), gmap, [=] (luma_t luma)
                                {
                                        return scale * luma;
                                });

                                return data;
                        }

                case color_mode::rgba:
                        {
                                tensor_t data(3, rows, cols);
                                auto rmap = data.plane_matrix(0);
                                auto gmap = data.plane_matrix(1);
                                auto bmap = data.plane_matrix(2);

                                tensor::transform(m_rgba.block(top, left, rows, cols), rmap, [=] (rgba_t rgba)
                                {
                                        return scale * color::make_red(rgba);
                                });
                                tensor::transform(m_rgba.block(top, left, rows, cols), gmap, [=] (rgba_t rgba)
                                {
                                        return scale * color::make_green(rgba);
                                });
                                tensor::transform(m_rgba.block(top, left, rows, cols), bmap, [=] (rgba_t rgba)
                                {
                                        return scale * color::make_blue(rgba);
                                });

                                return data;
                        }
                        break;

                default:
                        return tensor_t();
                }
        }

        bool image_t::make_rgba()
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        m_rgba.resize(rows(), cols());
                        tensor::transform(m_luma, m_rgba, [] (luma_t g) { return color::make_rgba(g, g, g); });

                        return setup_rgba();

                case color_mode::rgba:
                        return true;

                default:
                        return false;
                }
        }

        bool image_t::make_luma()
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        return true;

                case color_mode::rgba:
                        m_luma.resize(rows(), cols());
                        tensor::transform(m_rgba, m_luma, [] (rgba_t c) { return color::make_luma(c); });

                        return setup_luma();

                default:
                        return false;
                }
        }

        bool image_t::fill(rgba_t rgba)
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        m_luma.setConstant(color::make_luma(rgba));
                        return true;

                case color_mode::rgba:
                        m_rgba.setConstant(rgba);
                        return true;

                default:
                        return false;
                }
        }

        bool image_t::fill(luma_t luma)
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        m_luma.setConstant(luma);
                        return true;

                case color_mode::rgba:
                        m_rgba.setConstant(color::make_rgba(luma, luma, luma));
                        return true;

                default:
                        return false;
                }
        }

        bool image_t::copy(coord_t top, coord_t left, const rgba_matrix_t& patch)
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        {
                                luma_matrix_t luma(patch.rows(), patch.cols());
                                tensor::transform(patch, luma, [] (rgba_t rgba) { return color::make_luma(rgba); });
                                m_luma.block(top, left, patch.rows(), patch.cols()) = luma;
                        }
                        return true;

                case color_mode::rgba:
                        m_rgba.block(top, left, patch.rows(), patch.cols()) = patch;
                        return true;

                default:
                        return false;
                }
        }

        bool image_t::copy(coord_t top, coord_t left, const luma_matrix_t& patch)
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        m_luma.block(top, left, patch.rows(), patch.cols()) = patch;
                        return true;

                case color_mode::rgba:
                        {
                                rgba_matrix_t rgba(patch.rows(), patch.cols());
                                tensor::transform(patch, rgba, [] (luma_t luma) { return color::make_rgba(luma, luma, luma); });
                                m_rgba.block(top, left, patch.rows(), patch.cols()) = rgba;
                        }
                        return true;

                default:
                        return false;
                }
        }

        bool image_t::copy(coord_t top, coord_t left, const image_t& image)
        {
                switch (image.m_mode)
                {
                case color_mode::luma:
                        return copy(top, left, image.m_luma);

                case color_mode::rgba:
                        return copy(top, left, image.m_rgba);

                default:
                        return false;
                }
        }

        bool image_t::copy(coord_t top, coord_t left, const image_t& image, const rect_t& region)
        {
                switch (image.m_mode)
                {
                case color_mode::luma:
                        return  copy(top, left, luma_matrix_t(image.m_luma.block(
                                region.top(), region.left(), region.rows(), region.cols())));

                case color_mode::rgba:
                        return  copy(top, left, rgba_matrix_t(image.m_rgba.block(
                                region.top(), region.left(), region.rows(), region.cols())));

                default:
                        return false;
                }
        }

        bool image_t::set(coord_t row, coord_t col, rgba_t rgba)
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        m_luma(row, col) = color::make_luma(rgba);
                        return true;

                case color_mode::rgba:
                        m_rgba(row, col) = rgba;
                        return true;

                default:
                        return false;
                }
        }

        bool image_t::set(coord_t row, coord_t col, luma_t luma)
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        m_luma(row, col) = luma;
                        return true;

                case color_mode::rgba:
                        m_rgba(row, col) = color::make_rgba(luma, luma, luma);
                        return true;

                default:
                        return false;
                }
        }

        void image_t::transpose_in_place()
        {
                m_rgba.transposeInPlace();
                m_luma.transposeInPlace();
        }

        bool image_t::scale(scalar_t factor)
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        {
                                luma_matrix_t luma_scaled;
                                ncv::bilinear(m_luma, luma_scaled, factor);

                                m_luma = luma_scaled;
                        }

                        return setup_luma();

                case color_mode::rgba:
                        {
                                cielab_matrix_t cielab(rows(), cols());
                                tensor::transform(m_rgba, cielab, color::make_cielab);

                                cielab_matrix_t cielab_scaled;
                                ncv::bilinear(cielab, cielab_scaled, factor);

                                m_rgba.resize(cielab_scaled.rows(), cielab_scaled.cols());
                                tensor::transform(cielab_scaled, m_rgba, [] (const cielab_t& lab) { return color::make_rgba(lab); });
                        }

                        return setup_rgba();

                default:
                        return false;
                }
        }

        bool image_t::random()
        {
                switch (m_mode)
                {
                case color_mode::luma:
                        m_luma.setRandom();
                        return true;

                case color_mode::rgba:
                        m_rgba.setRandom();
                        return true;

                default:
                        return false;
                }
        }
}
