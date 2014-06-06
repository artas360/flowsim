#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <queue>
#include <vector>
#include <type_traits>

#include "include.hpp"
#include "allocator.hpp"
#include "config_interface.hpp"
#include "key_generator.hpp"
#include "random_generator.hpp"
#include "result.hpp"
#include "event_types.hpp"

/**
 * \class event_comparison
 * \brief Reversed order comparison used to sort events by theit handling time.
 */
template <typename T1, typename T2>
struct event_comparison {
    bool operator() (T1 *lhs, T2 *rhs) const {
        return (lhs->get_handling_time() >= rhs->get_handling_time());
    }
};

/**
 * \class Event_manager
 * \brief Class recording, sorting and handling simulation events.
 */
template <class Flow_controller, class time=float, class result=Result<float>>
class Event_manager {
    public:
        typedef time event_time_t;
        typedef result result_t;
        typedef Flow_controller flow_controller_t;
        typedef typename flow_controller_t::flow_key_t flow_key_t;
        typedef typename flow_controller_t::topology_t::node_key_t node_key_t;
        typedef typename flow_controller_t::topology_t::node_t node_t;
        typedef typename node_t::rate_t rate_t;
        typedef Event<Event_manager<flow_controller_t, event_time_t, result_t>> event_t;
        typedef std::priority_queue<event_t*, std::vector<event_t*>, event_comparison<event_t, event_t>> event_queue;

        Event_manager(flow_controller_t & flow_controller) : allocator_(),
                                                             event_list_(),
                                                             EOS_(false),
                                                             remaining_user_events_(0),
                                                             result_(),
                                                             flow_controller_(flow_controller),
                                                             time_elapsed_(0),
                                                             convergence_check_interval_(1000),
                                                             convergence_number_samples_(10),
                                                             convergence_epsilon_(1e-3) {
        }

        ~Event_manager() {
            while( not event_list_.empty() ){
                event_t *event = event_list_.top();
                event_list_.pop();
                allocator_.destroy(event);
            }
        }

        /**
         * \brief Constructs and records an event.
         *
         *  Constructs and stores an event of type event_type_t with
         *  parameters param.
         * \param param Variadic argument to be expanded in event_type_t constructor.
         */
        template <typename event_type_t, typename... Args>
        //void add_event(typename event_type_t::event_issuer_t const& event_issuer, Args... param) {
        void add_event(Args&&... param) {
             event_list_.emplace(allocator_.construct<event_t, event_type_t>(*this, param...));
        }

        /**
         * \brief Starts event processing i.e. start the simulation.
         */
        void start_event_processing() {
            bool has_converged = false;
            size_t counter = 0;
            static const std::string block_rate("Blocking_rate");
            init_run();

            while (not EOS_ and not (has_converged and remaining_user_events_ == 0)) {
                handle_next_event();

                if(++counter >= convergence_check_interval_) {
                    has_converged = result_.check_convergence(block_rate);
                    counter = 0;
                }

            }
        }

        /**
         * \brief Returns virtual time elpsed in the simulation.
         * \return Virtual time elpsed in the simulation.
         */
        inline event_time_t const& get_time_elapsed() const {
            return time_elapsed_;
        }

        Random_generator<event_time_t> & get_random_generator() {
            return rand_generator_;
        }

        /**
         * \brief Stops the event handling loop i.e stops the simulation.
         */
        void set_EOS() {
            EOS_ = true;
        }

        flow_controller_t & get_flow_controller() {
            return flow_controller_;
        }

        result_t & get_result() {
            return result_;
        }

        /**
         * \brief Returns the key of a randomly chosen exit node in the topology.
         * \param different_from Key from which the returned key must be different from.
         * \return Key of an exit node in the topology associated to the Event_manager.
         * \warning Throws if it can not return a node different from the parameter different_from.
         */
        node_key_t const get_random_exit_node(node_key_t const& different_from) {
            int loop_prevention = 100;
            node_key_t node_key;

            do {
                node_key = flow_controller_.get_topology().get_random_exit_node(rand_generator_.rand_int());
            } while(node_key == different_from and --loop_prevention);

            if(not loop_prevention)
                throw Loop_error();

            return node_key;
        }

