#ifndef __TOPOLOGYHPP__
#define __TOPOLOGYHPP__

#include <string>
#include <vector>
#include <iterator>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace boost{
    enum edge_obj1_t { edge_obj1 };
    BOOST_INSTALL_PROPERTY(edge, obj1);
    enum vertex_obj2_t { vertex_obj2 };
    BOOST_INSTALL_PROPERTY(vertex, obj2);
}

template <class Node, class Edge>
class Topology {
    typedef typename boost::property<boost::edge_obj1_t, Edge> Edge_obj;
    typedef typename boost::property<boost::vertex_obj2_t, Node> Vertex_obj;
    typedef typename boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Vertex_obj, Edge_obj> Graph;
    typedef typename boost::graph_traits<Graph>::edge_iterator Edge_iterator;
    typedef typename boost::property_map<Graph, boost::edge_obj1_t> Edge_map;
    typedef typename boost::property_map<Graph, boost::vertex_obj2_t> Node_map;
    //typedef typename boost::property_map<Graph, boost::edge_weight_t>::type Weight_map;
    typedef std::tuple<int, int, Edge> Edge_tuple;
    typedef typename std::vector<Edge_tuple>::iterator Edge_tuple_it;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
    private:
        Graph g;
        LOOK EXAMPLE L43 !
        //Weight_map map_weight;

        static std::pair<edge_descriptor, float> const& get_weight(std::pair<edge_descriptor, Edge> const& pair) {
            return std::pair<edge_descriptor, float>(get<0>(pair), get<1>(pair).get_weight());
        }

    public:

        template <class InputIterator>
        Topology(int node_count, InputIterator first, InputIterator last){
            import_topology_from_int(node_count, first, last);
        }

        void add_node(Node const& node){
            boost::add_vertex(node, g);
        }

        void add_edge(int src, int dst, Edge const& edge = Edge()){
            vertex_descriptor u = boost::vertex(src, g), v = boost::vertex(dst, g);

            boost::add_edge(u, v, edge, g);
        }

        template <class InputIterator>
        typename std::enable_if<std::is_same<typename InputIterator::value_type, std::pair<int, int>>::value, void>::type import_topology_from_int(int node_count, InputIterator first, InputIterator last) {
            g = Graph(node_count);

            for(InputIterator it = first; it != last; ++it)
                boost::add_edge(get<0>(*it), get<1>(*it), g);
        }

        template <class InputIterator>
        typename std::enable_if<std::is_same<typename InputIterator::value_type, std::tuple<int, int, Edge>>::value, void>::type import_topology_from_int(int node_count, InputIterator first, InputIterator last){
            g = Graph(node_count);

            for(InputIterator it = first; it != last; ++it)
                boost::add_edge(get<0>(*it), get<1>(*it), get<2>(*it), g);
        }

        void set_edge_unavailable(Node const& node1, Node const& node2);
        void free_edge(Node const& node1, Node const& node2);
        void get_edge_object(Node const& node1, Node const& node2) const;

        void shortest_path(Node const& src, Node const& dst) const{ //Throws
            vertex_descriptor u = boost::vertex(src, g), v = boost::vertex(dst, g);

//boost::shortest_path(g, u, );
        }

        void import_topology(std::string const& filename, const float arrival_rate, const float service_rate);
        void update_after_modif() {
                typename boost::property_map<Graph, boost::edge_obj1_t>::type map_edge = get(boost::edge_obj1, g);
                std::transform(map_edge.begin(), map_edge.end(), map_weight.begin(), get_weight);
        }
        Node const& get_random_entry_node(int number);
        Node const& get_random_exit_node(int number);
        Node const& get_entry_nodes(int number); //node iter
};

// Torus 2D
// Drawgraph

struct foo{
    foo(){}
};
typedef struct foo foo;

int main()
{
        std::vector<std::pair<int, int>> vect = {std::pair<int, int>(0,1), std::pair<int,int>(1,0), std::pair<int,int>(0,2)};
        Topology<int, int> topo(2, vect.begin(), vect.end());
}



        std::vector<std::tuple<int, int, foo>> vect2 = {std::tuple<int, int, foo>(0,1, foo()), std::tuple<int,int, foo>(1,0, foo()), std::tuple<int,int, foo>(0,2,foo())};
        Topology<int, foo> topo2(2, vect2.begin(), vect2.end());

#endif
