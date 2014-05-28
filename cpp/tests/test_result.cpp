#define BOOST_TEST_MODULE test_result
#include <boost/test/included/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <cmath>

#include "result.hpp"

typedef float result_value_t;
typedef std::string result_key_t;

typedef sample_container<result_value_t> TSample_container;

BOOST_AUTO_TEST_SUITE( test_sample_container )

BOOST_AUTO_TEST_CASE( test_constructors )
{
    size_t number_samples(3);
    TSample_container c(number_samples, .01);

    for(size_t i(0); i < number_samples - 1; ++i) {
        c.update_samples(0);
        BOOST_CHECK( not std::isfinite(c.standard_deviation()) );
    }
    c.update_samples(0);
    BOOST_CHECK( std::isfinite(c.standard_deviation()) );
}

BOOST_AUTO_TEST_CASE( test_standard_deviation_convergence )
{
    size_t number_samples(3);
    result_value_t interval(10);
    TSample_container c(number_samples, .01);
    for(size_t i(0); i < number_samples; ++i)
        c.update_samples(i * interval);
    BOOST_CHECK_CLOSE( c.standard_deviation(), 10, 1e-3 );
    BOOST_CHECK( not c.has_converged() );
    for(size_t i(0); i < number_samples - 1; ++i) {
        c.update_samples( i * .01 );
        BOOST_CHECK( not c.has_converged() );
    }
    c.update_samples( 3 * .01 );
    BOOST_CHECK( c.has_converged() );
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( test_result )

float mean(std::valarray<float>& values) {
    return values.sum() / values.size();
}

BOOST_AUTO_TEST_CASE( test_constructors )
{
}

BOOST_AUTO_TEST_SUITE_END()
