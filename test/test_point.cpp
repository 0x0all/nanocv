#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test_point"

#include <boost/test/unit_test.hpp>
#include "nanocv/vision/point.h"

namespace test
{
        void build_point(ncv::coord_t x, ncv::coord_t y)
        {
                const ncv::point_t point(x, y);

                BOOST_CHECK_EQUAL(point.x(), x);
                BOOST_CHECK_EQUAL(point.y(), y);
        }
}

BOOST_AUTO_TEST_CASE(test_point)
{
        test::build_point(3, 7);
        test::build_point(7, 3);
        test::build_point(-5, -1);
        test::build_point(-9, +1);
}
