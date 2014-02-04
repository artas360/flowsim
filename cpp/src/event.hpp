#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <boost/heap/priority_queue.hpp>
#include "allocator.hpp"

// Reversed order comparasion
template <typename T1, typename T2>
struct event_comparison {
    bool operator() (T1 *lhs, T2 *rhs) const {
        return (lhs->get_delay() > rhs->get_delay());
    }
};

template <class Event_manager, class Event_issuer>
class Event {
    typedef float time_t;
    private:
        time_t duration_;
        time_t delay_before_handling_;
        Event_manager event_manager_;
        Event_issuer event_issuer_;
        
    public:
        Event(Event_manager event_manager, Event_issuer event_issuer);
        // Usefull for construction?
        template <typename... Args>
         void init(Args... params);
        virtual int get_delay() const = 0;
        virtual int get_duration() const = 0;
        virtual void automated_update_result() = 0;
        virtual void update_result() = 0;
        virtual void handle_event() = 0;
        virtual void pass_time(time_t delay) = 0;
};

template <class Simulation>
class Event_manager {
    typedef FooAllocator Allocator;
    typedef Event<typename Simulation::Event_manager_t, typename Simulation::Event_issuer_t> event_t;
    typedef typename boost::heap::priority_queue<event_t*, boost::heap::compare<event_comparison<event_t, event_t>>> event_queue;

    private:
        Allocator allocator;
        event_queue event_list_;
        EOS = false;

    public:
        Event_manager() : event_list_(), allocator() {}

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
            allocator.free(event);
        }

        template <typename event_type_t, typename... Args>
         void add_event(typename Simulation::Event_issuer_t const& event_issuer, Args... param) {
             event_list_.emplace(allocator.allocate<event_type_t>(event_issuer, param...));
        }

        void start_event_processing(typename Simulation::result_t &result) {
            bool has_converged = false;
            long counter = 0;

            // for(;;) {}
            while (not EOS and not has_converged) {
                handle_next_event();
            }
        }

        void set_EOS();
        bool new_arrivals() const;
};

#endif
