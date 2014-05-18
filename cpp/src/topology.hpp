#ifndef __TOPOLOGY_HPP__
#define __TOPOLOGY_HPP__

#include <vector>
#include <exception>
#include <sstream>
#include <unordered_map>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/lexical_cast.hpp>

#include "include.hpp"

template<class T>
struct custom_reference {
    typedef T type;
    custom_reference() : pointer_(nullptr), is_valid_(false) {}
    custom_reference(T & target) : pointer_(&target), is_valid_(true) {}
    custom_reference(T && target) = delete;
    custom_reference(custom_reference<T> const& other) : pointer_(other.pointer_), is_valid_(other.is_valid_) {}
    T& operator()() {
        if(is_valid_)
            return *pointer_;
        throw std::runtime_error("Call to non init custom_reference");
    }
    T const& operator()() const {
        if(is_valid_)
            return *pointer_;
        throw std::runtime_error("Call to non init custom_reference");
    }
    operator T() {
        return (*this)();
    }
    operator const T() const {
        return (*this)();
    }
    private:
        T* pointer_;
        bool is_valid_;
};

namespace boost{
    enum edge_obj1_t { edge_obj1 };
    BOOST_INSTALL_PROPERTY(edge, obj1);
    enum vertex_obj2_t { vertex_obj2 };
    BOOST_INSTALL_PROPERTY(vertex, obj2);
}

template <class Node, class Edge>
class Topology {
    typedef custom_reference<const typename Edge::weight_t> weight_ref_t;
    typedef typename boost::property<boost::edge_obj1_t, Edge, boost::property<boost::edge_weight_t, weight_ref_t>> Edge_obj;

    typedef typename boost::property<boost::vertex_obj2_t, Node> Vertex_obj;
    typedef typename boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Vertex_obj, Edge_obj> Graph;

    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;

    public:
        typedef typename boost::graph_traits<Graph>::edge_iterator edge_iterator;
        typedef typename boost::graph_traits<Graph>::vertex_iterator node_iterator;
        typedef typename boost::property_map<Graph, boost::edge_obj1_t>::type edge_map_t;
        typedef typename boost::property_map<Graph, boost::vertex_obj2_t>::type node_map_t;
        typedef edge_descriptor edge_key_t;
        typedef vertex_descriptor node_key_t;
        typedef std::vector<edge_key_t> path_t;
        typedef typename Edge::infinity_wrapper_t infinity_wrapper_t;
        typedef typename Node::rate_t rate_t;
        typedef typename Node::name_t node_name_t;
        typedef Node node_t;
        typedef Edge edge_t;

    public:
        Topology() : g_() {
        }

        template<typename input_iterator>
        Topology(input_iterator first, input_iterator last, rate_t arrival_rate, rate_t service_rate, bool bidirect=true) : g_() {
            import_topology_from_description(first, last, arrival_rate, service_rate, bidirect);
        }

        edge_descriptor add_edge(vertex_descriptor const& src, vertex_descriptor const& dst, Edge const& edge) {
            // Can't use because not pointing at local copy of edge
            //typename Graph::edge_property_type ep(edge, weight_ref_t(edge.get_weight()));
            //auto res = boost::add_edge(src, dst, ep, g_);
            auto res = boost::add_edge(src, dst, edge, g_);
            if (not res.second)
                throw Edge_allocation_error();
            boost::put(boost::edge_weight, g_, res.first, weight_ref_t(get(boost::edge_obj1, g_, res.first).get_weight()));
            return res.first;
        }

        vertex_descriptor add_node(Node const& node){
            return boost::add_vertex(node, g_);
        }

        template <class Tuple>
        typename std::enable_if<std::is_same<typename std::tuple_size<Tuple>, typename std::tuple_size<std::tuple<typename std::tuple_element<0, Tuple>::type, typename std::tuple_element<1, Tuple>::type>>>::value, edge_descriptor>::type
        add_edge_from_tuple(vertex_descriptor src_node, vertex_descriptor dst_node, Tuple const&) {
            return add_edge(src_node, dst_node, Edge());
        }

        template <class Tuple>
        typename std::enable_if<std::is_same<typename std::tuple_size<Tuple>, typename std::tuple_size<std::pair<typename std::tuple_element<0, Tuple>::type, typename std::tuple_element<1, Tuple>::type>>>::value, edge_descriptor>::type
        add_edge_from_tuple(vertex_descriptor src_node, vertex_descriptor dst_node, Tuple const&) {
            return add_edge(src_node, dst_node, Edge());
        }

