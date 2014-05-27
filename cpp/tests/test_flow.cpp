#define BOOST_TEST_MODULE test_flows
#include <boost/test/included/unit_test.hpp>

#include "include.hpp"
#include "key_generator.hpp"
#include "flow.hpp"
#include "flow_controller.hpp"

typedef size_t flow_key_t;
typedef Key_generator<flow_key_t> TKey_generator;

BOOST_AUTO_TEST_SUITE( test_key_generator )

BOOST_AUTO_TEST_CASE( test_constructors )
{
    flow_key_t seed(10);
    TKey_generator kg;
    TKey_generator kgb(seed);
    BOOST_CHECK_NE( kg(), TKey_generator::not_key() );
    BOOST_CHECK_NE( kg(), TKey_generator::not_key() );
    BOOST_CHECK( TKey_generator::is_valid_key(kg()) );
    BOOST_CHECK( not TKey_generator::is_valid_key(TKey_generator::not_key()) );
    BOOST_CHECK( kgb() >= seed );
    BOOST_CHECK_NE( kgb(), TKey_generator::not_key() );
    BOOST_CHECK_NE( kgb(), TKey_generator::not_key() );
    BOOST_CHECK( TKey_generator::is_valid_key(kgb()) );
}

BOOST_AUTO_TEST_SUITE_END()

typedef size_t edge_key_t;
typedef Flow<edge_key_t> TFlow;

BOOST_AUTO_TEST_SUITE( test_flow )

BOOST_AUTO_TEST_CASE( test_constructors )
{
    edge_key_t keys[] = {1, 2, 5, 8};
    std::vector<edge_key_t> vect(keys, keys + sizeof(keys) / sizeof(edge_key_t));

    Flow<edge_key_t> flow(vect);
    BOOST_CHECK_EQUAL( flow.get_edges()[2], keys[2] );
    BOOST_CHECK_EQUAL( flow.length(), sizeof(keys) / sizeof(edge_key_t) );
    
    std::vector<edge_key_t> tmp(keys, keys + sizeof(keys) / sizeof(edge_key_t));
    Flow<edge_key_t> flow2(std::move(tmp));
    BOOST_CHECK_EQUAL( flow2.get_edges()[2], keys[2] );
    BOOST_CHECK_EQUAL( flow2.length(), sizeof(keys) / sizeof(edge_key_t) );

    vect[2] = 0;
    tmp[2] = 0;

    BOOST_CHECK_EQUAL( flow.get_edges()[2], 5 );
    BOOST_CHECK_EQUAL( flow2.get_edges()[2], 5 );
}

BOOST_AUTO_TEST_SUITE_END()


class FooEdge{
    public:
    FooEdge() : allocated(false) {}
    bool allocated;
    void free_flow(size_t) {allocated = false;}
    void allocate_flow(size_t) {allocated = true;}
};

class FooTopology{
    public:
        typedef size_t node_key_t;
        typedef ::edge_key_t edge_key_t;
        typedef std::vector<edge_key_t> path_t;
        // Dirty hack to provide an iterator type
        typedef path_t::iterator node_iterator;

        void shortest_path(node_key_t const & src, node_key_t const& dst, path_t & path) {
            if(src == dst)
                return;
            path.clear();
            path.push_back(src);
            path.push_back(dst);
        }
        FooEdge & get_edge_object(edge_key_t const& k){
            if(k == 0)
                return f0;
            else
                return f1;
        }
        FooEdge f0;
        FooEdge f1;
};

struct FooEvent_manager {
    typedef typename FooTopology::edge_key_t edge_key_t;
    typedef typename FooTopology::node_key_t node_key_t;
};


typedef Flow_controller<FooTopology, TFlow, flow_key_t> TFlow_controller;

BOOST_AUTO_TEST_SUITE( test_flow_controller )

BOOST_AUTO_TEST_CASE( test_constructors )
{
    FooTopology topo;
    TFlow_controller fc(topo);
    BOOST_CHECK_EQUAL( &topo, &(fc.get_topology()) );
}

BOOST_AUTO_TEST_CASE( test_allocate_free_flows )
{
    FooTopology topo;
    TFlow_controller fc(topo);

    flow_key_t k = fc.allocate_flow(0, 0);
    BOOST_CHECK_EQUAL( k, TKey_generator::not_key() );
    BOOST_CHECK( not topo.get_edge_object(0).allocated );
    BOOST_CHECK( not topo.get_edge_object(1).allocated );

    k = fc.allocate_flow(0, 1);
    BOOST_CHECK_NE( k, TKey_generator::not_key() );
    BOOST_CHECK( topo.get_edge_object(0).allocated );
    BOOST_CHECK( topo.get_edge_object(1).allocated );
    typename FooTopology::path_t path = {0, 1};
    BOOST_CHECK( fc.get_flow(k).get_edges() == path );

    BOOST_CHECK_EXCEPTION( fc.free_flow(-1), Not_registered_flow, [](Not_registered_flow const&){return true;} );
    fc.free_flow(k);
    BOOST_CHECK( not topo.get_edge_object(0).allocated );
    BOOST_CHECK( not topo.get_edge_object(1).allocated );
}

BOOST_AUTO_TEST_SUITE_END()
