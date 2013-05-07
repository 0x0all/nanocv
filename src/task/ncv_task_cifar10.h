#ifndef NANOCV_TASK_CIFAR10_H
#define NANOCV_TASK_CIFAR10_H

#include "ncv_task.h"

namespace ncv
{
        ////////////////////////////////////////////////////////////////////////////////
        // CIFAR10 task:
        //      - object classification
        //      - 32x32 color images as inputs
        //      - 10 outputs (10 labels)
        ////////////////////////////////////////////////////////////////////////////////
	
        class cifar10_task_t : public task_t
        {
        public:

                // create an object clone
                virtual rtask_t clone(const string_t& /*params*/) const
                {
                        return rtask_t(new cifar10_task_t(*this));
                }

                // describe the object
                virtual const char* name() const { return "cifar10"; }
                virtual const char* desc() const { return "CIFAR-10 (object classification)"; }

                // load images from the given directory
                virtual bool load(const string_t& dir);

                // load sample patch
                virtual void load(const image_sample_t& isample, sample_t& sample) const;

                // access functions
                virtual size_t n_rows() const { return 32; }
                virtual size_t n_cols() const { return 32; }
                virtual size_t n_inputs() const { return n_rows() * n_cols() * 3; }
                virtual size_t n_outputs() const { return 10; }

                virtual size_t n_images() const { return m_images.size(); }
                virtual const annotated_image_t& image(index_t i) const { return m_images[i]; }

                virtual size_t n_folds() const { return 1; }
                virtual const image_samples_t& fold(const fold_t& fold) const { return m_folds.find(fold)->second; }

        private:

                // load binary file
                size_t load(const string_t& bfile, protocol p);

                // build folds
                bool build_folds(size_t n_train_images, size_t n_test_images);

        private:

                // attributes
                annotated_images_t      m_images;
                fold_image_samples_t    m_folds;
        };
}

#endif // NANOCV_TASK_CIFAR10_H
