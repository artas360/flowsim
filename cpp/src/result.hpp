#ifndef __RESULT_HPP__
#define __RESULT_HPP__

#include <cmath>
#include <functional>
#include <limits>
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <valarray>

#include "include.hpp"


template<typename sample_value_t>
class sample_container {
    public:
        sample_container(size_t number_samples, sample_value_t epsilon) : counter_(0),
                                                                          epsilon_(epsilon),
                                                                          samples_(infinity_, number_samples) {}

        void update_samples(sample_value_t const& sample) {
            samples_[counter_++] = sample;
            counter_ %= samples_.size();
        }

        sample_value_t standard_deviation() {
            // probably not optimal but oneliner!
            return sqrt((std::pow(samples_, (float)2).sum() / samples_.size()) - pow(samples_.sum() / samples_.size(), 2));
        }

        bool has_converged() {
            assert(epsilon_ > 0);
            return standard_deviation() < epsilon_;
        }

    private:
        size_t counter_;
        sample_value_t epsilon_;
        const sample_value_t infinity_ = std::numeric_limits<sample_value_t>::has_quiet_NaN ?
                                         std::numeric_limits<sample_value_t>::quiet_NaN() :
                                         std::numeric_limits<sample_value_t>::max();
        std::valarray<sample_value_t> samples_;
};


// result_value_t should be default constructed at 0
template <class result_key_t,
          typename result_value_t=float,
          typename _time_t=float,
          class result_container_t=std::unordered_map<result_key_t, std::unordered_map<std::string, result_value_t>>,
          class sample_container_t=sample_container<result_value_t>>
class Result {

    public:

        typedef result_value_t value_t;
        typedef typename result_container_t::mapped_type submap_t;
        typedef typename std::function<result_value_t(submap_t const&, result_value_t const&, std::string, std::string)> compute_function_t;
        typedef typename std::function<result_value_t(submap_t const&, result_value_t const&)> binded_compute_function_t;
        typedef std::unordered_map<std::string, std::tuple<bool, binded_compute_function_t, bool>> function_map_t;
        typedef std::unordered_map<std::string, sample_container_t> convergence_map_t;
        typedef std::pair<_time_t, result_value_t> user_sample_t;
        typedef std::unordered_map<std::string, std::vector<user_sample_t>> user_sample_container_t;

        Result() : results_() {
        }

        void increase_value(std::string const& value_name, result_key_t const& key, result_value_t const& increment = 1) {
            results_[key][value_name] += increment;
        }

        void record_value(std::string const& value_name, result_key_t const& key, result_value_t const& value) {
            results_[key][value_name] = value;
        }

        void add_computed_value(bool is_node_value, std::string const& value_key, compute_function_t compute_function, std::string const& key1, std::string const& key2, bool update_on_get=false) {
            auto f = std::bind(compute_function, std::placeholders::_1, std::placeholders::_2, key1, key2);
            function_map_.emplace(value_key, std::make_tuple(is_node_value, f, update_on_get));
            if(not is_node_value)
                update_computed_value(value_key, general_key_, 0);
            else{
                for(auto it: results_)
                    update_computed_value(value_key, std::get<0>(it), 0);
            }
        }

        void update_computed_value(std::string const& value_key, result_key_t const& result_key, result_value_t const& new_element) {
            results_[result_key][value_key] = std::get<1>(function_map_[value_key])(results_[result_key], new_element);
        }

        result_value_t const& get_computed_value(result_key_t const& result_key, std::string const& value_key) {
            if(std::get<2>(function_map_[value_key]))
                return (results_[result_key][value_key] = std::get<1>(function_map_[value_key])(results_[result_key], 0));
            else
                return results_[result_key][value_key];
        }

        void register_convergence(std::string const& value_key, float epsilon, size_t number_samples) {
#ifdef __GNUC__
            convergence_map_.emplace(value_key, sample_container_t(number_samples, epsilon));
#else
            convergence_map_.emplace(value_key, number_samples, epsilon);
#endif // __GNUC__
        }

        bool check_convergence(std::string const& value_key) {
            return check_convergence(value_key, this->get(general_key_, value_key));
        }

        bool check_convergence(std::string const& value_key, result_value_t const& new_sample) {
            try {
                convergence_map_.at(value_key).update_samples(new_sample);
                return convergence_map_.at(value_key).has_converged();
            } catch(std::out_of_range e) {
                throw Not_registered_value();
            }
        }

