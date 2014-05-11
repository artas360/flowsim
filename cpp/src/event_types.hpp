#ifndef __EVENT_TYPES_HPP__
#define __EVENT_TYPES_HPP__

#include <limits>
#include <string>
#include <boost/lexical_cast.hpp>

#include "include.hpp"
#include "node.hpp"
#include "random_generator.hpp"
#include "result.hpp"
#include "flow_controller.hpp"
#include "config.hpp"

#define INSTANCIATE_EVENT_CLASS_NAME(X) template<class Event_manager, class Event_issuer>\
                                  const std::string X<Event_manager, Event_issuer>::__name__(#X);
#define INSTANCIATE_USER_EVENT_NAME(X) template<class Event_manager>\
                                  const std::string X<Event_manager>::__name__(#X);
template <class Event_manager>
class Event {
    public:
        typedef typename Event_manager::event_time_t event_time_t;

        // Usefull for construction?
        Event(Event_manager &event_manager, event_time_t end_event_time, event_time_t handling_time) : event_manager_(event_manager), end_event_time_(end_event_time), handling_time_(handling_time) {}
        virtual ~Event() {}

        virtual void handle_event() = 0;
        virtual void automated_update_result() = 0;

        virtual void update_result() {
        }

        virtual event_time_t get_handling_time() const {
            return handling_time_;
        }

        virtual event_time_t get_end_event_time() const {
            return end_event_time_;
        }

        virtual void post_handle() {
        }

        virtual std::string const& __class__() const = 0;

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
        typedef typename std::remove_reference<Event_issuer>::type event_issuer_t;
        typedef typename Event<Event_manager>::event_time_t event_time_t;

        Specialized_event(Event_manager &event_manager, Event_issuer const& event_issuer, event_time_t end_event_time, event_time_t handling_time) : Event<Event_manager>(event_manager, end_event_time, handling_time), event_issuer_(event_issuer) {
        }
        virtual ~Specialized_event() {}

        virtual void automated_update_result() {
            this->get_event_manager().get_result().increase_value(this->__class__(),
                                                                  this->get_event_manager().get_result().get_general_key());
            this->get_event_manager().get_result().increase_value(this->__class__(),
                                                                  std::get<0>(event_issuer_));
        }

    // TODO  protected

    public:

        event_issuer_t const& get_event_issuer() {
            return event_issuer_;
        }

        virtual std::string const& __class__() const {
            return __name__;
        }

    private:
        // Not ref for storing std::pair
        const event_issuer_t event_issuer_;
        static const std::string __name__; 
};
INSTANCIATE_EVENT_CLASS_NAME(Specialized_event)


template <class Event_manager, class Event_issuer>
class End_flow_event : public Specialized_event<Event_manager, Event_issuer> {
    typedef typename Event_manager::flow_key_t flow_t;
    typedef typename Event_manager::flow_controller_t flow_controller_t;

    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;
        End_flow_event(Event_manager &event_manager, Event_issuer const& event_issuer, event_time_t handling_time, flow_t &flow) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, handling_time), flow_(flow) {
        }

        void handle_event() {
            this->get_event_manager().get_flow_controller().free_flow(flow_);
        }

        std::string const& __class__() const {
            return __name__;
        }

    private:
        flow_t flow_;
        static const std::string __name__;
};
INSTANCIATE_EVENT_CLASS_NAME(End_flow_event)


template <class Event_manager, class Event_issuer>
class End_of_simulation_event : public Specialized_event<Event_manager, Event_issuer> {
    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;

        End_of_simulation_event(Event_manager &event_manager, Event_issuer const& event_issuer, event_time_t handling_time) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, handling_time) {
        }

        void handle_event() {
            this->get_event_manager().set_EOS();
        }

        std::string const& __class__() const {
            return __name__;
        }

    private:
        static const std::string __name__;
};
INSTANCIATE_EVENT_CLASS_NAME(End_of_simulation_event)


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

        std::string const& __class__() const {
            return __name__;
        }

    private:
        flow_t flow_;
        static const std::string __name__;
};
INSTANCIATE_EVENT_CLASS_NAME(Flow_allocation_success_event)


