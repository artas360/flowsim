#define BOOST_TEST_MODULE test_event
#include <boost/test/included/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <limits>
#include <cmath>

#include "include.hpp"
#include "event.hpp"

BOOST_AUTO_TEST_SUITE( test_random_generator )

BOOST_AUTO_TEST_CASE( test_constructors )
{
    Random_generator<float> rand;
    float mean = 0;
    float mean2 = 0;
    size_t samples = 1000;
    for(size_t i = 0; i < samples; ++i) {
        mean += rand.next_arrival(.1);
        mean2 += rand.rand_duration(.5);
        rand.rand_int();
    }
    // TODO switch to boost is_close algo
    BOOST_CHECK( (samples / mean) - .1 < .01 );
    BOOST_CHECK( (samples / mean) - .5 < .01 );

    Random_generator<float> rand1(1), rand2(1);
    for(size_t i = 0; i < 5; ++i) {
        BOOST_CHECK( abs(rand1.next_arrival(.1) - rand2.next_arrival(.1)) <= std::numeric_limits<float>::epsilon() );
        BOOST_CHECK( abs(rand1.rand_duration(.5) - rand2.rand_duration(.5)) <= std::numeric_limits<float>::epsilon() );
        BOOST_CHECK_EQUAL( rand1.rand_int(), rand2.rand_int() );
    }
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( test_event )

BOOST_AUTO_TEST_CASE( test_constructors )
{
}

BOOST_AUTO_TEST_SUITE_END()
