#define BOOST_TEST_MODULE test_physical_layer
#include <boost/test/included/unit_test.hpp>

#include "../include.hpp"
#include "../node.hpp"
#include "../edge.hpp"


typedef float rate_t;
typedef std::string _name_t;
typedef size_t _id_t;

typedef Node<rate_t, _name_t, _id_t> TNode;
template<>
_id_t TNode::counter_ = 0;

BOOST_AUTO_TEST_SUITE( test_node )

BOOST_AUTO_TEST_CASE( test_constructors )
{
    TNode a, b(.1, .2, "bolt"), c(b), d(.3, .4);

    BOOST_CHECK_EQUAL( b.get_name(), "bolt" );
    BOOST_CHECK_EQUAL( c.get_name(), "bolt" );
    BOOST_CHECK_EQUAL( b.get_number(), 1 );
    BOOST_CHECK_EQUAL( c.get_number(), 1 );
    BOOST_CHECK( (b.get_arrival_rate() - 0.1) < std::numeric_limits<rate_t>::epsilon() and
                 (c.get_arrival_rate() - 0.1) < std::numeric_limits<rate_t>::epsilon() );
    BOOST_CHECK( (b.get_service_rate() - 0.2) < std::numeric_limits<rate_t>::epsilon() and
                 (c.get_service_rate() - 0.2) < std::numeric_limits<rate_t>::epsilon() );
    BOOST_CHECK_EQUAL( d.get_number(), 2 );
    BOOST_CHECK_EXCEPTION( Node<>(-1, 0), Wrong_parameter, [](Wrong_parameter const&){return true;} );
}

BOOST_AUTO_TEST_CASE( test_swap )
{
    TNode a(.1, .2);
    a.swap_arr_rate(.9);
    BOOST_CHECK( (a.get_arrival_rate() - .9) < std::numeric_limits<rate_t>::epsilon() );
}
BOOST_AUTO_TEST_SUITE_END()


typedef size_t flow_key_t;
typedef float weight_t;
typedef Edge<flow_key_t, weight_t> TEdge;

BOOST_AUTO_TEST_SUITE( test_edge )

BOOST_AUTO_TEST_CASE( test_constructors )
{
    TEdge a, b(2, 3), c(b);
    BOOST_CHECK_EQUAL( b.get_weight(), 3 );
    BOOST_CHECK_EQUAL( c.get_weight(), 3 );
}

BOOST_AUTO_TEST_CASE( test_allocate_flow )
{
    TEdge b(2, 3), c(b);
    std::list<flow_key_t> l;
    BOOST_CHECK( b.get_flows() == l );
    BOOST_CHECK_EQUAL( b.allocate_flow(1), 1 );
    l.push_back(1);
    BOOST_CHECK( b.get_flows() == l );
    BOOST_CHECK_EQUAL( b.allocate_flow(2), 0 );
    l.push_back(2);
    BOOST_CHECK( b.get_flows() == l );

    BOOST_CHECK_EQUAL( c.allocate_flow(3), 1 );
    BOOST_CHECK_EQUAL( c.get_weight(), 3 );
    BOOST_CHECK_EQUAL( c.allocate_flow(4), 0 );
    BOOST_CHECK_EQUAL( c.get_weight(), infinity_wrapper_std<weight_t>()());

    BOOST_CHECK_EXCEPTION( b.allocate_flow(2), Edge_allocation_error, [](Edge_allocation_error const&){return true;} );

    b.free_flow(1);
    l.remove(1);
    BOOST_CHECK( b.get_flows() == l );
    b.free_flow(2);
    l.remove(2);
    BOOST_CHECK( b.get_flows() == l );

    c.free_flow(3);
    BOOST_CHECK_EQUAL( c.get_weight(), 3 );
    c.free_flow(4);
    BOOST_CHECK_EQUAL( c.get_weight(), 3 );
}

BOOST_AUTO_TEST_SUITE_END()
