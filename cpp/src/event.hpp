#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <boost/heap/priority_queue.hpp>

#include "include.hpp"
#include "allocator.hpp"
#include "result.hpp"
#include "event_types.hpp"

// Reversed order comparasion
template <typename T1, typename T2>
struct event_comparison {
    bool operator() (T1 *lhs, T2 *rhs) const {
        return (lhs->get_delay() > rhs->get_delay());
    }
};

template <class Simulation>
class Event_manager {
    public:
        typedef typename Simulation::result_t result_t;
        typedef typename Simulation::flow_controller_t flow_controller_t;
        typedef typename Simulation::flow_t flow_t;

        Event_manager(typename Simulation::flow_controller_t const& flow_controller) : allocator_(), event_list_(), EOS_(false), result_(),  flow_controller_(flow_controller){
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
            typename event_t::event_time_t time_elapsed = event->get_delay();
            for (typename event_queue::iterator it = event_list_.begin(); it != event_list_.end(); ++it) {
                (*it)->pass_time(time_elapsed);
                // No heap fixup since not changing order // s_handle_from_iterator
                // TODO : update time elapsed in results
            }

            event->handle_event();
            event->automated_update_result();
            event_list_.pop();
            allocator_.destroy(event);
        }

        template <typename event_type_t, typename... Args>
         void add_event(typename event_type_t::event_issuer_t &event_issuer, Args... param) {
             event_list_.emplace(allocator_.construct<event_type_t>(*this, event_issuer, param...));
        }

        void start_event_processing() {
            bool has_converged = false;
            //long counter = 0;

            // for(;;) {}
            while (not EOS_ and not has_converged) {
                handle_next_event();
            }
        }

        void set_EOS() {
            EOS_ = true;
        }

        typename Simulation::result_t const& get_result() {
            return result_;
        }

        bool new_arrivals() const;

    private:
        typedef FooAllocator Allocator;
        typedef Event<Event_manager<Simulation>> event_t;
        typedef typename boost::heap::priority_queue<event_t*, boost::heap::compare<event_comparison<event_t, event_t>>> event_queue;

        Allocator allocator_;
        event_queue event_list_;
        bool EOS_;
        result_t result_;
        flow_controller_t const& flow_controller_;
};

#endif

#if TEST

unsigned long Node::counter = 0;

struct FooResult {
};

struct FooFlow {
};

struct FooFlowController {
};

struct FooSimulation {
    typedef FooFlow flow_t;
    typedef FooResult result_t;
    typedef FooFlowController flow_controller_t;
};

int test_event() {
    Event_manager<FooSimulation> evt((FooFlowController()));
    Node node(0.1, 0.2);
    FooFlow flow;
    Arrival_event<decltype(evt), Node> b(evt, node);
    End_flow_event<decltype(evt), Node> c(evt, node, 10, flow);
    End_of_simulation_event<decltype(evt), Node> d(evt, node, 10);
    Flow_allocation_success_event<decltype(evt), Node> e(evt, node, flow);
    Flow_allocation_failure_event<decltype(evt), Node> f(evt, node);

    evt.add_event<decltype(b)>(node); 
    evt.add_event<decltype(c)>(node, 10, flow); 
    evt.add_event<decltype(d)>(node, 10); 
    evt.add_event<decltype(e)>(node, flow); 
    evt.add_event<decltype(f)>(node); 
    evt.start_event_processing();
    evt.add_event<decltype(b)>(node); 

    return EXIT_SUCCESS;
}

int main() {
    return test_event();
}

#endif 