template <class Event_manager, class Event_issuer>
class Flow_allocation_failure_event: public Specialized_event<Event_manager, Event_issuer> {
    public:
        Flow_allocation_failure_event(Event_manager &event_manager, Event_issuer const& event_issuer) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, event_manager.get_immediate_handling()) {
        }

        void handle_event() {
        }

        void update_result() {
            this->get_event_manager().get_result().increase_value(this->__class__(),
                                                                  std::get<0>(this->get_event_issuer()));
        }

        std::string const& __class__() const {
            return __name__;
        }

    private:
        static const std::string __name__;
};
INSTANCIATE_EVENT_CLASS_NAME(Flow_allocation_failure_event)


template <class Event_manager, class Event_issuer>
class Arrival_event : public Specialized_event<Event_manager, Event_issuer> {
    typedef typename Event_manager::flow_key_t flow_t;

    public:
        Arrival_event(Event_manager &event_manager, Event_issuer const& event_issuer) : Specialized_event<Event_manager, Event_issuer>(event_manager, event_issuer, 0, 0) {
            arrival_rate_ = std::get<1>(event_issuer).get().get_arrival_rate();
            service_rate_ = std::get<1>(event_issuer).get().get_service_rate();
            this->set_handling_time(event_manager.get_random_generator().next_arrival(arrival_rate_) + this->get_event_manager().get_time_elapsed());
            this->set_end_event_time(event_manager.get_random_generator().rand_duration(service_rate_) + this->get_handling_time());
            assert(this->get_handling_time() != this->get_end_event_time());
        }

        void handle_event() {
            flow_t flow;
            const typename Event_manager::node_key_t node_key = std::get<0>(this->get_event_issuer());
            if(this->get_event_manager().new_arrivals()) {
                this->get_event_manager().template add_event<Arrival_event>(this->get_event_issuer());
            }

            typename Event_manager::node_key_t const& dst_node = this->get_event_manager().get_random_exit_node(node_key);
            
            flow = this->get_event_manager().get_flow_controller().allocate_flow(node_key, dst_node);
            if (not Event_manager::flow_controller_t::key_generator_t::is_valid_key(flow)) {
                this->get_event_manager().template add_event<Flow_allocation_failure_event<Event_manager, Event_issuer>>(this->get_event_issuer());
            } else {
                // /!\ WARNING /!\ Make sure that success is handled before end_flow
                // Else it will .length an non-existing flow!
                this->get_event_manager().template add_event<Flow_allocation_success_event<Event_manager, Event_issuer>>(this->get_event_issuer(), flow);
                this->get_event_manager().template add_event<End_flow_event<Event_manager, Event_issuer>>(this->get_event_issuer(), this->get_end_event_time(), flow);
            }
        }

        std::string const& __class__() const {
            return __name__;
        }

    private:
        float arrival_rate_;
        float service_rate_;
        static const std::string __name__;
};
INSTANCIATE_EVENT_CLASS_NAME(Arrival_event)


// Shouldn't inherit User_event or infinite loop
template <class Event_manager>
class Sample_event : public Event<Event_manager> {
    typedef typename Event<Event_manager>::event_time_t event_time_t;
    public:
        Sample_event(Event_manager &event_manager, event_time_t time_interval, event_time_t handling_time) : Event<Event_manager>(event_manager, time_interval, handling_time) {
        }

        void handle_event() {
            // TODO result.take_snapshot()
            static std::string blocking_rate_s("Blocking_rate");
            std::cout << this->get_event_manager().get_result().get(this->get_event_manager().get_result().get_general_key(),
                                                                    blocking_rate_s) << std::endl;
            this->get_event_manager().template add_event<Sample_event<Event_manager>>(this->get_end_event_time(),
                                                                                      this->get_handling_time() + this->get_end_event_time());
        }

        virtual void automated_update_result() {
        }

        virtual std::string const& __class__() const {
            return __name__;
        }

    private:
        static const std::string __name__;
};
INSTANCIATE_USER_EVENT_NAME(Sample_event)


template <class Event_manager>
class User_event : public Event<Event_manager> {
    public:
        typedef typename Event<Event_manager>::event_time_t event_time_t;

        User_event(Event_manager &event_manager, event_time_t end_event_time, event_time_t handling_time) : Event<Event_manager>(event_manager, end_event_time, handling_time) {
            this->get_event_manager().new_user_event();
        }

        ~User_event() {
            // Is it a good idea in case of mem pool ?
            this->get_event_manager().handled_user_event();
        }

        virtual void automated_update_result() {
            this->get_event_manager().get_result().increase_value(this->__class__(),
                                                                  this->get_event_manager().get_result().get_general_key());
        }