        template <class Tuple>
        typename std::enable_if<std::is_same<typename std::tuple_size<Tuple>, typename std::tuple_size<std::tuple<typename std::tuple_element<0, Tuple>::type, typename std::tuple_element<1, Tuple>::type, Edge>>>::value, edge_descriptor>::type
        add_edge_from_tuple(vertex_descriptor src_node, vertex_descriptor dst_node, Tuple const& tuple) {
            return add_edge(src_node, dst_node, std::get<2>(tuple));
        }

        template <class input_iterator>
        void import_topology_from_description(input_iterator first, input_iterator last, rate_t arrival_rate, rate_t service_rate, bool bidirect=true) {
            // Do not use! creates strange Nodes
            //g_ = Graph(node_count > 0 ? node_count : last - first);

            typedef typename std::tuple_element<0, typename input_iterator::value_type>::type base_type;
            typename std::unordered_map<base_type, vertex_descriptor> tmp_node_map;
            typename std::unordered_map<base_type, vertex_descriptor>::const_iterator node_map_pos, node_map_end = tmp_node_map.cend();

            vertex_descriptor src_node, dst_node;
            for(input_iterator it = first; it != last; ++it) {
                if ((node_map_pos = tmp_node_map.find(get<0>(*it))) != node_map_end) {
                    src_node = node_map_pos->second;
                } else {
                    tmp_node_map[get<0>(*it)] = src_node = add_node(Node(arrival_rate, service_rate, get<0>(*it)));
                }
                if ((node_map_pos = tmp_node_map.find(get<1>(*it))) != node_map_end) {
                    dst_node = node_map_pos->second;
                } else {
                    tmp_node_map[get<1>(*it)] = dst_node = add_node(Node(arrival_rate, service_rate, get<1>(*it)));
                }

                add_edge_from_tuple(src_node, dst_node, *it);
                if(bidirect)
                    add_edge_from_tuple(dst_node, src_node, *it);
            }
        }

        void import_topology_from_map(std::vector<std::unordered_map<std::string, std::string>> const& node_list,
                                      std::vector<std::unordered_map<std::string, std::string>> const& edge_list) {
            // Reading Nodes
            id_t node_id;
            node_name_t node_name;
            rate_t arr_rate, serv_rate;
            const std::string arrival_rate_s("arrival_rate"),
                              service_rate_s("service_rate"),
                              name_s("name"),
                              id_s("id");
            for(auto node_desc: node_list) {
                try {
                    node_id = boost::lexical_cast<id_t, std::string>(node_desc.at(id_s));
                    arr_rate = boost::lexical_cast<rate_t, std::string>(node_desc.at(arrival_rate_s));
                    serv_rate = boost::lexical_cast<rate_t, std::string>(node_desc.at(service_rate_s));

                    node_name = boost::lexical_cast<node_name_t, std::string>(node_desc[name_s]);  // Optional param
                    if(id_to_key_.find(node_id) != id_to_key_.end())
                        throw Duplicated_node_error();
                    id_to_key_[node_id] = add_node(node_t(arr_rate, serv_rate, node_name));
                } catch (boost::bad_lexical_cast const& bc) {
                    throw Configuration_error(bc.what());
                } catch (std::out_of_range const&) {
                    throw Configuration_error("Missing required field in node description. Check DTD.");
                }
            }

            // Reading Edges
            id_t src_id, dst_id;
            typename edge_t::weight_t weight;
            typename edge_t::capacity_t capacity;
            bool unidir;
            const std::string src_id_s("source_id"),
                              capacity_s("capacity"),
                              weight_s("weight"),
                              dst_id_s("destination_id"),
                              unidir_s("unidirectional"),
                              true_s("True");

            for(auto edge_desc: edge_list) {
                try {
                    src_id = boost::lexical_cast<id_t, std::string>(edge_desc.at(src_id_s));
                    dst_id = boost::lexical_cast<id_t, std::string>(edge_desc.at(dst_id_s));
                    unidir = edge_desc.at(unidir_s) != true_s ? false : true;
                    weight = boost::lexical_cast<typename edge_t::weight_t, std::string>(edge_desc.at(weight_s));
                    capacity = boost::lexical_cast<typename edge_t::capacity_t, std::string>(edge_desc.at(capacity_s));

                    add_edge(id_to_key_.at(src_id),
                             id_to_key_.at(dst_id),
                             edge_t(capacity, weight));

                    if (not unidir)
                        add_edge(id_to_key_.at(dst_id),
                                 id_to_key_.at(src_id),
                                 edge_t(capacity, weight));

                } catch (boost::bad_lexical_cast const& bc) {
                    throw Configuration_error(bc.what());
                } catch (std::out_of_range const&) {
                    throw Configuration_error("Missing required field in edge description. Check DTD.");
                }
            }

        }

