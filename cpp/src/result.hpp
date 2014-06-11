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
#include <vector>

#include "include.hpp"


/**
 * \class sample_container
 * \brief Stores and check convergence of samples.
 */
template<typename sample_value_t>
class sample_container {
    public:
        sample_container(size_t number_samples, sample_value_t epsilon) : counter_(0),
                                                                          epsilon_(epsilon),
                                                                          samples_(infinity_, number_samples) {}

        /**
         * \brief Adds a sample to the container by replacing
         * the oldest one.
         * \param New sample.
         */
        void update_samples(sample_value_t const& sample) {
            samples_[counter_++] = sample;
            counter_ %= samples_.size();
        }

        /**
         * \brief Computes the standard deviation of the set of
         * samples.
         * \return Standard deviation of the set of samples.
         */
        sample_value_t standard_deviation() {
            // probably not optimal but oneliner!
            return sqrt((std::pow(samples_, (float)2).sum() / samples_.size()) - pow(samples_.sum() / samples_.size(), 2));
        }

        /**
         * \brief Checks if the samples standard deviation is
         * bellow the sample_container epsilon.
         * \return True if standard_deviation() < epsilon else false.
         */
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


/**
 * \class Result
 * \brief Provides mechanisms to store, update and output results.
 *
 * \warning result_value_t should be default constructed at 0.
 *
 * The data structure containing the results is:<BR>
 * <pre>
 * Result -- holder_key1 -- value_name1
 *        |              |- ...
 *        |              |- value_nameX
 *        |- ...
 *        |- general_key -- value_name1
 *                       |- ...
 *                       |- value_nameX
 *                       |- key_specific_result
 * </pre>
 */
template <class holder_key_t,
          typename result_value_t=float,
          typename _time_t=float,
          typename value_name_t=std::string,
          class result_container_t=std::unordered_map<holder_key_t, std::unordered_map<value_name_t, result_value_t>>,
          class sample_container_t=sample_container<result_value_t>>
class Result {

    public:
        typedef result_value_t value_t;
        typedef value_name_t name_t;
        typedef typename result_container_t::mapped_type submap_t;
        typedef typename std::function<result_value_t(submap_t const&, result_value_t const&, value_name_t const&, value_name_t const&)> compute_function_t;
        typedef typename std::function<result_value_t(submap_t const&, result_value_t const&)> binded_compute_function_t;
        typedef std::unordered_map<value_name_t, std::tuple<bool, binded_compute_function_t, bool>> function_map_t;
        typedef std::unordered_map<value_name_t, sample_container_t> convergence_map_t;
        typedef std::pair<_time_t, result_value_t> user_sample_t;
        typedef std::unordered_map<value_name_t, std::vector<user_sample_t>> user_sample_container_t;

        Result() : results_() {
        }

        /**
         * \brief Increments the value of the specified element.
         * Creates (and default constructs) the element if necessary.
         * \param value_name Name of the value to be updated.
         * \param holder_key Key of the holder of the value to be updated.
         * \param increment Increment to be used.
         */
        void increase_value(value_name_t const& value_name,
                            holder_key_t const& holder_key,
                            result_value_t const& increment = 1) {
            results_[holder_key][value_name] += increment;
        }

        /**
         * \brief Records the value of the specified element.
         * Creates (and default constructs) the element if necessary.
         * \param value_name Name of the value to be updated.
         * \param holder_key Key of the holder of the value to be updated.
         * \param value value to be recorded.
         */
        void record_value(value_name_t const& value_name,
                          holder_key_t const& holder_key,
                          result_value_t const& value) {
            results_[holder_key][value_name] = value;
        }

        /**
         * \brief Snapshots the value of value_name held by general_key_.
         *
         * Registers the value of value_name held by general_key_
         * in a separated container that is not modified by any other
         * function. This record is associated with the value of time.
         * This is used for time sampling of values.
         *
         * \param value_name Name of the value to be snapshot.
         * \param time Value to be associated with the snapshot.
         * \todo Allow snapshots of value not held by general_key.
         */
        void snapshot_value(value_name_t const& value_name, _time_t const& time) {
            user_sample_container_[value_name].emplace_back(time, get(general_key_, value_name));
        }

