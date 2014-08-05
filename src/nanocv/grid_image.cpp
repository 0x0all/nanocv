#include "grid_image.h"

namespace ncv
{
        grid_image_t::grid_image_t(
                size_t patch_rows, size_t patch_cols,
                size_t group_rows, size_t group_cols,
                size_t border,
                rgba_t back_color)
                :       m_prows(patch_rows),
                        m_pcols(patch_cols),
                        m_grows(group_rows),
                        m_gcols(group_cols),
                        m_border(border),
                        m_bcolor(back_color)
        {
                const size_t rows = m_prows * m_grows + m_border * (m_grows + 1);
                const size_t cols = m_pcols * m_gcols + m_border * (m_gcols + 1);

                m_image.resize(rows, cols, color_mode::rgba);
                m_image.fill(m_bcolor);
        }

        bool grid_image_t::set(size_t grow, size_t gcol, const image_t& image)
        {
                return set(grow, gcol, image, geom::make_rect(0, 0, image.cols(), image.rows()));
        }

        bool grid_image_t::set(size_t grow, size_t gcol, const image_t& image, const rect_t& region)
        {
                if (    grow < m_grows &&
                        gcol < m_gcols &&
                        static_cast<size_t>(geom::rows(region)) == m_prows &&
                        static_cast<size_t>(geom::cols(region)) == m_pcols)
                {
                        const size_t iy = m_prows * grow + m_border * (grow + 1);
                        const size_t ix = m_pcols * gcol + m_border * (gcol + 1);

                        return m_image.copy(iy, ix, image, region);
                }
                else
                {
                        return false;
                }
        }
}