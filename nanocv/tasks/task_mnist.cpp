#include "task_mnist.h"
#include "nanocv/loss.h"
#include "nanocv/logger.h"
#include "nanocv/math/cast.hpp"
#include "nanocv/file/stream.h"
#include "nanocv/file/archive.h"

namespace ncv
{
        mnist_task_t::mnist_task_t(const string_t& configuration)
                :       task_t(configuration)
        {
        }

        bool mnist_task_t::load(const string_t& dir)
        {
                const string_t test_ifile = dir + "/t10k-images-idx3-ubyte.gz";
                const string_t test_gfile = dir + "/t10k-labels-idx1-ubyte.gz";
                const size_t n_test_samples = 10000;

                const string_t train_ifile = dir + "/train-images-idx3-ubyte.gz";
                const string_t train_gfile = dir + "/train-labels-idx1-ubyte.gz";
                const size_t n_train_samples = 60000;

                clear_memory(n_train_samples + n_test_samples);

                return  load(train_ifile, train_gfile, protocol::train, n_train_samples) &&
                        load(test_ifile, test_gfile, protocol::test, n_test_samples);
        }

        bool mnist_task_t::load(const string_t& ifile, const string_t& gfile, protocol p, size_t count)
        {
                size_t iindex = n_images();
                size_t icount = 0;
                size_t gcount = 0;

                std::vector<char> buffer(irows() * icols());
                char label[2];

                // load images
                const auto iop = [&] (const string_t&, const io::data_t& data)
                {
                        io::stream_t stream(data.data(), data.size());

                        stream.read(buffer.data(), 16);
                        while (stream.read(buffer.data(), buffer.size()))
                        {
                                image_t image;
                                image.load_luma(buffer.data(), irows(), icols());
                                add_image(image);

                                ++ icount;
                        }                        
                        
                        return true;
                };

                log_info() << "MNIST: loading file <" << ifile << "> ...";
                if (!io::decode(ifile, "MNIST: ", iop))
                {
                        log_error() << "MNIST: failed to load file <" << ifile << ">!";
                        return false;
                }

                // load ground truth
                const auto gop = [&] (const string_t&, const io::data_t& data)
                {
                        io::stream_t stream(data.data(), data.size());

                        stream.read(buffer.data(), 8);
                        while (stream.read(label, 1) && stream.gcount() == 1)
                        {
                                const size_t ilabel = math::cast<size_t>(label[0]);
                                if (ilabel >= osize())
                                {
                                        continue;
                                }

                                sample_t sample(iindex, sample_region(0, 0));
                                sample.m_label = "digit" + text::to_string(ilabel);
                                sample.m_target = ncv::class_target(ilabel, osize());
                                sample.m_fold = { 0, p };
                                add_sample(sample);

                                ++ gcount;
                                ++ iindex;
                        }
                        
                        return true;
                };

                log_info() << "MNIST: loading file <" << gfile << "> ...";
                if (!io::decode(gfile, "MNIST: ", gop))
                {
                        log_error() << "MNIST: failed to load file <" << gfile << ">!";
                        return false;
                }

                // OK
                log_info() << "MNIST: loaded " << icount << "/" << gcount << " samples.";
                return (count == gcount) && (count == icount);
        }
}
