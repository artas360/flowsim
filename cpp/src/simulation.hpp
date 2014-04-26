#ifndef __SIMULATION_HPP__
#define __SIMULATION_HPP__

#include <vector>

#include "include.hpp"

#include "node.hpp"
#include "edge.hpp"
#include "topology.hpp"

#include "event.hpp"

#include "flow.hpp"
#include "flow_controller.hpp"

template<class Topology, class Flow_manager, class Event_manager>
class Simulation {
    public:
        typedef typename Topology::rate_t rate_t;
        template <class iterator>
        Simulation(iterator first,
                   iterator last,
                   rate_t arrival_rate,
                   rate_t service_rate) : topology_(first,
                                                    last,
                                                    arrival_rate,
                                                    service_rate),
                                          flow_controller_(topology_),
                                          event_manager_(flow_controller_) {
        }

        typename Event_manager::result_t const& launch_simulation() {
            event_manager_.start_event_processing();
            return event_manager_.get_result();
        }

    private:
        Topology topology_;
        Flow_manager flow_controller_;
        Event_manager event_manager_;
};

#ifdef SIMULATION

template<>
size_t Node<float, size_t, size_t>::counter_ = 0;

int main(int, char*[]) {
    // Topology types
    typedef size_t node_id_t;
    typedef size_t name_t; // use of import_description_from_int -> name is an int
    typedef float rate_t;
    typedef Node<rate_t, name_t, node_id_t> node_t;

    typedef size_t flow_key_t;
    typedef float weigth_t;
    typedef Edge<flow_key_t, weigth_t> edge_t;
    typedef Topology<node_t, edge_t> topology_t;

    // Flow types
    typedef Flow<typename topology_t::edge_key_t> flow_t;
    typedef Flow_controller<topology_t, flow_t> flow_controller_t;

    // Event types
    typedef float time;
    typedef Result<name_t, float> result_t;
    typedef Event_manager<flow_controller_t, time, result_t> event_manager_t;

    // Simulation
    typedef Simulation<topology_t, flow_controller_t, event_manager_t> simulation_t;


    // Run
    std::vector<std::tuple<id_t, id_t, edge_t>> description = {std::make_tuple(0, 1, edge_t(1, 1)),
                                                             std::make_tuple(0, 2, edge_t(1, 1)),
                                                             std::make_tuple(1, 2, edge_t(1, 1)),
                                                             std::make_tuple(1, 3, edge_t(1, 1)),
                                                             std::make_tuple(2, 3, edge_t(1, 1))};

    simulation_t simulation(description.cbegin(),
                            description.cend(),
                            .1,
                            .9);

    result_t results = simulation.launch_simulation();

    std::cout << results << std::endl;

    return EXIT_SUCCESS;
}

#endif

#endif
