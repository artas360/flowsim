#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <queue>
#include <vector>

#include "include.hpp"
#include "allocator.hpp"
#include "event_types.hpp"
#include "random_generator.cpp"


// Reversed order comparasion
template <typename T1, typename T2>
struct event_comparison {
    bool operator() (T1 *lhs, T2 *rhs) const {
        return (lhs->get_handling_time() > rhs->get_handling_time());
    }
};

template <class Simulation>
class Event_manager {
    public:
        typedef float event_time_t;
        typedef Event<Event_manager<Simulation>> event_t;
        // Can't iterate over std::priority_queue
        typedef std::priority_queue<event_t*, std::vector<event_t*>, event_comparison<event_t, event_t>> event_queue;
        typedef typename Simulation::result_t result_t;
        typedef typename Simulation::flow_controller_t flow_controller_t;
        typedef typename Simulation::flow_key_t flow_key_t;
        typedef typename Simulation::topology_t::node_key_t node_key_t;

        Event_manager(typename Simulation::flow_controller_t & flow_controller) : allocator_(),
                                                                                       event_list_(),
                                                                                       EOS_(false),
                                                                                       result_(),
                                                                                       flow_controller_(flow_controller),
                                                                                       time_elapsed_(0){
        }

        ~Event_manager() {
            while( not event_list_.empty() ){
                event_t *event = event_list_.top();
                event_list_.pop();
                allocator_.destroy(event);
            }
        }

        void handle_next_event(){
            if (event_list_.empty()) {
                EOS_ = true;
                return;
            }

            event_t *event = event_list_.top();
            time_elapsed_ = event->get_handling_time();

            result_.record_value(std::string("time_elapsed"),
                                 result_.get_general_key(),
                                 time_elapsed_);

            event->handle_event();
            event->automated_update_result();
            event->post_handle();

            event_list_.pop();
            allocator_.destroy(event);
        }

        template <typename event_type_t, typename... Args>
         void add_event(typename event_type_t::event_issuer_t &event_issuer, Args... param) {
             event_list_.emplace(allocator_.construct<event_type_t>(*this, event_issuer, param...));
        }

        void init_run() {
            for(auto node_key: flow_controller_.get_entry_nodes()) {
                add_event<Arrival_event>(node_key);
            }

            result_.add_computed_value(false,
                                       std::string("Blocking_rate"),
                                       result_t::event_division,
                                       std::string("Flow_allocation_failure_event"),
                                       std::string("Arrival_event"),
                                       true);

            result_.register_convergence("Blocking_rate",
                                         convergence_number_samples_,
                                         convergence_epsilon_);

            // TODO Macro list of etypes
            // or enum + __class__ specialazation
            // for(class__ in enum)
            //  class__.register_new_result(result_);
        }

        void start_event_processing() {
            bool has_converged = false;
            size_t counter = 0;
            const std::string block_rate("Blocking_rate");

            init_run();

            while(not EOS_ and not has_converged){
                handle_next_event();
            }

            while (not EOS_ and not has_converged) {
                handle_next_event();

                if(++counter >= convergence_check_interval_) {
                    has_converged = result_.check_convergence(block_rate);
                    counter = 0;
                }

            }
        }

        inline event_time_t const& get_time_elapsed() const {
            return time_elapsed_;
        }

        Random_generator<event_time_t> get_random_generator() {
            return rand_generator_;
        }

        void set_EOS() {
            EOS_ = true;
        }

        flow_controller_t & get_flow_controller() {
            return flow_controller_;
        }

        typename Simulation::result_t & get_result() {
            return result_;
        }

        node_key_t const get_random_exit_node(node_key_t const& different_from) {
            int loop_prevention = 100;
            node_key_t node_key;

            do{
                node_key = flow_controller_.get_topology().get_random_entry_node(rand_generator_.rand_int());
            } while(node_key == different_from and --loop_prevention);

            if(not loop_prevention)
                throw Loop_error();

            return node_key;
        }

        inline bool new_arrivals() const {
            return true;
        }

#if TEST_EVENT
        event_t* const& top() const {
            return event_list_.top();
        }
#endif

    private:
        typedef FooAllocator Allocator;

        Allocator allocator_;
        event_queue event_list_;
        bool EOS_;
        result_t result_;
        flow_controller_t & flow_controller_;
        event_time_t time_elapsed_;
        Random_generator<event_time_t> rand_generator_;

        // Convergence Params
        size_t convergence_check_interval_;
        size_t convergence_number_samples_;
        typename result_t::result_value_t convergence_epsilon_;
};

#endif

#if TEST_EVENT

template <class Simulation>
std::ostream& operator<< (std::ostream &out, Event_manager<Simulation> const& event_manager) {
    out << "reading events" << std::endl;
    for(auto it: event_manager) {
        out << it->get_handling_time() << std::endl;
    }
    out << "Top event" << std::endl;
    out << event_manager.top()->get_handling_time() << std::endl;
    return out;
}

struct FooFlowKey{
};

struct FooResult {
    typedef float result_value_t;
    void increase_value(std::string, Node<>) {
    }
};

struct FooFlow {
};

struct FooFlowController {
    void free_flow(FooFlowKey) {
    }
};

struct FooTopology {
    typedef int node_key_t;
};

struct FooSimulation {
    typedef FooFlowKey flow_key_t;
    typedef FooResult result_t;
    typedef FooFlowController flow_controller_t;
    typedef FooTopology topology_t;
};

int test_event() {
    FooFlowController ffc;
    Event_manager<FooSimulation> evt(ffc);
    Node<> node(0.1, 0.2);
    FooFlowKey flow;
    Arrival_event<decltype(evt), Node<>> b(evt, node);
    End_flow_event<decltype(evt), Node<>> c(evt, node, 10, flow);
    End_of_simulation_event<decltype(evt), Node<>> d(evt, node, 1);
    Flow_allocation_success_event<decltype(evt), Node<>> e(evt, node, flow);
    Flow_allocation_failure_event<decltype(evt), Node<>> f(evt, node);

    evt.add_event<decltype(c)>(node, 10, flow); 
    FTEST(evt.top()->get_handling_time() == 10);
    evt.add_event<decltype(d)>(node, 1); 
    FTEST(evt.top()->get_handling_time() == 1);
    evt.add_event<decltype(b)>(node); 
    FTEST(evt.top()->get_handling_time() == 0);
    evt.add_event<decltype(e)>(node, flow); 
    FTEST(evt.top()->get_handling_time() == decltype(evt)::event_t::immediate_handling());
    evt.add_event<decltype(f)>(node); 
    FTEST(evt.top()->get_handling_time() == decltype(evt)::event_t::immediate_handling());
    evt.start_event_processing();
    // Check with valgrind that if was free'd
    evt.add_event<decltype(b)>(node); 
    

    return EXIT_SUCCESS;
}

int main() {
    return test_event();
}

#endif 
