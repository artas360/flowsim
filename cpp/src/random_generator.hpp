#ifndef __RANDOM_GENERATOR_HPP__
#define __RANDOM_GENERATOR_HPP__

#include <random>
#include <chrono>
#include <limits>
#include "include.hpp"

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

        result_type next_arrival(rate_type arrival_rate) {
            return arrival_distribution(arrival_rate)(generator_);
        }

        result_type rand_duration(rate_type service_rate) {
            return duration_distribution(service_rate)(generator_);
        }

        size_t rand_int(size_t lowest = 0, size_t highest = std::numeric_limits<size_t>::max()) {
            return number_generator(lowest, highest)(generator_);
        }

        // In event_manager
        //node_key const& random_exit_node(node_key const&/* different_from*/) {
        //}

    private:
        generator generator_;
};

#if TEST_RANDOM_GENERATOR

int test_random_generator() {
    Random_generator<float> rand(2);
    for(int i = 0; i < 10; ++i) {
        std::cout << rand.next_arrival(.1) << std::endl; 
        std::cout << rand.next_arrival(.4) << std::endl; 
    }
    return EXIT_SUCCESS;
}

int main() {
    return test_random_generator();
}

#endif

#endif