        path_t shortest_path(node_key_t const& src, node_key_t const& dst) {
            size_t num_nodes = boost::num_vertices(g_);
            static std::vector<vertex_descriptor> p(num_nodes);
            static std::vector<typename Edge::weight_t> d(num_nodes);
            p.reserve(num_nodes);
            d.reserve(num_nodes);
            boost::dijkstra_shortest_paths(g_, src,
                    predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g_))).
                    distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g_))).
                    distance_inf(infinity_)
                    );

            if(d[dst] == infinity_ or p[dst] == dst)
                return path_t();

            path_t shortest_path;
            vertex_descriptor ed_d = dst, ed_s = p[ed_d];
            while(ed_d != src) {
                shortest_path.push_back(edge(ed_s, ed_d, g_).first);
                ed_d = ed_s;
                ed_s = p[ed_s];
            }
            return shortest_path;
        }

        node_key_t id_to_key(typename Node::id_t const& id) const {
            // can throw
            return id_to_key_.at(id);
        }

        void swap_node_arr_rate(node_key_t const& target, rate_t const& new_arrival_rate) {
            get_node_object(target).swap_arr_rate(new_arrival_rate);
        }

        Edge& get_edge_object(edge_key_t const& edge_key) {
            return boost::get(boost::edge_obj1, g_, edge_key);
        }

        Edge const& get_edge_object(edge_key_t const& edge_key) const {
            return get_edge_object(edge_key);
        }

        Node& get_node_object(node_key_t const& node_key) {
            return boost::get(boost::vertex_obj2, g_, node_key);
        }

        Node const& get_node_object(node_key_t const& node_key) const {
            return boost::get(boost::vertex_obj2, g_, node_key);
        }

        void dump_graphviz(std::ostream& out) {
            boost::write_graphviz(out, g_);
        }

        // Delegated to Edge inner mecanism
        // void set_edge_unavailable(node_key_t const& src, node_key_t const& dst);
        // void free_edge(node_key_t const& node1, node_key_t const& node2);

        node_key_t get_random_entry_node(size_t number) const {
            return *(boost::vertices(g_).first + (number % num_vertices(g_)));
        }

        node_key_t get_random_exit_node(size_t number) const {
            return *(boost::vertices(g_).first + (number % num_vertices(g_)));
        }

        // TODO find a way to const cast
        std::pair<node_iterator, node_iterator> nodes() const {
            return boost::vertices(g_);
        }

#if TEST_TOPOLOGY
        std::pair<edge_iterator, edge_iterator> edges() {
            return boost::edges(g_);
        }

        edge_map_t get_edge_map() {
            return get(boost::edge_obj1, g_);
        }

        node_map_t get_node_map() {
            return get(boost::vertex_obj2, g_);
        }
#endif

    private:
        Graph g_;
        std::unordered_map<typename Node::id_t, vertex_descriptor> id_to_key_;
        const typename Edge::weight_t infinity_ = infinity_wrapper_t()();
};

template<class descriptor=size_t, class container_t=std::vector<std::pair<descriptor, descriptor>>>
container_t torus2D(size_t y, size_t x, size_t ) {
    // typedef typename container_t::value_type value_t;
    if (x <= 2 or y <= 2)
        throw Not_implemented_yet();
    container_t edges;
    for(size_t i = 0; i < x; ++i) {
        for(size_t j = 0; j < y; ++j) {
            edges.emplace_back(i * y + j, ((i + 1) % x) * y + j);
            edges.emplace_back(i * y + j, ((j + 1) % y) + i * y);
        }
    }
    return edges;
}

#if TEST_TOPOLOGY

#include <iostream>
#include <limits>
#include <sstream>
#include <fstream>
#include "node.hpp"
#include "edge.hpp"

//struct Node {
//    typedef int id_t;
//    typedef float rate_t;
//    typedef int name_t;
//    static int counter;
//    id_t a;
//    rate_t arr, serv;
//    name_t name_;
//    Node() {}
//    Node(rate_t arrival_rate, rate_t service_rate, name_t const& name = name_t()) : a(counter++), arr(arrival_rate), serv(service_rate), name_(name) {}
//    Node(Node const& other) : a(other.a), arr(other.arr), serv(other.serv) {}
//    id_t get_number() {return a;}
//};

