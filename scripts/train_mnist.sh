#!/bin/bash

source common.sh

# common parameters
params=""
params=${params}${task_mnist}
params=${params}" --loss logistic --trials 1 --threads ${max_threads}"

# models
conv100_max="--model forward-network --model-params "
conv100_max=${conv100_max}"conv:dims=16,rows=9,cols=9,mask=100;act-snorm;pool-max;"
conv100_max=${conv100_max}"conv:dims=32,rows=5,cols=5,mask=100;act-snorm;pool-max;"
conv100_max=${conv100_max}"conv:dims=64,rows=3,cols=3,mask=100;act-snorm;"

conv50_max="--model forward-network --model-params "
conv50_max=${conv50_max}"conv:dims=16,rows=9,cols=9,mask=50;act-snorm;pool-max;"
conv50_max=${conv50_max}"conv:dims=32,rows=5,cols=5,mask=50;act-snorm;pool-max;"
conv50_max=${conv50_max}"conv:dims=64,rows=3,cols=3,mask=50;act-snorm;"

conv25_max="--model forward-network --model-params "
conv25_max=${conv25_max}"conv:dims=16,rows=9,cols=9,mask=25;act-snorm;pool-max;"
conv25_max=${conv25_max}"conv:dims=32,rows=5,cols=5,mask=25;act-snorm;pool-max;"
conv25_max=${conv25_max}"conv:dims=64,rows=3,cols=3,mask=25;act-snorm;"

mlp0="--model forward-network --model-params "
mlp1=${mlp0}"linear:dims=128;act-snorm;"
mlp2=${mlp1}"linear:dims=64;act-snorm;"
mlp3=${mlp2}"linear:dims=32;act-snorm;"

outlayer="linear:dims=10;"

# trainers
stoch_sg="--trainer stochastic --trainer-params opt=sg,epoch=16"
stoch_sga="--trainer stochastic --trainer-params opt=sga,epoch=16"
stoch_sia="--trainer stochastic --trainer-params opt=sia,epoch=16"
stoch_nag="--trainer stochastic --trainer-params opt=nag,epoch=16"

mbatch0_lbfgs="--trainer minibatch --trainer-params opt=lbfgs,epoch=128,batch=1024,ratio=1.0,iters=8,eps=1e-6"
mbatch0_cgd="--trainer minibatch --trainer-params opt=cgd,epoch=128,batch=1024,ratio=1.0,iters=8,eps=1e-6"
mbatch0_gd="--trainer minibatch --trainer-params opt=gd,epoch=128,batch=1024,ratio=1.0,iters=8,eps=1e-6"

mbatch1_lbfgs="--trainer minibatch --trainer-params opt=lbfgs,epoch=128,batch=1024,ratio=1.05,iters=8,eps=1e-6"
mbatch1_cgd="--trainer minibatch --trainer-params opt=cgd,epoch=128,batch=1024,ratio=1.05,iters=8,eps=1e-6"
mbatch1_gd="--trainer minibatch --trainer-params opt=gd,epoch=128,batch=1024,ratio=1.05,iters=8,eps=1e-6"

mbatch2_lbfgs="--trainer minibatch --trainer-params opt=lbfgs,epoch=128,batch=1024,ratio=1.1,iters=8,eps=1e-6"
mbatch2_cgd="--trainer minibatch --trainer-params opt=cgd,epoch=128,batch=1024,ratio=1.1,iters=8,eps=1e-6"
mbatch2_gd="--trainer minibatch --trainer-params opt=gd,epoch=128,batch=1024,ratio=1.1,iters=8,eps=1e-6"

batch_lbfgs="--trainer batch --trainer-params opt=lbfgs,iters=128,eps=1e-6"
batch_cgd="--trainer batch --trainer-params opt=cgd,iters=128,eps=1e-6"
batch_gd="--trainer batch --trainer-params opt=gd,iters=128,eps=1e-6"

# train models
for model in `echo "mlp0 mlp1"` # mlp2 mlp3 conv100_max conv50_max conv25_max"`
do
        for trainer in `echo "mbatch0_gd mbatch0_cgd mbatch0_lbfgs mbatch1_gd mbatch1_cgd mbatch1_lbfgs mbatch2_gd mbatch2_cgd mbatch2_lbfgs batch_gd batch_cgd batch_lbfgs"`
        do
                fn_train ${dir_exp_mnist} ${trainer}_${model} ${params} ${!trainer} ${avg_crit} ${!model}${outlayer}
                #fn_train ${dir_exp_mnist} ${trainer}_${model}_l2n ${params} ${!trainer} ${l2n_crit} ${!model}${outlayer}
                #fn_train ${dir_exp_mnist} ${trainer}_${model}_var ${params} ${!trainer} ${var_crit} ${!model}${outlayer}
        done
        #for trainer in `echo "stoch_sg stoch_sga stoch_sia stoch_nag"`
        #do
        #        fn_train ${dir_exp_mnist} ${trainer}_${model} ${params} ${!trainer} ${avg_crit} ${!model}${outlayer}
        #        fn_train ${dir_exp_mnist} ${trainer}_${model}_l2n ${params} ${!trainer} ${l2n_crit} ${!model}${outlayer}
        #done
done

exit

# compare models
bash plot_models.sh ${dir_exp_mnist}/models.pdf ${dir_exp_mnist}/*.state

# bash plot_models.sh ${dir_exp_mnist}/conv_max_models.pdf ${dir_exp_mnist}/conv_max_*.state
# bash plot_models.sh ${dir_exp_mnist}/conv_min_models.pdf ${dir_exp_mnist}/conv_min_*.state
# bash plot_models.sh ${dir_exp_mnist}/conv_avg_models.pdf ${dir_exp_mnist}/conv_avg_*.state

