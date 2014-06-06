#ifndef __TOPOLOGY_HPP__
#define __TOPOLOGY_HPP__

#include <vector>
#include <unordered_map>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/lexical_cast.hpp>

#include "include.hpp"


/**
 * \struct custom_reference
 * \brief Default constructible std::reference_wrapper.
 * \note Shouldn't be used, but no better solution so far.
 */
template<class T>
struct custom_reference {
    typedef T type;
    custom_reference() : pointer_(nullptr), is_valid_(false) {}

    custom_reference(T & target) : pointer_(&target), is_valid_(true) {}

    custom_reference(T && target) = delete;

    custom_reference(custom_reference const& other) : pointer_(other.pointer_),
                                                      is_valid_(other.is_valid_) {}

    T const& operator()() const {
#if not NDEBUG
        if(not is_valid_)
            throw std::runtime_error("Call to non init custom_reference");
#endif
        return *pointer_;
    }

    operator const T() const {
        return (*this)();
    }

    private:
        const T* pointer_;
        bool is_valid_;
};


namespace boost{
    enum edge_obj1_t { edge_obj1 };
    BOOST_INSTALL_PROPERTY(edge, obj1);
    enum vertex_obj2_t { vertex_obj2 };
    BOOST_INSTALL_PROPERTY(vertex, obj2);
}
/**
 * \class Topology
 * \brief Wrapper around a boost graph
 */
template <class Node, class Edge>
class Topology {
    typedef custom_reference<const typename Edge::weight_t> weight_ref_t;
    typedef typename boost::property<boost::edge_obj1_t, Edge, boost::property<boost::edge_weight_t, weight_ref_t>> Edge_obj;

    typedef typename boost::property<boost::vertex_obj2_t, Node> Vertex_obj;
    typedef typename boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Vertex_obj, Edge_obj> Graph;

    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;

    typedef std::unordered_map<typename Node::id_t, vertex_descriptor> id_to_key_map_t;

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
        typedef typename node_t::id_t id_t;

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

            typename id_to_key_map_t::const_iterator node_map_pos, node_map_end = id_to_key_.cend();

            vertex_descriptor src_node, dst_node;
            for(input_iterator it = first; it != last; ++it) {
                if ((node_map_pos = id_to_key_.find(get<0>(*it))) != node_map_end) {
                    src_node = node_map_pos->second;
                } else {
                    id_to_key_[get<0>(*it)] = src_node = add_node(Node(arrival_rate, service_rate, get<0>(*it)));
                }
                if ((node_map_pos = id_to_key_.find(get<1>(*it))) != node_map_end) {
                    dst_node = node_map_pos->second;
                } else {
                    id_to_key_[get<1>(*it)] = dst_node = add_node(Node(arrival_rate, service_rate, get<1>(*it)));
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

        void shortest_path(node_key_t const& src, node_key_t const& dst, path_t &shortest_path) {
            size_t num_nodes = boost::num_vertices(g_);
            static std::vector<vertex_descriptor> p(num_nodes);
            static std::vector<typename Edge::weight_t> d(num_nodes);
            p.reserve(num_nodes);
            d.reserve(num_nodes);
            shortest_path.clear();

            boost::dijkstra_shortest_paths(g_, src,
                    predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g_))).
                    distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g_))).
                    distance_inf(infinity_)
                    );

            if(d[dst] == infinity_ or p[dst] == dst)
                return;

            vertex_descriptor ed_d = dst, ed_s = p[ed_d];
            while(ed_d != src) {
                shortest_path.push_back(boost::edge(ed_s, ed_d, g_).first);
                ed_d = ed_s;
                ed_s = p[ed_s];
            }
            return;
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

        edge_key_t edge(node_key_t const& src, node_key_t const& dst) const {
            // TODO error handling
            assert(boost::edge(src, dst, g_).second);
            return boost::edge(src, dst, g_).first;
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

        std::pair<edge_iterator, edge_iterator> edges() {
            return boost::edges(g_);
        }

        const edge_map_t get_edge_map() {
            return get(boost::edge_obj1, g_);
        }

        const node_map_t get_node_map() {
            return get(boost::vertex_obj2, g_);
        }

    private:
        Graph g_;
        id_to_key_map_t id_to_key_;
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

#endif
