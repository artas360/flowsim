#ifndef __EVENT_TYPES_HPP__
#define __EVENT_TYPES_HPP__

#include <limits>

#include "include.hpp"
#include "node.hpp"
#include "random_generator.hpp"


template <class Event_manager>
class Event {
    public:
        typedef typename Event_manager::event_time_t event_time_t;

        // Usefull for construction?
        Event(Event_manager &event_manager, event_time_t end_event_time, event_time_t handling_time) : event_manager_(event_manager), end_event_time_(end_event_time), handling_time_(handling_time) {}
        virtual ~Event() {}

        template <typename... Args>
         void init(Args... params);
        virtual void update_result() = 0;
        virtual void handle_event() = 0;

        event_time_t const& immediate_handling() const {
            //return -std::numeric_limits<event_time_t>::infinity();
            return event_manager_.get_time_elapsed();
        }

        virtual event_time_t get_handling_time() const {
            return handling_time_;
        }

        virtual int get_end_event_time() const {
            return end_event_time_;
        }

        virtual void automated_update_result() {
        }

    protected:

        virtual void set_handling_time(event_time_t const& handling_time) {
            handling_time_ = handling_time;
        }

        virtual void set_end_event_time(event_time_t const& end_event_time) {
            end_event_time_ = end_event_time;
        }

        virtual Event_manager & get_event_manager() {
            return event_manager_;
        }

    private:
        Event_manager &event_manager_;
        event_time_t end_event_time_;
        event_time_t handling_time_;
};


template <class Event_manager, class Event_issuer>
class Specialized_event : public Event<Event_manager>{

    public:
        typedef Event_issuer event_issuer_t;
        typedef typename Event<Event_manager>::event_time_t event_time_t;

        Specialized_event(Event_manager &event_manager, Event_issuer &event_issuer, event_time_t end_event_time, event_time_t handling_time) : Event<Event_manager>(event_manager, end_event_time, handling_time), event_issuer_(event_issuer) {}
        virtual ~Specialized_event() {}

    protected:

        event_issuer_t & get_event_issuer() {
            return event_issuer_;
        }

    private:
        event_issuer_t &event_issuer_;
};

// TODO : change Node to node_key_t
template <class Event_manager, class Event_issuer=Node<>>
class Arrival_event : public Specialized_event<Event_manager, Event_issuer> {

    public:
        Arrival_event(Event_manager &event_manager, Event_issuer &event_issuer) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, 0) {
            arrival_rate_ = event_issuer.get_arrival_rate();
            service_rate_ = event_issuer.get_service_rate();
            this->set_end_event_time(event_manager.get_random_generator().rand_duration(service_rate_) + this->get_event_manager().get_time_elapsed());
            this->set_handling_time(event_manager.get_random_generator().next_arrival(arrival_rate_) + this->get_event_manager().get_time_elapsed());
        }

        void update_result() {
            this->get_event_manager().get_result().increase_value(std::string("Arrival_event"), this->get_event_issuer());
        }

        void handle_event() {
            this->get_event_manager().template add_event<Arrival_event>(this->get_event_issuer());
        }

    private:
        float arrival_rate_;
        float service_rate_;
};


template <class Event_manager, class Event_issuer=Node<>>
class End_flow_event : public Specialized_event<Event_manager, Event_issuer> {
    typedef typename Event_manager::flow_key_t flow_t;
    typedef typename Event_manager::flow_controller_t flow_controller_t;

    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;
        End_flow_event(Event_manager &event_manager, Event_issuer &event_issuer, event_time_t handling_time, flow_t &flow) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, handling_time), flow_(flow) {}

        void update_result() {
            this->get_event_manager().get_result().increase_value(std::string("End_flow_event"), this->get_event_issuer());
        }

        void handle_event() {
            this->get_event_manager().get_flow_controller().free_flow(flow_);
        }

    private:
        flow_t flow_;
};


template <class Event_manager, class Event_issuer>
class End_of_simulation_event : public Specialized_event<Event_manager, Event_issuer> {
    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;

        End_of_simulation_event(Event_manager &event_manager, Event_issuer &event_issuer, event_time_t handling_time) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, handling_time) {}

        void update_result() {
        }

        void handle_event() {
            this->get_event_manager().set_EOS();
        }
};


template <class Event_manager, class Event_issuer>
class Flow_allocation_success_event: public Specialized_event<Event_manager, Event_issuer> {
    public:
        typedef typename Event_manager::flow_key_t flow_t;

        Flow_allocation_success_event(Event_manager &event_manager, Event_issuer &event_issuer, flow_t &flow) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, Event<Event_manager>::immediate_handling()), flow_(flow) {}

        void update_result() {
            this->get_event_manager().get_result().increase_value(std::string("Flow_allocation_success_event"), this->get_event_issuer());
            // TODO : access flow_key and flow.length 
            //this->get_event_manager().get_result().update_computed_value(std::string("mean_nodes_per_flow"), flow);
        }

        void handle_event() {
        }

    private:
        flow_t flow_;
};


template <class Event_manager, class Event_issuer>
class Flow_allocation_failure_event: public Specialized_event<Event_manager, Event_issuer> {
    public:
        Flow_allocation_failure_event(Event_manager &event_manager, Event_issuer &event_issuer) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, Event<Event_manager>::immediate_handling()) {}

        void update_result() {
            this->get_event_manager().get_result().increase_value(std::string("Flow_allocation_failure_event"), this->get_event_issuer());
        }

        void handle_event() {
        }
};

#endif
