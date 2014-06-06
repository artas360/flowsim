#ifndef __RANDOM_GENERATOR_HPP__
#define __RANDOM_GENERATOR_HPP__

#include <random>
#include <chrono>
#include <limits>

#include "include.hpp"

/**
 * \class Random_generator
 * \brief Random generator to be used throught the simulation.
 *
 * One unique instance of this class should be the only 
 * entity calling random functions during one simulation.
 * This will ensure that, by providing the the random seed,
 * the results can be reproduced.
 */
template <typename result_type,
          typename rate_type=result_type,
          typename arrival_distribution = std::exponential_distribution<result_type>,
          typename duration_distribution = std::exponential_distribution<result_type>,
          typename number_generator=std::uniform_int_distribution<size_t>,
          typename generator=std::default_random_engine>
class Random_generator {
    public:
        Random_generator(unsigned seed = std::chrono::system_clock::now().time_since_epoch().count()) :
            generator_(seed) {
        }

        /**
         * \brief Generates a random value based on arrival_distribution
         * with arrival_rate parameter.
         * \param arrival_rate Value to be passed to the arrival_distribution.
         * \return Random value based on arrival_distribution
         * with arrival_rate parameter.
         */
        result_type next_arrival(rate_type arrival_rate) {
            return arrival_distribution(arrival_rate)(generator_);
        }

        /**
         * \brief Generates a random value based on duration_distribution
         * with service_rate parameter.
         * \param arrival_rate Value to be passed to the duration_distribution.
         * \return Random value based on duration_distribution
         * with service_rate parameter.
         */
        result_type rand_duration(rate_type service_rate) {
            return duration_distribution(service_rate)(generator_);
        }

        /**
         * \brief Generates a random integer based on number_generator.
         * The result is between the specified bounds.
         * \param lowest Lowest value the function can return.
         * \param highest Highest value the function can return.
         * \return Random value in [lowest, highest]
         */
        size_t rand_int(size_t lowest = 0, size_t highest = std::numeric_limits<size_t>::max()) {
            return number_generator(lowest, highest)(generator_);
        }

    private:
        generator generator_;
};

#endif
