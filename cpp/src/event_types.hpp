#ifndef __EVENT_TYPES_HPP__
#define __EVENT_TYPES_HPP__

#include <limits>
#include <string>

#include "include.hpp"
#include "node.hpp"
#include "random_generator.hpp"
#include "result.hpp"
#include "flow_controller.hpp"

#define INSTANCIATE_CLASS_NAME(X) ;
//template<>\
//std::string __class__(#X const&) {\
//    return std::string("#X");\
//}

template <class etype>
std::string __class__(etype const&) {
    return std::string();
}

template <class Event_manager>
class Event {
    public:
        typedef typename Event_manager::event_time_t event_time_t;

        // Usefull for construction?
        Event(Event_manager &event_manager, event_time_t end_event_time, event_time_t handling_time) : event_manager_(event_manager), end_event_time_(end_event_time), handling_time_(handling_time) {}
        virtual ~Event() {}

        template <typename... Args>
         void init(Args... params);
        virtual void handle_event() = 0;
        virtual void automated_update_result() = 0;

        virtual void update_result() {
        }

        virtual event_time_t get_handling_time() const {
            return handling_time_;
        }

        virtual int get_end_event_time() const {
            return end_event_time_;
        }

        virtual void post_handle() {
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

        Specialized_event(Event_manager &event_manager, Event_issuer const& event_issuer, event_time_t end_event_time, event_time_t handling_time) : Event<Event_manager>(event_manager, end_event_time, handling_time), event_issuer_(event_issuer) {}
        virtual ~Specialized_event() {}

        virtual void automated_update_result() {
            this->get_event_manager().get_result().increase_value(this->__class__(),
                                                                  this->get_event_manager().get_result().get_general_key());
            this->get_event_manager().get_result().increase_value(this->__class__(),
                                                                  std::get<0>(event_issuer_));
        }

        std::string __class__() const {
            return std::string("End_flow_event");
        }

    protected:

        event_issuer_t & get_event_issuer() {
            return event_issuer_;
        }

    private:
        // Not ref for storing std::pair
        event_issuer_t event_issuer_;
};


template <class Event_manager, class Event_issuer>
class End_flow_event : public Specialized_event<Event_manager, Event_issuer> {
    typedef typename Event_manager::flow_key_t flow_t;
    typedef typename Event_manager::flow_controller_t flow_controller_t;

    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;
        End_flow_event(Event_manager &event_manager, Event_issuer const& event_issuer, event_time_t handling_time, flow_t &flow) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, handling_time), flow_(flow) {}

        void handle_event() {
            this->get_event_manager().get_flow_controller().free_flow(flow_);
        }

        std::string __class__() const {
            return std::string("End_flow_event");
        }

    private:
        flow_t flow_;
};
INSTANCIATE_CLASS_NAME(End_flow_event)


template <class Event_manager, class Event_issuer>
class End_of_simulation_event : public Specialized_event<Event_manager, Event_issuer> {
    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;

        End_of_simulation_event(Event_manager &event_manager, Event_issuer const& event_issuer, event_time_t handling_time) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, handling_time) {}

        void handle_event() {
            this->get_event_manager().set_EOS();
        }

        std::string __class__() const {
            return std::string("End_of_simulation_event");
        }
};
INSTANCIATE_CLASS_NAME(End_of_simulation_event)


template <class Event_manager, class Event_issuer>
class Flow_allocation_success_event: public Specialized_event<Event_manager, Event_issuer> {
    public:
        typedef typename Event_manager::flow_key_t flow_t;

        Flow_allocation_success_event(Event_manager &event_manager, Event_issuer const& event_issuer, flow_t &flow) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, event_manager.get_immediate_handling()), flow_(flow) {
        }

        void handle_event() {
        }

        void update_result() {
            #if 0 //Need support for register_new_result
            this->get_event_manager().get_result().update_computed_value(std::string("mean_nodes_per_flow"),
                                                                         this->get_event_manager().get_result().get_general_key(),
                                                                         this->get_event_manager().get_flow_controller().get_flow(flow_).length());
            #endif
        }

        std::string __class__() const {
            return std::string("Flow_allocation_success_event");
        }

    private:
        flow_t flow_;
};
INSTANCIATE_CLASS_NAME(Flow_allocation_success_event)


template <class Event_manager, class Event_issuer>
class Flow_allocation_failure_event: public Specialized_event<Event_manager, Event_issuer> {
    public:
        Flow_allocation_failure_event(Event_manager &event_manager, Event_issuer const& event_issuer) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, event_manager.get_immediate_handling()) {}

        void handle_event() {
        }

        void update_result() {
            this->get_event_manager().get_result().increase_value(this->__class__(),
                                                                  std::get<0>(this->get_event_issuer()));
        }

        std::string __class__() const {
            return std::string("Flow_allocation_failure_event");
        }
};
INSTANCIATE_CLASS_NAME(Flow_allocation_failure_event)


template <class Event_manager, class Event_issuer>
class Arrival_event : public Specialized_event<Event_manager, Event_issuer> {
    typedef typename Event_manager::flow_key_t flow_t;

    public:
        Arrival_event(Event_manager &event_manager, Event_issuer const& event_issuer) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, 0) {
            arrival_rate_ = std::get<1>(event_issuer).get_arrival_rate();
            service_rate_ = std::get<1>(event_issuer).get_service_rate();
            this->set_handling_time(event_manager.get_random_generator().next_arrival(arrival_rate_) + this->get_event_manager().get_time_elapsed());
            this->set_end_event_time(event_manager.get_random_generator().rand_duration(service_rate_) + this->get_handling_time());
        }

        void handle_event() {
            flow_t flow;
            const typename Event_manager::node_key_t node_key = std::get<0>(this->get_event_issuer());
            if(this->get_event_manager().new_arrivals()) {
                this->get_event_manager().template add_event<Arrival_event>(this->get_event_issuer());
            }

            typename Event_manager::node_key_t const& dst_node = this->get_event_manager().get_random_exit_node(node_key);

            try {
                flow = this->get_event_manager().get_flow_controller().allocate_flow(node_key, dst_node);
                assert(Event_manager::flow_controller_t::key_generator_t::is_valid_key(flow));
            } catch (No_path_error e) {
                this->get_event_manager().template add_event<Flow_allocation_failure_event<Event_manager, Event_issuer>>(this->get_event_issuer());
                return;
            }

            // /!\ WARNING /!\ Make sure that success is handled before end_flow
            // Else it will .length an non-existing flow!
            this->get_event_manager().template add_event<Flow_allocation_success_event<Event_manager, Event_issuer>>(this->get_event_issuer(), flow);
            this->get_event_manager().template add_event<End_flow_event<Event_manager, Event_issuer>>(this->get_event_issuer(), this->get_end_event_time(), flow);
        }

        std::string __class__() const {
            return std::string("Arrival_event");
        }

    private:
        float arrival_rate_;
        float service_rate_;
};
INSTANCIATE_CLASS_NAME(Arrival_event)


// TODO: User events

#endif
