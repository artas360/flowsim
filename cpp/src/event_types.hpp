#ifndef __EVENT_TYPES_HPP__
#define __EVENT_TYPES_HPP__

#include <limits>

#include "include.hpp"
#include "node.hpp"

template <class Event_manager>
class Event {
    public:
        typedef float event_time_t;

        // Usefull for construction?
        Event(Event_manager &event_manager, event_time_t duration, event_time_t delay_before_handling) : event_manager_(event_manager), duration_(duration), delay_before_handling_(delay_before_handling) {}
        virtual ~Event() {}

        template <typename... Args>
         void init(Args... params);
        virtual void update_result() = 0;
        virtual void handle_event() = 0;

        static event_time_t immediate_handling() {
            return std::numeric_limits<event_time_t>::infinity();
        }

        virtual event_time_t get_delay() const {
            return delay_before_handling_;
        }

        virtual int get_duration() const {
            return duration_;
        }

        virtual void automated_update_result() {
        }

        virtual void pass_time(event_time_t delay) {
            delay_before_handling_ -= delay;
        }

    private:
        Event_manager &event_manager_;
        event_time_t duration_;
        event_time_t delay_before_handling_;
};

template <class Event_manager, class Event_issuer>
class Specialized_event : Event<Event_manager>{

    public:
        typedef Event_issuer event_issuer_t;
        typedef typename Event<Event_manager>::event_time_t event_time_t;

        Specialized_event(Event_manager &event_manager, Event_issuer &event_issuer, event_time_t duration, event_time_t delay_before_handling) : Event<Event_manager>(event_manager, duration, delay_before_handling), event_issuer_(event_issuer) {}
        virtual ~Specialized_event() {}
    private:

        event_issuer_t &event_issuer_;
};

template <class Event_manager, class Event_issuer=Node>
class Arrival_event : public Specialized_event<Event_manager, Event_issuer>{
    public:
        Arrival_event(Event_manager &event_manager, Event_issuer &event_issuer) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, 0) {
            arrival_rate_ = event_issuer.get_arrival_rate();
            service_rate_ = event_issuer.get_service_rate();
        }

        void update_result() {
        }

        void handle_event() {
        }

    private:
        float arrival_rate_;
        float service_rate_;
};

template <class Event_manager, class Event_issuer=Node>
class End_flow_event : public Specialized_event<Event_manager, Event_issuer> {
    typedef typename Event_manager::flow_t flow_t;
    typedef typename Event_manager::flow_controller_t flow_controller_t;

    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;
        End_flow_event(Event_manager &event_manager, Event_issuer &event_issuer, event_time_t delay_before_handling, flow_t &flow) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, delay_before_handling), flow_(flow) {}

        void update_result() {
        }

        void handle_event() {
        }

    private:
        flow_t &flow_;
};

template <class Event_manager, class Event_issuer>
class End_of_simulation_event : public Specialized_event<Event_manager, Event_issuer> {
    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;

        End_of_simulation_event(Event_manager &event_manager, Event_issuer &event_issuer, event_time_t delay_before_handling) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, delay_before_handling) {}

        void update_result() {
        }

        void handle_event() {
        }
};

template <class Event_manager, class Event_issuer>
class Flow_allocation_success_event: public Specialized_event<Event_manager, Event_issuer> {
    public:
        typedef typename Event_manager::flow_t flow_t;

        Flow_allocation_success_event(Event_manager &event_manager, Event_issuer &event_issuer, flow_t &flow) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, Event<Event_manager>::immediate_handling()), flow_(flow) {}

        void update_result() {
        }

        void handle_event() {
        }

    private:
        flow_t &flow_;
};

template <class Event_manager, class Event_issuer>
class Flow_allocation_failure_event: public Specialized_event<Event_manager, Event_issuer> {
    public:
        Flow_allocation_failure_event(Event_manager &event_manager, Event_issuer &event_issuer) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, Event<Event_manager>::immediate_handling()) {}

        void update_result() {
        }

        void handle_event() {
        }
};

#endif
