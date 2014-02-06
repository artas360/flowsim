#ifndef __EVENT_TYPES_HPP__
#define __EVENT_TYPES_HPP__

#include "node.hpp"

template <class Event_manager>
class Event {
    typedef float time_t;
    private:
        time_t duration_;
        time_t delay_before_handling_;
        Event_manager event_manager_;
        
    public:
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

template <class Event_manager, class Event_issuer>
class Specialized_event : Event<Event_manager>{
    typedef Event_issuer event_issuer_t;
    private:
        event_issuer_t event_issuer_;
};

template <class Event_manager, class Event_issuer=Node>
class Arrival_event : Specialized_event<Event_manager, Event_issuer>{
    public:
        Arrival_event(Event_manager event_manager, Event_issuer event_issuer);
};

#endif
