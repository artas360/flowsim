#define BOOST_TEST_MODULE test_physical_layer
#include <boost/test/included/unit_test.hpp>

#include <sstream>

#include "include.hpp"
#include "node.hpp"
#include "edge.hpp"
#include "topology.hpp"


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


typedef Node<rate_t, _id_t, _id_t> TNode2;
template<>
_id_t TNode2::counter_ = 0;
typedef Topology<TNode2, TEdge> TTopology;
typedef typename TTopology::node_key_t node_key_t;
typedef typename TTopology::edge_key_t edge_key_t;

BOOST_AUTO_TEST_SUITE( test_topology )

BOOST_AUTO_TEST_CASE( test_add )
{
    TNode2 n1(.5, .6, 0), n2(.7, .8, 1), n3(.9, .8, 2);
    TEdge e1(5, 2);
    TTopology topo;
    node_key_t kn1 = topo.add_node(n1);
    node_key_t kn2 = topo.add_node(n2);
    node_key_t kn3 = topo.add_node(std::move(n3));
    edge_key_t ke1 = topo.add_edge(kn1, kn2, e1);
    edge_key_t ke2 = topo.add_edge(kn1, kn3, TEdge(10, 100));

    typename TTopology::node_map_t nmap = topo.get_node_map();
    BOOST_CHECK_EQUAL( nmap[kn1].get_name(), 0 );
    BOOST_CHECK_EQUAL( nmap[kn2].get_name(), 1 );
    BOOST_CHECK_EQUAL( nmap[kn3].get_name(), 2 );

    typename TTopology::edge_map_t emap = topo.get_edge_map();
    BOOST_CHECK_EQUAL( emap[ke1].get_weight(), 2 );
    BOOST_CHECK_EQUAL( emap[ke2].get_weight(), 100 );
}

BOOST_AUTO_TEST_CASE( test_import_topology_from_description )
{
    // TODO wrong input case
    TTopology topo2;
    std::vector<std::tuple<int, int, TEdge>> description = {std::make_tuple(0, 1, TEdge(1, 3)),
                                                            std::make_tuple(0, 2, TEdge(1, 9)),
                                                            std::make_tuple(1, 2, TEdge(1, 2)),
                                                            std::make_tuple(1, 3, TEdge(1, 10)),
                                                            std::make_tuple(2, 3, TEdge(1, 1))};
    topo2.import_topology_from_description(description.cbegin(), description.cend(), .5, .6);
    
    auto node_map = topo2.get_node_map();
    BOOST_CHECK_EQUAL( node_map[topo2.id_to_key(0)].get_name(), 0 );
    BOOST_CHECK_EQUAL( node_map[topo2.id_to_key(3)].get_name(), 3 );

    // TODO switch to boost is_close algo
    BOOST_CHECK( (node_map[topo2.id_to_key(3)].get_arrival_rate() - .5) <= std::numeric_limits<rate_t>::epsilon() );
    BOOST_CHECK( (node_map[topo2.id_to_key(3)].get_service_rate() - .6) <= std::numeric_limits<rate_t>::epsilon() );

    // TODO test edges
}

BOOST_AUTO_TEST_CASE( test_import_topology_from_map )
{
}

BOOST_AUTO_TEST_CASE( test_shortest_path )
{
    TTopology topo2;
    std::vector<std::tuple<int, int, TEdge>> description = {std::make_tuple(0, 1, TEdge(1, 3)),
                                                            std::make_tuple(0, 2, TEdge(1, 9)),
                                                            std::make_tuple(1, 2, TEdge(1, 2)),
                                                            std::make_tuple(1, 3, TEdge(1, 10)),
                                                            std::make_tuple(2, 3, TEdge(1, 1))};
    topo2.import_topology_from_description(description.cbegin(), description.cend(), .5, .6);

    typename TTopology::path_t path;
    topo2.shortest_path(topo2.id_to_key(0), topo2.id_to_key(3), path);
    BOOST_CHECK_EQUAL( path.size(), 3 );
    BOOST_CHECK_EQUAL( topo2.edge(topo2.id_to_key(2), topo2.id_to_key(3)), path[0] );
    BOOST_CHECK_EQUAL( topo2.edge(topo2.id_to_key(1), topo2.id_to_key(2)), path[1] );
    BOOST_CHECK_EQUAL( topo2.edge(topo2.id_to_key(0), topo2.id_to_key(1)), path[2] );

    topo2.get_edge_object(topo2.edge(topo2.id_to_key(1), topo2.id_to_key(2))).allocate_flow(0);

    topo2.shortest_path(topo2.id_to_key(0), topo2.id_to_key(3), path);
    BOOST_CHECK_EQUAL( path.size(), 2 );
    BOOST_CHECK_EQUAL( topo2.edge(topo2.id_to_key(2), topo2.id_to_key(3)), path[0] );
    BOOST_CHECK_EQUAL( topo2.edge(topo2.id_to_key(0), topo2.id_to_key(2)), path[1] );

    BOOST_CHECK( topo2.get_random_entry_node(999) < 4 );
}

BOOST_AUTO_TEST_CASE( test_torus )
{
    std::stringstream sstr;
    for(auto it: torus2D(3, 4, true))
        sstr << "(" << std::get<0>(it) << "," << std::get<1>(it) << ")";
    BOOST_CHECK_EQUAL((std::string("(0,3)(0,1)(1,4)(1,2)(2,5)(2,0)(3,6)(3,4)(4,7)(4,5)")+
                       std::string("(5,8)(5,3)(6,9)(6,7)(7,10)(7,8)(8,11)(8,6)(9,0)")+
                       std::string("(9,10)(10,1)(10,11)(11,2)(11,9)")).compare(sstr.str()), 0);
}

BOOST_AUTO_TEST_SUITE_END()
