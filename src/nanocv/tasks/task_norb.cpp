#include "task_norb.h"
#include "common/logger.h"
#include "common/math.hpp"
#include "loss.h"
#include <fstream>

namespace ncv
{
        static const strings_t tlabels =
        {
                "animal",
                "human",
                "plane",
                "truck",
                "car",
                "blank"
        };

        bool norb_task_t::load(const string_t& dir)
        {
                const size_t n_train_samples = 29160 * 10;
                const size_t n_test_samples = 29160 * 2;

                clear_memory(n_train_samples + n_test_samples);

                return  load(dir + "/norb-5x46789x9x18x6x2x108x108-training-01", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-02", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-03", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-04", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-05", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-06", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-07", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-08", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-09", protocol::train) +
                        load(dir + "/norb-5x46789x9x18x6x2x108x108-training-10", protocol::train) == n_train_samples &&

                        load(dir + "/norb-5x01235x9x18x6x2x108x108-testing-01", protocol::test) +
                        load(dir + "/norb-5x01235x9x18x6x2x108x108-testing-02", protocol::test) == n_test_samples;
        }

        static bool read_header(std::ifstream& file, int32_t& magic, std::vector<int32_t>& dims)
        {
                // read data type & #dimensions
                int32_t ndims;
                if (    !file.read((char*)&magic, sizeof(int32_t)) ||
                        !file.read((char*)&ndims, sizeof(int32_t)))
                {
                        return false;
                }

                // read 3D/4D dimensions
                int32_t dim1, dim2, dim3, dim4;
                if (    !file.read((char*)&dim1, sizeof(int32_t)) ||
                        !file.read((char*)&dim2, sizeof(int32_t)) ||
                        !file.read((char*)&dim3, sizeof(int32_t)) ||
                        (ndims > 3 && !file.read((char*)&dim4, sizeof(int32_t))))
                {
                        return false;
                }

                // OK
                dims.push_back(dim1);
                dims.push_back(dim2);
                dims.push_back(dim3);
                if (ndims > 3)
                {
                        dims.push_back(dim4);
                }

                return true;
        }

        size_t norb_task_t::load(const string_t& bfile, protocol p)
        {
                return load(bfile + "-cat.mat", bfile + "-dat.mat", p);
        }

        size_t norb_task_t::load(const string_t& cfile, const string_t& dfile, protocol p)
        {
                log_info() << "NORB: loading cat-file <" << cfile << "> ...";
                log_info() << "NORB: loading dat-file <" << dfile << "> ...";

                // image and label data streams
                std::ifstream fimage(dfile.c_str(), std::ios::in | std::ios::binary);
                std::ifstream flabel(cfile.c_str(), std::ios::in | std::ios::binary);

                if (!fimage.is_open() || !flabel.is_open())
                {
                        log_error() << "NORB: failed to open files!";
                        return 0;
                }

//                static const int magic_f32 = 0x1E3D4C51;
//                static const int magic_f64 = 0x1E3D4C53;
                static const int magic_i32 = 0x1E3D4C54;
                static const int magic_i08 = 0x1E3D4C55;
//                static const int magic_i16 = 0x1E3D4C56;

                // read headers
                int32_t magic_image, magic_label;
                std::vector<int32_t> dims_image, dims_label;
                if (    !read_header(fimage, magic_image, dims_image) ||
                        !read_header(flabel, magic_label, dims_label))
                {
                        log_error() << "NORB: failed to read headers!";
                        return 0;
                }

                if (    magic_image != magic_i08 ||
                        magic_label != magic_i32 ||

                        dims_image.size() != 4 ||
                        dims_label.size() != 3 ||

                        dims_image[0] != dims_label[0] ||
                        dims_image[1] != 2 ||
                        dims_image[2] != static_cast<int>(n_rows()) ||
                        dims_image[3] != static_cast<int>(n_cols()) ||

                        dims_label[1] != 1)
                {
                        log_error() << "NORB: invalid headers!";
                        return 0;
                }

                // load annotations and images (as binary data)
                const size_t n_cameras = 2;
                const size_t n_pixels = n_rows() * n_cols();
                const size_t cnt = dims_image[0];

                std::vector<char> dimage(cnt * n_pixels * n_cameras);
                std::vector<int32_t> dlabel(cnt);

                if (    !fimage.read((char*)&dimage[0], sizeof(int8_t) * dimage.size()) ||
                        !flabel.read((char*)&dlabel[0], sizeof(int32_t) * dlabel.size()))
                {
                        log_error() << "NORB: failed to read data!";
                        return 0;
                }

                // decode annotations and images
                for (size_t i = 0; i < cnt; i ++)
                {
                        const size_t ilabel = dlabel[i];
                        if (ilabel >= n_outputs())
                        {
                                continue;
                        }

                        for (size_t camera = 0; camera < n_cameras; camera ++)
                        {
                                sample_t sample(m_images.size(), sample_region(0, 0));
                                sample.m_label = tlabels[ilabel];
                                sample.m_target = ncv::class_target(ilabel, n_outputs());
                                sample.m_fold = { 0, p };
                                m_samples.push_back(sample);

                                image_t image;
                                image.load_luma(&dimage[i * n_pixels * n_cameras + camera], n_rows(), n_cols());
                                m_images.push_back(image);
                        }
                }

                log_info() << "NORB: loaded " << cnt << " samples.";

                return cnt;
        }
}