        // TODO ? container template
        result_value_t process_nodes_value(submap_t const&, result_value_t const&, std::string const& value_name, std::string const&, std::function<result_value_t(std::valarray<result_value_t>&)> process_function) {
            size_t i = results_.size() - 1;
            std::valarray<result_value_t> values(i);
            for(auto it: results_) {
                if (std::get<0>(it) != general_key_)
                    values[--i] = get(std::get<0>(it), value_name);
            }
            assert(i == 0);
            return process_function(values);
        }

        inline result_key_t const& get_general_key() const {
            return general_key_;
        }

        result_value_t const& get(result_key_t const& result_key, std::string const& value_name) {
            // Will emplace new element if wrong key input
            if(function_map_.find(value_name) != function_map_.cend())
                return get_computed_value(result_key, value_name);
            else
                return results_[result_key][value_name];
        }

        void record_value(std::string const& value_name, _time_t const& time) {
            user_sample_container_[value_name].emplace_back(time, get(general_key_, value_name));
        }

        std::ostream& stream_samples(std::ostream &out, std::string const& value_name) const {
            typename user_sample_container_t::const_iterator it(user_sample_container_.find(value_name));

            if(it != user_sample_container_.cend()) {
                for(auto it2: std::get<1>(*it))
                    if(std::isfinite(it2.second))
                        out << it2.first << ' ' << it2.second << ' ';
            }
            return out;
        }

        // Can't be const because get can reevaluate a value
        std::ostream& stream_results(std::ostream &out) {
            for(auto it: results_) {
                std::cout << "Holder " << std::get<0>(it) << std::endl;
                for(auto it2: std::get<1>(it))
                    out << "\t" << std::setw(40) << std::setiosflags(std::ios::left) <<
                    std::get<0>(it2)  << " "<< get(std::get<0>(it), std::get<0>(it2)) << std::endl;
            }
            return out;
        }

        static result_value_t event_division(submap_t const& submap, result_value_t const&, std::string const& numerator_key, std::string const& denominator_key) {
            try {
                assert(submap.at(denominator_key) != 0);
                return submap.at(numerator_key) / submap.at(denominator_key);
            } catch (std::out_of_range) {
                assert(std::numeric_limits<result_value_t>::has_quiet_NaN);
                return std::numeric_limits<result_value_t>::quiet_NaN();
            }
        }

        static result_value_t update_mean(submap_t const& submap, result_value_t const& new_element, std::string const& mean_key, std::string const& denominator_key) {
            // Loss in precision ...
            try {
                result_key_t number_values = submap.at(denominator_key);
                return ((submap.at(mean_key) * (number_values - 1)) + new_element) / number_values;
            } catch(std::out_of_range){
                return new_element;
            }
        }

    private:
        result_container_t results_;
        function_map_t function_map_;
        convergence_map_t convergence_map_;
        user_sample_container_t user_sample_container_;
        const result_key_t general_key_ = std::numeric_limits<result_key_t>::has_quiet_NaN ?
                                            std::numeric_limits<result_key_t>::quiet_NaN() :
                                            std::numeric_limits<result_key_t>::max();
};

template <class result_key_t, typename result_value_t, class result_container_t>
std::ostream& operator<<(std::ostream &out, Result<result_key_t, result_value_t, result_container_t> & results) {
    return results.stream_results(out);
}

#if TEST_RESULT

float mean(std::valarray<float>& values) {
    return values.sum() / values.size();
}

int test_result() {
    Result<int> result;
    auto gen_key = result.get_general_key();
    result.increase_value("counter", gen_key);
    result.increase_value("counter", 1);
    result.increase_value("counter", 2);
    result.increase_value("machin", gen_key, 1);
    result.increase_value("machin", gen_key, 2.);

    result.add_computed_value(true, std::string("mean"), Result<int>::update_mean, std::string("mean"), std::string("counter"));
    typename Result<int>::compute_function_t fct = std::bind(&Result<int>::process_nodes_value, &result, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, mean);
    result.add_computed_value(false, std::string("Block"), fct, std::string("mean"), std::string(), true);
    result.update_computed_value(std::string("mean"), 1, 1);
    result.register_convergence(std::string("Block"), .03, 6);
    result.check_convergence(std::string("Block"));
    std::cout << "printing results" << std::endl;
    std::cout << result << std::endl;
    result.increase_value("counter", 1);
    result.update_computed_value(std::string("mean"), 1, 4);

    std::cout << "printing results" << std::endl;
    std::cout << result << std::endl;
    return EXIT_SUCCESS;
}

int main() {
    return test_result();
}

#endif

#endif
