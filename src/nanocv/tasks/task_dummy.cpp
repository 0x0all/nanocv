#include "task_dummy.h"
#include "loss.h"
#include "util/random.hpp"

namespace ncv
{
        dummy_task_t::dummy_task_t(const string_t& configuration)
                :       task_t(configuration),
                        m_rows(28),
                        m_cols(28),
                        m_outputs(10),
                        m_folds(1),
                        m_color(color_mode::luma),
                        m_size(1000)
        {
        }

        void dummy_task_t::set_rows(size_t rows)
        {
                m_rows = rows;
        }

        void dummy_task_t::set_cols(size_t cols)
        {
                m_cols = cols;
        }

        void dummy_task_t::set_outputs(size_t outputs)
        {
                m_outputs = outputs;
        }

        void dummy_task_t::set_folds(size_t folds)
        {
                m_folds = folds;
        }

        void dummy_task_t::set_color(color_mode color)
        {
                m_color = color;
        }

        void dummy_task_t::set_size(size_t size)
        {
                m_size = size;
        }

        void dummy_task_t::setup()
        {
                random_t<size_t> rng(1, 100);

                m_images.clear();
                m_samples.clear();

                for (size_t f = 0; f < n_folds(); f ++)
                {
                        for (protocol p : {protocol::train, protocol::test})
                        {
                                for (size_t i = 0; i < m_size / 2; i ++)
                                {
                                        const size_t ilabel = (rng() < 90) ? 0 : (rng() % n_outputs());

                                        sample_t sample(m_images.size(), sample_region(0, 0));
                                        sample.m_label = "label" + text::to_string(ilabel);
                                        sample.m_target = ncv::class_target(ilabel, n_outputs());
                                        sample.m_fold = {f, p};
                                        m_samples.push_back(sample);

                                        image_t image(n_rows(), n_cols(), color());
                                        image.random();
                                        m_images.push_back(image);
                                }
                        }
                }
        }
}