        virtual std::string const& __class__() const {
            return __name__;
        }

    private:
        static const std::string __name__;
};
INSTANCIATE_USER_EVENT_NAME(User_event)


template <class Event_manager>
class Watcher_event : public User_event<Event_manager> {
    public:
        typedef typename User_event<Event_manager>::event_time_t event_time_t;
        Watcher_event(Event_manager &event_manager, event_time_t handling_time) : User_event<Event_manager>(event_manager, 0, handling_time) {
        }

        void handle_event() {
        }

        virtual std::string const& __class__() const {
            return __name__;
        }

    private:
        static const std::string __name__;
};
INSTANCIATE_USER_EVENT_NAME(Watcher_event)


template <class Event_manager>
class Arrival_burst_event : public User_event<Event_manager> {
    typedef typename Event_manager::rate_t rate_t;
    typedef typename Event_manager::node_t::id_t node_id_t;
    public:
        typedef typename User_event<Event_manager>::event_time_t event_time_t;
        Arrival_burst_event(Event_manager &event_manager, event_time_t handling_time, node_id_t target, rate_t new_arrival_rate) : User_event<Event_manager>(event_manager, 0, handling_time), target_(target), new_arrival_rate_(new_arrival_rate) {
        }

        void handle_event() {
            this->get_event_manager().get_flow_controller().get_topology().swap_node_arr_rate(target_, new_arrival_rate_);
        }

        virtual std::string const& __class__() const {
            return __name__;
        }

    private:
        node_id_t target_;
        rate_t new_arrival_rate_;
        static const std::string __name__;
};
INSTANCIATE_USER_EVENT_NAME(Arrival_burst_event)


template<class Event_manager>
class User_event_analyzer {
    public:
        typedef typename config_list::value_type event_description_t;
        typedef typename event_description_t::key_type description_key_t;
        typedef typename event_description_t::mapped_type description_value_t;
        typedef typename Event_manager::event_time_t event_time_t;
        typedef typename Event_manager::node_t::id_t node_id_t;

        User_event_analyzer(Event_manager &event_manager) : event_manager_(event_manager) {
        }

        void analyze(event_description_t const& event_description) {
            static description_key_t trigger_type_s("trigger_type"); 
            try {
                description_value_t trigger_type = event_description.at(trigger_type_s);
                if (trigger_type == std::string("time")) {
                    analyze_time_event(event_description);
                }
                else {
                    throw Configuration_error("In Event description: unknown trigger_type.");
                }

            } catch (std::out_of_range const&) {
                throw Configuration_error("Missing event_type in event description.");
            }
        }

        void analyze_time_event(event_description_t const& event_description) {
            static description_key_t trigger_value_s("trigger_value"),
                                     event_target_s("event_target"),
                                     effect_value_s("effect_value"),
                                     type_s("type");
            description_value_t type;
            event_time_t handling_time;
            // float most generic?
            float effect_value;
            // Only node target ???
            node_id_t event_target;
            std::string tmp;
            try {
                type = boost::lexical_cast<std::string, description_value_t>(event_description.at(type_s));
                handling_time = boost::lexical_cast<event_time_t, description_value_t>(event_description.at(trigger_value_s));
                effect_value = boost::lexical_cast<float, description_value_t>(event_description.at(effect_value_s));
                if((tmp = event_description.at(event_target_s)) != std::string("None"))
                    event_target = boost::lexical_cast<node_id_t, description_value_t>(tmp);
            } catch (std::out_of_range const&) {
                throw Configuration_error("Missing field in event description.");
            } catch (boost::bad_lexical_cast const&) {
                throw Configuration_error("Invalid field in Simulation configuration.");
            }

            if(type == std::string("arrival_burst_event")) {
                event_manager_.template add_event<Arrival_burst_event<Event_manager>>(handling_time,
                                                                                      event_manager_.get_flow_controller().get_topology().id_to_key(event_target),
                                                                                      effect_value);
            } else if(type == std::string("sample_event")) {
                event_manager_.template add_event<Sample_event<Event_manager>>(effect_value,
                                                                               handling_time);
            } else if(type == std::string("watcher_event")) {
                event_manager_.template add_event<Watcher_event<Event_manager>>(handling_time);
            } else {
                throw Configuration_error("Unknown event type.");
            }
        }

    private:
        Event_manager &event_manager_;
};


#endif