        /**
         * \brief Returns whether new packet should be generated by the nodes.
         * \return Always true, since the function is deprecated.
         * \deprecated
         */
        inline bool new_arrivals() const {
            return true;
        }

        /**
         * \brief Returns the time_t value to be used for immediatly handling an event.
         *
         * It is not guaranted that any event whose handling time has been set to
         * this value will be the next event handled. The only guaranty is that such an
         * event will be handled without increasing the virtual time elapsed in the
         * simulation.
         * \return time_t value to be used for immediatly handling an event.
         */
        event_time_t const& get_immediate_handling() const {
            return time_elapsed_;
        }

        /**
         * \brief Forwards the list of users to an User_event_analyzer.
         * \param conf container of all user events' description.
         */
        void load_user_events(config_list const& conf) {
            User_event_analyzer<Event_manager<flow_controller_t, event_time_t, result_t>> analyzer(*this);
            for(auto event: conf) {
                analyzer.analyze(event);
            }
        }

        /**
         * \brief Extracts all relevant parameters from conf and handles them.
         * \param conf container simulation parameters.
         */
        void load_simulation_config(config_list const& conf) {
            typedef typename config_list::value_type::mapped_type description_value_t;
            typedef typename config_list::value_type::key_type description_key_t;

            description_key_t convergence_s("Convergence"),
                              convergence_check_interval_s("check_interval"),
                              convergence_number_samples_s("number_samples"),
                              convergence_epsilon_s("epsilon");

            try {
                typename config_list::value_type convergence_description = conf[0];
                convergence_check_interval_ = boost::lexical_cast<size_t, description_value_t>(convergence_description.at(convergence_check_interval_s));
                convergence_epsilon_ = boost::lexical_cast<decltype(convergence_epsilon_), description_value_t>(convergence_description.at(convergence_epsilon_s));
                convergence_number_samples_ = boost::lexical_cast<size_t, description_value_t>(convergence_description.at(convergence_number_samples_s));
            } catch (std::out_of_range const&) {
                throw Configuration_error("Missing field in Simulation configuration.");
            } catch (boost::bad_lexical_cast const&) {
                throw Configuration_error("Invalid field in Simulation configuration.");
            }
        }

        void new_user_event() {
            ++remaining_user_events_;
        }

        void handled_user_event() {
            --remaining_user_events_;
        }

    protected:
        const event_t* top() const {
            return event_list_.top();
        }

    private:
        void handle_next_event(){
            static const std::string time_elapsed("time_elapsed");

            if (event_list_.empty()) {
                EOS_ = true;
                return;
            }

            event_t *event = event_list_.top();
            event_list_.pop();
            time_elapsed_ = event->get_handling_time();


            result_.record_value(time_elapsed,
                                 result_.get_general_key(),
                                 time_elapsed_);

            event->handle_event();
            event->automated_update_result(result_);
            event->post_handle();

            allocator_.destroy(event);
        }

        void init_run() {
            typedef std::pair<const node_key_t, std::reference_wrapper<const node_t>> node_key_obj;

            auto bounds = flow_controller_.get_entry_nodes();
            for(auto node_key = std::get<0>(bounds);
                node_key != std::get<1>(bounds);
                ++node_key) {
                add_event<Arrival_event<Event_manager<flow_controller_t, event_time_t, result_t>, node_key_obj>>(
                    std::make_pair(*node_key, std::cref(flow_controller_.get_topology().get_node_object(*node_key))));
            }

            result_.add_computed_value(false,
                                       std::string("Blocking_rate"),
                                       result_t::event_division,
                                       std::string("Flow_allocation_failure_event"),
                                       std::string("Arrival_event"),
                                       true);

            result_.register_convergence(std::string("Blocking_rate"),
                                         convergence_epsilon_,
                                         convergence_number_samples_);

            // TODO Macro list of etypes
            // or enum + __class__ specialazation
            // for(class__ in enum)
            //  class__.register_new_result(result_);
        }

        typedef FooAllocator Allocator;

        Allocator allocator_;
        event_queue event_list_;
        bool EOS_;
        size_t remaining_user_events_;
        result_t result_;
        flow_controller_t & flow_controller_;
        event_time_t time_elapsed_;
        Random_generator<event_time_t> rand_generator_;

