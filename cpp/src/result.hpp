#ifndef __RESULT_HPP__
#define __RESULT_HPP__

#include <string>
#include <functional>
#include "include.hpp"

#include <unordered_map>

// result_value_t should be default constructed at 0
template <class Topology, typename result_value_t=float, class Node_result=std::unordered_map<std::string, result_value_t>>
class Result {
    typedef typename Topology::node_key_t node_key_t;
    typedef std::unordered_map<node_key_t, Node_result> node_results_t;
    typedef std::unordered_map<std::string, result_value_t> general_results_t;
    typedef std::unordered_map<std::string, std::pair<std::reference_wrapper<node_key_t>, std::function>> function_map_t;

    public:
        Result() : node_results_(), general_results_() {
        }

        void init_node_data(node_key_t const& node_key) {
            // emplace() checks if key already exits
            node_results_.emplace(node_key);
        }

        void increase_value(std::string const& value_name, node_key_t const& node_key, result_value_t const& increment = 1) {
            node_results_[node_key][value_name] += increment;
        }

        void increase_value(std::string const& value_name, result_value_t const& increment = 1) {
            general_results_[value_name] += increment;
        }

        template<typename... Args>
        void add_computed_value(bool is_node_value, std::string value_key, std::function compute_function, Args... args) {
            std::pair<std::forward_iterator<result_value_t>, std::forward_iterator<result_value_t>> targets;
            tragets = is_node_value ? std::make_pair(node_results_.begin(), node_results_.end()) :
                                      std::make_pair(general_results_.begin(), general_results_.end());

            function_map_[value_key] = make_pair(target, std::bind(compute_function, std::placeolder _1, args...));
            // Could use reference wraper on one of the args (and change where it points)
            // or pass another bind() as arg!
        }

        result_value_t const& mean(std::reference_wrapper<node_key_t> node_key_ref) {
        }

    private:
        node_results_t node_results_;
        general_results_t general_results_;
        function_map_t function_map_;
};

#endif
