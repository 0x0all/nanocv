#include "task_cifar100.h"
#include "common/logger.h"
#include "common/cast.hpp"
#include "common/io_arch.h"
#include "common/io_stream.h"
#include "loss.h"

namespace ncv
{
        static const string_t tlabels[] =
        {
                "apple",
                "aquarium_fish",
                "baby",
                "bear",
                "beaver",
                "bed",
                "bee",
                "beetle",
                "bicycle",
                "bottle",
                "bowl",
                "boy",
                "bridge",
                "bus",
                "butterfly",
                "camel",
                "can",
                "castle",
                "caterpillar",
                "cattle",
                "chair",
                "chimpanzee",
                "clock",
                "cloud",
                "cockroach",
                "couch",
                "crab",
                "crocodile",
                "cup",
                "dinosaur",
                "dolphin",
                "elephant",
                "flatfish",
                "forest",
                "fox",
                "girl",
                "hamster",
                "house",
                "kangaroo",
                "keyboard",
                "lamp",
                "lawn_mower",
                "leopard",
                "lion",
                "lizard",
                "lobster",
                "man",
                "maple_tree",
                "motorcycle",
                "mountain",
                "mouse",
                "mushroom",
                "oak_tree",
                "orange",
                "orchid",
                "otter",
                "palm_tree",
                "pear",
                "pickup_truck",
                "pine_tree",
                "plain",
                "plate",
                "poppy",
                "porcupine",
                "possum",
                "rabbit",
                "raccoon",
                "ray",
                "road",
                "rocket",
                "rose",
                "sea",
                "seal",
                "shark",
                "shrew",
                "skunk",
                "skyscraper",
                "snail",
                "snake",
                "spider",
                "squirrel",
                "streetcar",
                "sunflower",
                "sweet_pepper",
                "table",
                "tank",
                "telephone",
                "television",
                "tiger",
                "tractor",
                "train",
                "trout",
                "tulip",
                "turtle",
                "wardrobe",
                "whale",
                "willow_tree",
                "wolf",
                "woman",
                "worm"
        };

        cifar100_task_t::cifar100_task_t(const string_t& configuration)
                :       task_t(configuration)
        {
        }

        bool cifar100_task_t::load(const string_t& dir)
        {
                const string_t bfile = dir + "/cifar-100-binary.tar.gz";

                const string_t train_bfile = "train.bin";
                const size_t n_train_samples = 50000;

                const string_t test_bfile = "test.bin";
                const size_t n_test_samples = 10000;

                clear_memory(n_train_samples + n_test_samples);

                const auto op = [&] (const string_t& filename, const io::data_t& data)
                {
                        if (boost::algorithm::iends_with(filename, train_bfile))
                        {
                                log_info() << "CIFAR-100: loading file <" << filename << "> ...";
                                load(data, protocol::train);
                        }

                        else if (boost::algorithm::iends_with(filename, test_bfile))
                        {
                                log_info() << "CIFAR-100: loading file <" << filename << "> ...";
                                load(data, protocol::test);
                        }
                };

                log_info() << "CIFAR-100: loading file <" << bfile << "> ...";

                return  io::decode(bfile, "CIFAR-100: ", op) &&
                        m_samples.size() == n_train_samples + n_test_samples &&
                        m_images.size() == n_train_samples + n_test_samples;
        }
        
        size_t cifar100_task_t::load(const io::data_t& data, protocol p)
        {
                std::vector<char> vbuffer(n_rows() * n_cols() * 3);
                char* buffer = vbuffer.data();
                char label[2];

                io::stream_t stream(data);

                size_t cnt = 0;
                while ( stream.read(label, 2) &&       // coarse & fine labels!
                        stream.read(buffer, vbuffer.size()))
                {
                        const size_t ilabel = math::cast<size_t>(label[1]);

                        sample_t sample(m_images.size(), sample_region(0, 0));
                        sample.m_label = tlabels[ilabel];
                        sample.m_target = ncv::class_target(ilabel, n_outputs());
                        sample.m_fold = { 0, p };
                        m_samples.push_back(sample);

                        image_t image;
                        image.load_rgba(buffer, n_rows(), n_cols(), n_rows() * n_cols());
                        m_images.push_back(image);

                        ++ cnt;
                }

                log_info() << "CIFAR-100: loaded " << cnt << " samples.";

                return cnt;
        }
}
