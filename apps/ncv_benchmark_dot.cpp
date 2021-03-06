#include "nanocv/tensor.h"
#include "nanocv/tabulator.h"
#include "nanocv/measure.hpp"
#include "nanocv/math/dot.hpp"
#include "nanocv/tensor/dot.hpp"
#include <iostream>

using namespace ncv;

template
<
        typename top,
        typename tvector,
        typename tscalar = typename tvector::Scalar
>
static void test_dot(tabulator_t::row_t& row, top op, const tvector& vec1, const tvector& vec2)
{
        const size_t trials = 16;

        row << ncv::measure_robustly_usec([&] ()
        {
                const volatile tscalar ret = op(vec1.data(), vec2.data(), vec1.size());
                ret;
        }, trials);
}

static void test_dot(size_t size, tabulator_t::row_t& row)
{
        vector_t vec1(size), vec2(size);
        vec1.setRandom();
        vec2.setRandom();

        test_dot(row, ncv::math::dot<scalar_t>, vec1, vec2);
        test_dot(row, ncv::math::dot_unroll<scalar_t, 2>, vec1, vec2);
        test_dot(row, ncv::math::dot_unroll<scalar_t, 3>, vec1, vec2);
        test_dot(row, ncv::math::dot_unroll<scalar_t, 4>, vec1, vec2);
        test_dot(row, ncv::math::dot_unroll<scalar_t, 5>, vec1, vec2);
        test_dot(row, ncv::math::dot_unroll<scalar_t, 6>, vec1, vec2);
        test_dot(row, ncv::math::dot_unroll<scalar_t, 7>, vec1, vec2);
        test_dot(row, ncv::math::dot_unroll<scalar_t, 8>, vec1, vec2);
        test_dot(row, ncv::tensor::dot<scalar_t>, vec1, vec2);
}

int main(int, char* [])
{
        static const size_t min_size = 32 * 1024;
        static const size_t max_size = 4 * 1024 * 1024;

        tabulator_t table("size\\dot");

        table.header() << "dot [us]"
                       << "dotul2 [us]"
                       << "dotul3 [us]"
                       << "dotul4 [us]"
                       << "dotul5 [us]"
                       << "dotul6 [us]"
                       << "dotul7 [us]"
                       << "dotul8 [us]"
                       << "doteig [us]";

        for (size_t size = min_size; size <= max_size; size *= 2)
        {
                tabulator_t::row_t& row = table.append(text::to_string(size / 1024) + "K");

                test_dot(size, row);
        }

        table.print(std::cout);

	return EXIT_SUCCESS;
}