typedef Node<float, size_t, size_t> node_t;
typedef Edge<size_t, float> edge_t;
template<>
size_t node_t::counter_ = 0;

//template <typename weight_t>
//struct infinity_wrapper {
//    weight_t operator()() {
//        return std::numeric_limits<weight_t>::infinity();
//    }
//};

//struct edge_t {
//    typedef float weight_t;
//    typedef struct infinity_wrapper<weight_t> infinity_wrapper;
//    int a;
//    weight_t b;
//    edge_t() : a(0), b(a) {}
//    edge_t(int a) : a(a), b(a) {}
//    weight_t const& get_weight() const {return b;}
//    void switch_weight() {b = std::numeric_limits<float>::infinity();}
//};

int test_topology() {
    typedef Topology<node_t, edge_t>::node_key_t node_key;
    typedef Topology<node_t, edge_t>::edge_key_t edge_key;
    node_t n1(.5, .6, 0), n2(.7, .8, 1), &n3(n2);
    edge_t e1(5, 2);
    Topology<node_t, edge_t> topo;
    node_key kn1 = topo.add_node(n1);
    node_key kn2 = topo.add_node(n2);
    node_key kn3 = topo.add_node(std::move(n3));
    edge_key ke1 = topo.add_edge(kn1, kn2, e1);
    edge_key ke2 = topo.add_edge(kn1, kn3, edge_t(10, 100));
    typename Topology<node_t, edge_t>::edge_map_t map = topo.get_edge_map();
    FTEST(map[ke1].get_weight() == 2);
    FTEST(map[ke2].get_weight() == 100);

    // Test import_topology_from_int
    Topology<node_t, edge_t> topo2;
    std::vector<std::tuple<int, int, edge_t>> description = {std::make_tuple(0, 1, edge_t(1, 3)),
                                                           std::make_tuple(0, 2, edge_t(1, 9)),
                                                           std::make_tuple(1, 2, edge_t(1, 2)),
                                                           std::make_tuple(1, 3, edge_t(1, 10)),
                                                           std::make_tuple(2, 3, edge_t(1, 1))};
    topo2.import_topology_from_description(description.cbegin(), description.cend(), .5, .6);
    auto edge_map2 = topo2.get_edge_map();
//    auto node_map2 = topo2.get_node_map();

    typedef Topology<node_t, edge_t>::edge_iterator edge_iter;
//    edge_iter ei, ei_end;
//    for (std::tie(ei, ei_end) = topo2.edges(); ei != ei_end; ++ei)
//        std::cout << "edge_t " << edge_map2[*ei].b << std::endl;
//
    typedef Topology<node_t, edge_t>::node_iterator node_iter;
    std::pair<node_iter, node_iter> vp;
//    for (vp = topo2.nodes(); vp.first != vp.second; ++vp.first)
//        std::cout << "node_t " << node_map2[*vp.first].a << std::endl;
//    std::cout << std::endl;

    vp = topo2.nodes();
    auto path = topo2.shortest_path(*vp.first, *(vp.first+3));
    std::stringstream sstr;
    for(auto it: path)
        sstr << it;
    FTEST(std::string("(2,3)(1,2)(0,1)").compare(sstr.str()) == 0);

    edge_map2[*topo2.edges().first].allocate_flow(0);

    vp = topo2.nodes();
    path = topo2.shortest_path(*vp.first, *(vp.first+3));
    std::stringstream sstr2;
    for(auto it: path)
        sstr2 << it;
    FTEST(std::string("(2,3)(0,2)").compare(sstr2.str()) == 0);

    FTEST(topo2.get_random_entry_node(999) < 4);

    std::stringstream sstr3;
    auto torus = torus2D(3, 4, true);
    for(auto it: torus2D(3, 4, true))
        sstr3 << "(" << std::get<0>(it) << "," << std::get<1>(it) << ")";
    FTEST((std::string("(0,3)(0,1)(1,4)(1,2)(2,5)(2,0)(3,6)(3,4)(4,7)(4,5)")+
           std::string("(5,8)(5,3)(6,9)(6,7)(7,10)(7,8)(8,11)(8,6)(9,0)")+
           std::string("(9,10)(10,1)(10,11)(11,2)(11,9)")).compare(sstr3.str()) == 0);

    Topology<node_t, edge_t> topo3(torus.begin(), torus.end(), .5, .6);
    std::ofstream file("test.graphviz", std::ios_base::out);
    topo3.dump_graphviz(file);
    file.close();

    return EXIT_SUCCESS;
}

int main() {
    return test_topology();
}

#endif

#endif
