#ifndef __FLOW_CONTROLLER_HPP__
#define __FLOW_CONTROLLER_HPP__

#include <unordered_map>
#include <vector>
#include <tuple>

#include "include.hpp"

template <typename key_t=size_t>
class Key_generator {
    public:
        Key_generator(key_t seed = key_t(1)) : counter_(seed) {}

        key_t next() {
            return ++counter_;
        }

        key_t operator()() {
            return next();
        }

        static key_t is_valid_key(key_t key) {
            return key != no_key;
        }

        static const key_t not_key() {
            return no_key;
        }

    private:
        key_t counter_;
        static const key_t no_key = 0;
};

template <class Topology, class flow_t, class key_t=size_t, class Flow_container = std::unordered_map<key_t, flow_t>, class Key_generator=Key_generator<key_t>>
class Flow_controller {
    public:
        typedef key_t flow_key_t;
        typedef Topology topology_t;
        typedef Key_generator key_generator_t;
        typedef typename Topology::node_key_t node_key_t;
        typedef typename Topology::edge_key_t edge_key_t;
        typedef typename Flow_container::const_iterator const_iterator;
        Flow_controller(Topology &topology) : topology_(topology),
                                             flows_(),
                                             key_gen_() {
        }

        virtual key_t allocate_flow(node_key_t const& src, node_key_t const& dst) {
            // Adding flow to container
            key_t flow_key(key_gen_());
            assert(Key_generator::is_valid_key(flow_key));
            std::pair<typename Flow_container::iterator, bool> pair;

            typename Topology::path_t shortest_path = topology_.shortest_path(src, dst);
            if(not shortest_path.empty()) {
                pair = flows_.emplace(flow_key, shortest_path);
                assert(pair.second);
            }
            else {
                return key_generator_t::not_key();
            }

            auto a = ((*(pair.first)).second.get_edges());
            assert(not a.empty());
            // Trying to reserve Edge ressources along the path
            typename flow_t::const_iterator it((*(pair.first)).second.get_edges().cbegin()),
                                            end((*(pair.first)).second.get_edges().cend());

            try {
                for(; it != end; ++it) {
                    // Delegated to Edges
                    // if (topology_.get_edge_object(edge_key).allocate_flow(flow_key) == 0)
                    //     topology_.set_edge_unavailable(edge_key);
                    topology_.get_edge_object(*it).allocate_flow(flow_key);
                }
                return flow_key;
            } catch (Ressource_allocation_error e) {
                // Reverting changes if something wrong happened
                // Should not happen since shortest_path returns allocable edges
                typename flow_t::const_iterator it2((*(pair.first)).second.get_edges().cbegin());
                for(; it2 != it; ++it2) {
                    topology_.get_edge_object(*it2).free_flow(flow_key);
                }
                flows_.erase(pair.first);
                return Key_generator::not_key();
            }
        }

        virtual void free_flow(key_t flow_key) {
            assert(Key_generator::is_valid_key(flow_key));
            typename Flow_container::iterator iter(flows_.find(flow_key));
            if (iter == flows_.end()) {
                throw Not_registered_flow();
            }

            typename flow_t::const_iterator it((*iter).second.get_edges().cbegin()),
                                            end((*iter).second.get_edges().cend());

            // freeing edges in flow
            for(; it != end; ++it) {
                topology_.get_edge_object(*it).free_flow(flow_key);
            }
        }

        virtual flow_t const& get_flow(key_t flow_key) const {
            assert(Key_generator::is_valid_key(flow_key));
            // TODO check if exception thrown
            return flows_.at(flow_key);
        }

        // TODO
        // iterator<node_key_t> cbegin() const? , REF?
        std::pair<typename topology_t::node_iterator, typename topology_t::node_iterator>
        get_entry_nodes() const {
            return topology_.nodes();
        }

        topology_t const& get_topology() const {
            return topology_;
        }

        topology_t & get_topology() {
            return topology_;
        }

    private:
        Topology &topology_;
        Flow_container flows_;
        Key_generator key_gen_;
};

#endif

#if TEST_FLOW_CONTROLLER

class FooEdge {
    public:
    void free_flow(size_t) {}
    void allocate_flow(size_t) {}
};

class FooTopology{
    public:
        typedef size_t node_key_t;
        typedef size_t edge_key_t;
        typedef std::vector<edge_key_t> path_t;

        path_t shortest_path(node_key_t const & src, node_key_t const& dst) {
            path_t path = {src, dst};
            return path;
        }
        FooEdge & get_edge_object(edge_key_t const&){
            return f;
        }
        FooEdge f;
};

class FooEvent_manager {
};

template<class edge_key_t>
class FooFlow {
    public:
        typedef std::vector<edge_key_t> container_t;
        FooFlow(container_t) {}
        container_t get_edges() {
            return container_t();
        }
};

int main() {
    FooTopology topo;
    Flow_controller<FooEvent_manager, FooTopology, FooFlow<size_t>, uint32_t> fc(topo);
    return EXIT_SUCCESS;
}

#endif