        /**
         * \brief Registers a handle associated to a particular value.
         * \param is_node_value Indicates wether this new computed value
         * belong to all key != from general key or belongs to general_key only.
         * \param value_name Name of the value to be updated.
         * \param compute_function Handle function to be called to update the value.
         * The signature of the function must be the same as compute_function_t.
         * \param key1 Key to be passed to the function.
         * \param key2 Key to be passed to the function.
         * \param update_on_get Indicates whether the value must be updated with the
         * handle every time the value is read.
         *
         * The function will store the parameters given so that calls
         * to update_computed_value will require few arguments. The way to handle
         * the value and the keys key1 and key2 being stored in the class.
         */
        void add_computed_value(bool is_node_value,
                                value_name_t const& value_name,
                                compute_function_t compute_function,
                                value_name_t const& key1,
                                value_name_t const& key2,
                                bool update_on_get=false) {
            auto f = std::bind(compute_function, std::placeholders::_1, std::placeholders::_2, key1, key2);
            function_map_.emplace(value_name, std::make_tuple(is_node_value, f, update_on_get));
            if(not is_node_value)
                update_computed_value(value_name, general_key_, 0);
            else{
                for(auto it: results_)
                    update_computed_value(value_name, std::get<0>(it), 0);
            }
        }

        /**
         * \brief Registers that a value can be sampled and checked
         * for convergence. The convergence logic is handled by the
         * class sample_container.
         * \param value_name Name of the value to watched for
         * convergence. Must be owned by general_key.
         * \param epsilon Value used to decided if the convergence
         * has occurred. See sample_container for details.
         * \param number_samples Number of samples on which the
         * convergence condition is evealuated. See sample_container
         * for details.
         * \todo allow for values not belonging to general_key_ to
         * be registered.
         */
        void register_convergence(value_name_t const& value_name,
                                  float epsilon,
                                  size_t number_samples) {
#ifdef __GNUC__
            convergence_map_.emplace(value_name, sample_container_t(number_samples, epsilon));
#else
            convergence_map_.emplace(value_name, number_samples, epsilon);
#endif // __GNUC__
        }

        /**
         * \brief Checks if the given value has converged. Will
         * force a sample of the value to be updated with the 
         * current value.
         * \param value_name Name of the value to be checked for
         * convergence.
         * \return True if the value has converged, else false.
         */
        bool check_convergence(value_name_t const& value_name) {
            return check_convergence(value_name, this->get(general_key_, value_name));
        }

        /**
         * \brief Checks if the given value has converged. Will
         * force a sample of the value to be updated with the 
         * given sample.
         * \param value_name Name of the value to be checked for
         * convergence.
         * \return True if the value has converged, else false.
         * \Warning Throws if value_name has not been registered
         * for convergence!
         */
        bool check_convergence(value_name_t const& value_name, result_value_t const& new_sample) {
            try {
                convergence_map_.at(value_name).update_samples(new_sample);
                return convergence_map_.at(value_name).has_converged();
            } catch(std::out_of_range e) {
                throw Not_registered_value();
            }
        }

        /**
         * \brief Applies the given function to a container
         * holding the value of value_name for all keys except
         * general_key.
         *
         * A lot of the parameters are unnamed. They are provided
         * for compliance with the computed values handle
         * prototype.
         *
         * \param value_name Name of the value to apply the
         * function on.
         * \param process_function Function that will be called
         * on the container of values retrieved.
         * \return Result of the process_function.
         */
        template <template<class> class _container_t=std::vector>
        result_value_t process_nodes_value(submap_t const&,
                                           result_value_t const&,
                                           value_name_t const& value_name,
                                           value_name_t const&,
                                           std::function<result_value_t(_container_t<result_value_t>&)> process_function) {
            std::vector<result_value_t> values;
            values.reserve(results_.size() - 1);
            for(auto it: results_) {
                if (std::get<0>(it) != general_key_)
                    values.emplace_back(get(std::get<0>(it), value_name));
            }
            assert(values.size() == results_.size() - 1);
            return process_function(values);
        }

        /**
         * \return Value to refer to the general key.
         */
        constexpr holder_key_t const& get_general_key() const {
            return general_key_;
        }

        /**
         * \brief Returns the value of value_name held by key.
         *
         * Will trigger re-evaluation of the value if value_name
         * refers to a computed value with update_on_get set to
         * true.
         *
         * \param value_name Name of the value to be read.
         * \param holder_key Key of the holder of the value to be updated.
         * \return Value of value_name held by key.
         */
        result_value_t const& get(holder_key_t const& holder_key, value_name_t const& value_name) {
            // Will emplace new element if wrong key input
            if(function_map_.find(value_name) != function_map_.cend())
                return get_computed_value(holder_key, value_name);
            else
                return results_[holder_key][value_name];
        }

