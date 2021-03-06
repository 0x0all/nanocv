#!/bin/bash

source common_train.sh

# common parameters
params=""
params=${params}${task_mnist}
params=${params}" --loss classnll --trials 10 --threads ${max_threads}"

# models
conv_max="--model forward-network --model-params "
conv_max=${conv_max}"conv:dims=16,rows=9,cols=9;pool-max;act-snorm;"
conv_max=${conv_max}"conv:dims=32,rows=5,cols=5;pool-max;act-snorm;"
conv_max=${conv_max}"conv:dims=64,rows=3,cols=3;act-snorm;"

mlp0="--model forward-network --model-params "
mlp1=${mlp0}"linear:dims=128;act-snorm;"
mlp2=${mlp1}"linear:dims=64;act-snorm;"
mlp3=${mlp2}"linear:dims=32;act-snorm;"

outlayer="linear:dims=10;"

# trainers
stoch_ag="--trainer stochastic --trainer-params opt=ag,epoch=32"
stoch_sg="--trainer stochastic --trainer-params opt=sg,epoch=32"
stoch_sga="--trainer stochastic --trainer-params opt=sga,epoch=32"
stoch_sia="--trainer stochastic --trainer-params opt=sia,epoch=32"
stoch_adagrad="--trainer stochastic --trainer-params opt=adagrad,epoch=32"
stoch_adadelta="--trainer stochastic --trainer-params opt=adadelta,epoch=32"

mbatch_gd="--trainer minibatch --trainer-params opt=gd,epoch=32,eps=1e-4"
mbatch_cgd="--trainer minibatch --trainer-params opt=cgd,epoch=32,eps=1e-4"
mbatch_lbfgs="--trainer minibatch --trainer-params opt=lbfgs,epoch=32,eps=1e-4"

batch_gd="--trainer batch --trainer-params opt=gd,iters=32,eps=1e-4"
batch_cgd="--trainer batch --trainer-params opt=cgd,iters=32,eps=1e-4"
batch_lbfgs="--trainer batch --trainer-params opt=lbfgs,iters=32,eps=1e-4"

models="mlp0 mlp1 mlp2 mlp3 conv_max"

# train models
for model in ${models}
do
        #for trainer in `echo "batch_gd batch_cgd batch_lbfgs"`
        for trainer in `echo "mbatch_gd mbatch_cgd mbatch_lbfgs"`
	do
                fn_train ${dir_exp_mnist} ${trainer}_${model} ${params} ${!trainer} ${avg_crit} ${!model}${outlayer}
		fn_train ${dir_exp_mnist} ${trainer}_${model}_l2n ${params} ${!trainer} ${l2n_crit} ${!model}${outlayer}
	        fn_train ${dir_exp_mnist} ${trainer}_${model}_var ${params} ${!trainer} ${var_crit} ${!model}${outlayer}
        done

#        for trainer in `echo "stoch_ag stoch_adagrad stoch_adadelta stoch_sg stoch_sga stoch_sia"`
#        do
#                #fn_train ${dir_exp_mnist} ${trainer}_${model} ${params} ${!trainer} ${avg_crit} ${!model}${outlayer}
#        	#fn_train ${dir_exp_mnist} ${trainer}_${model}_l2n ${params} ${!trainer} ${l2n_crit} ${!model}${outlayer}
#        	#fn_train ${dir_exp_mnist} ${trainer}_${model}_var ${params} ${!trainer} ${var_crit} ${!model}${outlayer}
#        done
done

# compare optimizers
for model in ${models}
do
        bash plot_models.sh ${dir_exp_mnist}/${model}.pdf ${dir_exp_mnist}/*_${model}*.state
	bash plot_models.sh ${dir_exp_mnist}/${model}_stoch.pdf ${dir_exp_mnist}/stoch_*_${model}*.state
	bash plot_models.sh ${dir_exp_mnist}/${model}_mbatch.pdf ${dir_exp_mnist}/mbatch_*_${model}*.state
done

# compare models
# bash plot_models.sh ${dir_exp_mnist}/conv_max_models.pdf ${dir_exp_mnist}/conv_max_*.state
# bash plot_models.sh ${dir_exp_mnist}/conv_min_models.pdf ${dir_exp_mnist}/conv_min_*.state
# bash plot_models.sh ${dir_exp_mnist}/conv_avg_models.pdf ${dir_exp_mnist}/conv_avg_*.state

