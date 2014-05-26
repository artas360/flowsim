#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <string>
#include <limits>

#include "include.hpp"

template <typename rate_t, typename name_t, typename id_t>
class Abstract_node {
    public:
        virtual operator id_t() const = 0;
        virtual id_t const& get_number() const = 0;
        virtual name_t const& get_name() const = 0;
        virtual rate_t const& get_arrival_rate() const = 0;
        virtual rate_t const& get_service_rate() const = 0;
};

template <typename _rate_t=float, typename _name_t=std::string, typename _id_t=size_t>
class Node : public Abstract_node<_rate_t, _name_t, _id_t> {

    public:
        typedef _rate_t rate_t;
        typedef _name_t name_t;
        typedef _id_t id_t;

    private:
        static id_t counter_;
        id_t number_;
        rate_t arrival_rate_;
        rate_t service_rate_;
        name_t name_;

    public:
        Node() noexcept {} // Should not increase counter because of graph instanciation

        Node(Node const& other) noexcept {
            *this = other;
        }

        Node(rate_t const& arrival_rate, rate_t const& service_rate, name_t const& name = name_t()) : arrival_rate_(arrival_rate),
                                                                                                      service_rate_(service_rate),
                                                                                                      name_(name) {
            if(arrival_rate_ < 0 or service_rate_ < 0)
                throw Wrong_parameter();
            number_ = ++counter_;
        }

        Node& operator=(Node const& other) {
            number_ = other.number_;
            arrival_rate_ = other.arrival_rate_;
            service_rate_ = other.service_rate_;
            name_ = other.name_;
            return *this;
        }

        void swap_arr_rate(rate_t new_rate) {
            assert(new_rate > 0);
            arrival_rate_ = new_rate;
        }

        operator id_t() const {
            return number_;
        }

        id_t const& get_number() const {
            return number_;
        }

        name_t const& get_name() const {
            return name_;
        }

        rate_t const& get_arrival_rate() const {
            return arrival_rate_;
        }

        rate_t const& get_service_rate() const {
            return service_rate_;
        }
};

#endif
