#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test_corr2d"

#include <boost/test/unit_test.hpp>
#include "nanocv/tensor.h"
#include "nanocv/logger.h"
#include "nanocv/math/close.hpp"
#include "nanocv/math/corr2d.hpp"
#include "nanocv/math/epsilon.hpp"

namespace test
{
        using namespace ncv;

        template
        <
                typename top,
                typename tmatrix,
                typename tscalar = typename tmatrix::Scalar
        >
        tscalar test_cpu(top op, const tmatrix& idata, const tmatrix& kdata, tmatrix& odata)
        {
                odata.setZero();

                op(idata, kdata, odata);

                return odata.sum();
        }

        void test_corr2d(int isize, int ksize)
        {
                const int osize = isize - ksize + 1;

                matrix_t idata(isize, isize);
                matrix_t kdata(ksize, ksize);
                matrix_t odata(osize, osize);

                idata.setRandom();
                kdata.setRandom();
                odata.setRandom();

                idata /= isize;
                kdata /= ksize;
                odata /= osize;

                const scalar_t corrcpu_egb = test_cpu(ncv::math::corr2d_egb<matrix_t>, odata, kdata, idata);
                const scalar_t corrcpu_egr = test_cpu(ncv::math::corr2d_egr<matrix_t>, odata, kdata, idata);
                const scalar_t corrcpu_cpp = test_cpu(ncv::math::corr2d_cpp<matrix_t>, odata, kdata, idata);
                const scalar_t corrcpu_mdk = test_cpu(ncv::math::corr2d_mdk<matrix_t>, odata, kdata, idata);
                const scalar_t corrcpu_mdo = test_cpu(ncv::math::corr2d_mdo<matrix_t>, odata, kdata, idata);
                const scalar_t corrcpu_dyn = test_cpu(ncv::math::corr2d_dyn<matrix_t>, odata, kdata, idata);

                const scalar_t epsilon = math::epsilon1<scalar_t>();

                BOOST_CHECK_LE(math::abs(corrcpu_egb - corrcpu_egb), epsilon);
                BOOST_CHECK_LE(math::abs(corrcpu_egr - corrcpu_egb), epsilon);
                BOOST_CHECK_LE(math::abs(corrcpu_cpp - corrcpu_egb), epsilon);
                BOOST_CHECK_LE(math::abs(corrcpu_mdk - corrcpu_egb), epsilon);
                BOOST_CHECK_LE(math::abs(corrcpu_mdo - corrcpu_egb), epsilon);
                BOOST_CHECK_LE(math::abs(corrcpu_dyn - corrcpu_egb), epsilon);
        }
}

BOOST_AUTO_TEST_CASE(test_corr2d)
{
        using namespace ncv;

        const int min_isize = 24;
        const int max_isize = 48;
        const int min_ksize = 5;
        const int n_tests = 64;

        for (int isize = min_isize; isize <= max_isize; isize += 4)
        {
                for (int ksize = min_ksize; ksize <= isize - min_ksize; ksize += 2)
                {
                        for (int t = 0; t < n_tests; t ++)
                        {
                                test::test_corr2d(isize, ksize);
                        }
                }
        }
}