        /**
         * \brief Outputs the snapshots of value_name into the stream.
         * \param out Stream to output the snpashots into.
         * \param value_name Name of the value to be outputed.
         *
         * The output will be:<BR>
         * time1 value1 time2 value2 ...
         */
        std::ostream& stream_samples(std::ostream &out, value_name_t const& value_name) const {
            typename user_sample_container_t::const_iterator it(user_sample_container_.find(value_name));

            if(it != user_sample_container_.cend()) {
                for(auto it2: std::get<1>(*it))
                    if(std::isfinite(it2.second))
                        out << it2.first << ' ' << it2.second << ' ';
            }
            return out;
        }

        /**
         * \brief Outputs the contents of the all the values of every
         * holder.
         * \param out Stream to output the results into.
         *
         * The output will be:<BR>
         * <pre>
         * Holder X:
         *      value_name1 value1
         *      value_name2 value2
         *      ...
         * Holder Y:
         *      value_name1 value1
         *      ...
         * ...
         * </pre>
         *
         * \note Can't be declared const because get has side effects.
         */
        std::ostream& stream_results(std::ostream &out) {
            for(auto it: results_) {
                std::cout << "Holder " << std::get<0>(it) << std::endl;
                for(auto it2: std::get<1>(it))
                    out << "\t" << std::setw(40) << std::setiosflags(std::ios::left) <<
                    std::get<0>(it2)  << " "<< get(std::get<0>(it), std::get<0>(it2)) << std::endl;
            }
            return out;
        }

        /**
         * \brief Simple division function to be used as a
         * computed value handle.
         * \return Value of numerator_key divided by
         * the value of denominator_key
         */
        static result_value_t event_division(submap_t const& submap,
                                             result_value_t const&,
                                             value_name_t const& numerator_key,
                                             value_name_t const& denominator_key) {
            static_assert(std::numeric_limits<result_value_t>::has_quiet_NaN,
                          "Result::result_value_t doesn't have a quiet NaN");
            try {
                assert(submap.at(denominator_key) != 0);
                return submap.at(numerator_key) / submap.at(denominator_key);
            } catch (std::out_of_range) {
                return std::numeric_limits<result_value_t>::quiet_NaN();
            }
        }

        /**
         * \brief Function used to update a mean.
         * Function to be used as a computed value handle.
         * \return Updated value of the mean.
         * \warning The way the mean is updated will decrease
         * the precision of result. The more the function is
         * called the less accurate the result.
         */
        static result_value_t update_mean(submap_t const& submap,
                                          result_value_t const& new_element,
                                          value_name_t const& mean_key,
                                          value_name_t const& denominator_key) {
            try {
                holder_key_t number_values = submap.at(denominator_key);
                return ((submap.at(mean_key) * (number_values - 1)) + new_element) / number_values;
            } catch(std::out_of_range){
                return new_element;
            }
        }

    private:
        void update_computed_value(value_name_t const& value_key,
                                   holder_key_t const& holder_key,
                                   result_value_t const& new_element) {
            results_[holder_key][value_key] =
                std::get<1>(function_map_[value_key])(results_[holder_key], new_element);
        }

        result_value_t const& get_computed_value(holder_key_t const& holder_key,
                                                 value_name_t const& value_key) {
            if(std::get<2>(function_map_[value_key]))
                return (results_[holder_key][value_key]
                    = std::get<1>(function_map_[value_key])(results_[holder_key], 0));
            else
                return results_[holder_key][value_key];
        }

        result_container_t results_;
        function_map_t function_map_;
        convergence_map_t convergence_map_;
        user_sample_container_t user_sample_container_;
        const holder_key_t general_key_ = std::numeric_limits<holder_key_t>::has_quiet_NaN ?
                                          std::numeric_limits<holder_key_t>::quiet_NaN() :
                                          std::numeric_limits<holder_key_t>::max();
};

template <class holder_key_t, typename result_value_t, class result_container_t, class value_name_t>
std::ostream& operator<<(std::ostream &out, Result<holder_key_t, result_value_t, result_container_t, value_name_t> & results) {
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
