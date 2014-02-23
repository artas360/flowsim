#ifndef __RANDOM_GENERATOR_HPP__
#define __RANDOM_GENERATOR_HPP__

#include <random>
#include <chrono>
#include "include.hpp"

template <class Topology,
          typename result_type,
          typename rate_type=result_type,
          typename arrival_distribution = std::exponential_distribution<result_type>,
          typename duration_distribution = std::exponential_distribution<result_type>,
          typename number_generator=std::uniform_int_distribution<size_t>,
          typename generator=std::default_random_engine>
class Random_generator {
    public:
        typedef typename Topology::node_key node_key;
        Random_generator(Topology topology, rate_type arrival_rate, rate_type service_rate, unsigned seed = std::chrono::system_clock::now().time_since_epoch().count()) :
            generator_(seed),
            arrival_distribution_(arrival_rate),
            duration_distribution(service_rate),
            number_generator_() {
                topology;
        }

        result_type next_arrival() {
            return arrival_distribution_(generator_);
        }

        result_type rand_duration() {
            return duration_distribution_(generator_);
        }

        size_t rand_int() {
            return number_generator_(generator_);
        }

        std::pair<node_key const&, node_key const&> random_io_nodes() {
        }

        node_key const& random_exit_node(node_key const&/* different_from*/) {
        }
    private:
        generator generator_;
        arrival_distribution arrival_distribution_;
        duration_distribution duration_distribution_;
        number_generator number_generator_;
};


#endif
