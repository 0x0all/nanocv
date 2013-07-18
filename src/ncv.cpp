#include "ncv.h"
#include <cfenv>

#include "loss/loss_classnll.h"
#include "loss/loss_hinge.h"
#include "loss/loss_logistic.h"
#include "loss/loss_square.h"

#include "task/task_cifar10.h"
#include "task/task_mnist.h"
#include "task/task_stl10.h"
#include "task/task_cmufaces.h"

#include "activation/activation_unit.h"
#include "activation/activation_tanh.h"
#include "activation/activation_fun1.h"
#include "activation/activation_fun2.h"

#include "model/conv_network.h"

namespace ncv
{
        //-------------------------------------------------------------------------------------------------

        void init()
        {
                // round to nearest integer
                std::fesetround(FE_TONEAREST);

                // register losses
                loss_manager_t::instance().add("classnll", classnll_loss_t());
                loss_manager_t::instance().add("hinge", hinge_loss_t());
                loss_manager_t::instance().add("logistic", logistic_loss_t());
                loss_manager_t::instance().add("square", square_loss_t());

                // register tasks
                task_manager_t::instance().add("mnist", mnist_task_t());
                task_manager_t::instance().add("cifar10", cifar10_task_t());
                task_manager_t::instance().add("stl10", stl10_task_t());
                task_manager_t::instance().add("cmu-faces", cmufaces_task_t());

                // register activation/transfer functions
                activation_manager_t::instance().add("unit", unit_activation_t());
                activation_manager_t::instance().add("tanh", tanh_activation_t());
                activation_manager_t::instance().add("fun1", fun1_activation_t());
                activation_manager_t::instance().add("fun2", fun2_activation_t());

                // register models
//                model_manager_t::instance().add("convnet", conv_network_model_t());
        }

        //-------------------------------------------------------------------------------------------------

        size_t test(const model_t& model, const task_t& task, const fold_t& fold, const loss_t& loss,
                scalar_t& lvalue, scalar_t& lerror)
        {
                lvalue = lerror = 0.0;
                size_t count = 0;

                const samples_t& samples = task.samples(fold);
                for (size_t i = 0; i < samples.size(); i ++)
                {
                        const sample_t& sample = samples[i];
                        const image_t& image = task.image(sample.m_index);

                        const vector_t target = image.make_target(sample.m_region);
                        if (image.has_target(target))
                        {
                                const vector_t output = model.value(image, sample.m_region);

                                lvalue += loss.value(target, output);
                                lerror += loss.error(target, output);
                                ++ count;
                        }
                }

                if (count > 0)
                {
                        lvalue /= count;
                        lerror /= count;
                }

                return count;
        }

        //-------------------------------------------------------------------------------------------------
}
	
