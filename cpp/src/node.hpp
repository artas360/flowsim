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
    // TODO initialize somewhere
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

#if TEST

template<>
size_t Node<>::counter_ = 0;

#endif

#if TEST_NODE

int test_node() {
    Node<> a, b(.1, .2, "bolt"), c(b), d(.3, .4);

    FTEST(b.get_name() == "bolt" and c.get_name() == "bolt");
    FTEST(b.get_number() == 1 and c.get_number() == 1);
    FTEST((b.get_arrival_rate() - 0.1) < std::numeric_limits<float>::epsilon() and
          (c.get_arrival_rate() - 0.1) < std::numeric_limits<float>::epsilon());
    FTEST((b.get_service_rate() - 0.2) < std::numeric_limits<float>::epsilon() and
          (c.get_service_rate() - 0.2) < std::numeric_limits<float>::epsilon());
    FTEST(d.get_number() == 2);
    ASSERT_RAISES(Node<>, -1, 0);

    return EXIT_SUCCESS;
}

int main() {
    return test_node();
}

#endif
