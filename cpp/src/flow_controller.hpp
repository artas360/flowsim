#ifndef __FLOW_CONTROLLER_HPP__
#define __FLOW_CONTROLLER_HPP__

#include <unordered_map>
#include <tuple>

#include "include.hpp"
#include "flow.hpp"

template <typename key_t=uint32_t>
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
            return key != not_key;
        }

        static const key_t no_key() {
            return not_key;
        }

    private:
        key_t counter_;
        static const key_t not_key = 0;
};

template <class Event_manager, class Topology, class flow_t, class Key, class Flow_container = std::unordered_map<Key, flow_t>, class Key_generator=Key_generator<Key>>
class Flow_controller {
    typedef typename Topology::node_key_t node_key_t;
    typedef typename Topology::edge_key_t edge_key_t;
    typedef Key key_t;
    public:
        Flow_controller(Topology topology) : topology_(topology),
                                             flows_(),
                                             key_gen_() {
        }

        virtual key_t const& allocate_flow(node_key_t const& src, node_key_t const& dst) {
            modified_edges_.clear();

            // Adding flow to container
            key_t flow_key(key_gen_());
            assert(Key_generator::is_valid_key(flow_key));
            std::pair<typename Flow_container::iterator, bool> pair;
            try {
                pair = flows_.emplace(flow_key, topology_.shortest_path(src, dst));
                assert(pair.second());
            } catch (No_path_error e) {
                throw No_path_error();
            }

            // Trying to reserve Edge ressources along the path
            typename flow_t::container_t::const_iterator it(*(pair.first()).get_nodes().cbegin());
            typename flow_t::container_t::value_type former_node(*it);

            edge_key_t edge_key;
            try {
                for(++it; it != *(pair.first()).get_nodes().cend(); ++it) {
                    edge_key = topology_.get_edge_key(former_node, *it);
                    if (topology_.get_edge_object(edge_key).allocate_flow(flow_key) == 0) {
                        topology_.set_edge_unavailable(edge_key);
                    }
                    modified_edges_.push_back(edge_key);
                }
                return flow_key;
            } catch (Ressource_allocation_error e) {
                // Reverting changes if something wrong happened
                // Should not happen since shortest_path gives allocable edges
                topology_.set_edge_unavailable(edge_key);
                for(auto edge_key: modified_edges_) {
                    topology_.get_edge_object(edge_key).free_flow(flow_key);
                }
                flows_.erase(pair.first());
                return Key_generator::no_key();
            }
        }

        virtual void free_flow(key_t flow_key) {
            typename Flow_container::iterator iter(flows_.find(flow_key));
            if (iter == flows_.end()) {
                throw Not_registered_flow();
            }

            typename flow_t::container_t::const_iterator it(*(iter.second()).get_nodes().cbegin());
            typename flow_t::container_t::value_type former_node(*it);

            // freeing edges in flow
            for(++it; it != *(iter.second()).get_nodes().cend(); ++it) {
                edge_key_t const& edge_key(topology_.get_edge_key(former_node, *it));
                    topology_.free_edge(edge_key, flow_key);
            }
        }

        virtual void entry_nodes_iter() const {
            // Use std::pair
        }
        
    private:
        Topology topology_;
        Flow_container flows_;
        Key_generator key_gen_;
        std::vector<edge_key_t const&> modified_edges_;
};

#endif

#if TEST

#include "event.hpp"

class FooTopology{
};

int main() {
    FooTopology topo;
    Flow_controller<Event_manager<>, FooTopology, Flow<>, uint32_t> fc();
    return EXIT_SUCCESS;
}

#endif
