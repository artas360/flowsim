#ifndef __TOPOLOGYHPP__
#define __TOPOLOGYHPP__

#include <string>
#include <vector>
#include <iterator>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

template <class Node, class Edge>
class Topology {
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Node, Edge> Graph;
    private:
        Graph g;

    public:

    Topology();

    void add_node(Node&);
    void add_nodes(); //Node iter
    void add_edge(Edge&);
    void add_edges(); //Edge iter

    void import_topology_from_int(std::) {
    }


    void set_edge_unavailable(Node const& node1, Node const& node2);
    void free_edge(Node const& node1, Node const& node2);
    void get_edge_object(Node const& node1, Node const& node2) const;
    void shortest_path(Node const& node1, Node const& node2) const;
    void import_topology(std::string const& filename, const float arrival_rate, const float service_rate);
    Node const& get_random_entry_node(int number);
    Node const& get_random_exit_node(int number);
    Node const& get_entry_nodes(int number); //node iter
};

// Torus 2D
// Drawgraph

#endif