        // Convergence Params
        size_t convergence_check_interval_;
        size_t convergence_number_samples_;
        typename result_t::value_t convergence_epsilon_;
};

#endif

#if TEST_EVENT

#include "node.hpp"

struct FooResult {
    typedef float result_value_t;
    void increase_value(std::string, Node<>) {
    }
    bool check_convergence(std::string const&) {
        return false;
    }
    std::string get_general_key() {
        return std::string();
    }
};

struct FooTopology {
    FooTopology() : node(.1, .2){
    }
    typedef size_t node_key_t;
    typedef Node<> node_t;
    typedef typename std::vector<node_key_t>::const_iterator citerator;
    node_key_t get_random_exit_node(size_t){
        return n;
    }
    Node<> const& get_node_object(node_key_t) {
        return node;
    }

    std::vector<node_key_t> topo;
    node_key_t n;
    Node<> node;
};

struct FooFlow{
    size_t length(){
        return 1;
    }
};

template <class Key_generator=Key_generator<size_t>>
struct FooFlowController {
    typedef size_t FooFlowKey;
    typedef FooFlowKey flow_key_t;
    typedef Key_generator key_generator_t;
    typedef FooTopology topology_t;
    FooFlowController(){
    }
    void free_flow(FooFlowKey) {
    }
    FooTopology &get_topology() {
        return footopo;
    }
    std::pair<FooTopology::citerator, FooTopology::citerator> get_entry_nodes() const {
        return std::make_pair(footopo.topo.cbegin(), footopo.topo.cend());
    }
    FooFlow & get_flow(FooFlowKey){
        return flow;
    }
    size_t allocate_flow(typename FooTopology::node_key_t, typename FooTopology::node_key_t) {
        return 1;
    }
    FooTopology footopo;
    FooFlow flow;

};


struct FooSimulation {
    typedef size_t flow_key_t;
    typedef Result<size_t> result_t;
    typedef FooFlowController<> flow_controller_t;
    typedef FooTopology topology_t;
};

int test_event() {
    typedef size_t FooFlowKey;
    typedef std::pair<typename FooTopology::node_key_t, std::reference_wrapper<Node<>>> pNode;
    FooFlowController<> ffc;
    Event_manager<FooFlowController<>> evt(ffc);
    Node<> object_node(0.1, 0.2);
    pNode node(std::make_pair(0, std::reference_wrapper<Node<>>(object_node)));
    FooFlowKey flow;
    Arrival_event<decltype(evt), pNode> b(evt, node);
    FTEST(b.get_handling_time() > 0);
    End_flow_event<decltype(evt), pNode> c(evt, node, 10, flow);
    FTEST(c.get_handling_time() == 10);
    End_of_simulation_event<decltype(evt), pNode> d(evt, node, 1);
    FTEST(d.get_handling_time() == 1);
    Flow_allocation_success_event<decltype(evt), pNode> e(evt, node, flow);
    FTEST(e.get_handling_time() == evt.get_time_elapsed());
    Flow_allocation_failure_event<decltype(evt), pNode> f(evt, node);
    FTEST(f.get_handling_time() == evt.get_time_elapsed());

    evt.add_event<decltype(c)>(node, 10, flow);
    FTEST(evt.top()->get_handling_time() == 10);
    evt.add_event<decltype(d)>(node, 1);
    FTEST(evt.top()->get_handling_time() == 1);
    evt.add_event<decltype(b)>(node);
    Arrival_event<decltype(evt), pNode> z(evt, node);
    // Arrival_event has a random handling time....
    FTEST(evt.top()->get_handling_time() > 0);
    evt.add_event<decltype(e)>(node, flow);
    FTEST(evt.top()->get_handling_time() == evt.get_time_elapsed());
    evt.add_event<decltype(f)>(node);
    FTEST(evt.top()->get_handling_time() == evt.get_time_elapsed());

    evt.start_event_processing();
    // Check with valgrind that if was free'd
    evt.add_event<decltype(b)>(node);


    return EXIT_SUCCESS;
}

int main() {
    return test_event();
}

#endif
