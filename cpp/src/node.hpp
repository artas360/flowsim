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


/**
 * \class Node
 * \brief Class representing a node.
 */
template <typename _rate_t=float, typename _name_t=std::string, typename _id_t=size_t>
class Node : public Abstract_node<_rate_t, _name_t, _id_t> {

    public:
        typedef _rate_t rate_t;
        typedef _name_t name_t;
        typedef _id_t id_t;

        Node() noexcept {} // Should not increase counter

        Node(Node const& other) : number_(other.number_),
                                  arrival_rate_(other.arrival_rate_),
                                  service_rate_(other.service_rate_),
                                  name_(other.name_) {
        }

        Node(Node&& other) 
                noexcept(noexcept(other.swap(other))) {
            swap(other);
        }

        Node(rate_t const& arrival_rate, rate_t const& service_rate, name_t const& name = name_t()) : arrival_rate_(arrival_rate),
                                                                                                      service_rate_(service_rate),
                                                                                                      name_(name) {
            if(arrival_rate_ < 0 or service_rate_ < 0)
                throw Wrong_parameter();
            number_ = ++counter_;
        }

        Node& operator=(Node other)
                noexcept(noexcept(other.swap(other))) { // No fail!! because pass by copy!
            swap(other);
            return *this;
        }

        void swap(Node& other) 
                noexcept(noexcept(std::swap(std::declval<rate_t &>(),
                                            std::declval<rate_t &>())) &&
                         noexcept(std::swap(std::declval<id_t &>(),
                                            std::declval<id_t &>())) &&
                         noexcept(std::swap(std::declval<name_t &>(),
                                            std::declval<name_t &>()))) {
            using std::swap;

            swap(name_, other.name_);
            swap(number_, other.number_);
            swap(arrival_rate_, other.arrival_rate_);
            swap(service_rate_, other.service_rate_);
        }

        /**
         * \brief Changes the value of arrival_rate to new_rate
         * \param Arrival rate to be set.
         */
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

    private:
        static id_t counter_;
        id_t number_;
        rate_t arrival_rate_;
        rate_t service_rate_;
        name_t name_;
};

template <typename rate_t, typename name_t, typename id_t>
void swap(Node<rate_t, name_t, id_t>& first, Node<rate_t, name_t, id_t>& second) noexcept {
    first.swap(second);
}

#endif
