#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <boost/heap/priority_queue.hpp>
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
    typedef FooAllocator Allocator;
    typedef Event<Event_manager<Simulation>> event_t;
    typedef typename boost::heap::priority_queue<event_t*, boost::heap::compare<event_comparison<event_t, event_t>>> event_queue;

    private:
        Allocator allocator_;
        event_queue event_list_;
        bool EOS_;
        typename Simulation::result_t result_;
        typename Simulation::flow_controller_t const& flow_controller_;

    public:
        Event_manager(typename Simulation::flow_controller_t const& flow_controller) : allocator_(), event_list_(), EOS_(false), result_(),  flow_controller_(flow_controller){
        }

        void handle_next_event(){
            if (event_list_.empty())
                return;

            event_t *event = event_list_.top();
            typename event_t::time_t time_elapsed = event->get_delay();
            for (typename event_queue::iterator it = event_list_.begin(); it != event_list_.end(); ++it) {
                *it->pass_time(time_elapsed);
                // No heap fixup since not changing order // s_handle_from_iterator
                // TODO : update time elapsed in results
            }

            event->handle_event();
            event->automated_update_result();
            event_list_.pop();
            allocator_.free(event);
        }

        template <typename event_type_t, typename... Args>
         void add_event(typename event_type_t::event_issuer_t const& event_issuer, Args... param) {
             event_list_.emplace(allocator_.allocate<event_type_t>(event_issuer, param...));
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
};

#endif

#if TEST

struct FooResult {
};

struct FooFlowController {
};

struct FooSimulation {
    typedef FooResult result_t;
    typedef FooFlowController flow_controller_t;
};

int main() {
    Event_manager<FooSimulation> evt((FooFlowController()));
    Arrival_event<decltype(evt)> a(evt, Node(1));
    return EXIT_SUCCESS;
}

#endif 
