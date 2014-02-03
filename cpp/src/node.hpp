#ifndef __NODEHPP__
#define __NODEHPP__

#include <iostream>
#include <string.h>

#include "include.hpp"
#include "exception.hpp"

class Abstract_node {
    public:
        virtual operator int() = 0;
        virtual std::string const& get_name() const = 0;
        virtual float get_arrival_rate() const = 0;
        virtual float get_service_rate() const = 0;
};

class Node : public Abstract_node {
    // TODO initialize somewhere
    static unsigned long counter;

    private:
        long number;
        float arrival_rate;
        float service_rate;
        std::string name;

    public:
        Node() noexcept;
        Node(Node const&);
        Node& operator=(Node const&);
        Node(float arrival_rate, float service_rate, std::string const& name = "") : arrival_rate(arrival_rate), service_rate(service_rate), name(name)
        {
            if(arrival_rate < 0 or service_rate < 0)
                throw Wrong_parameter();
            number = ++counter;
        }

        operator int() const {
            return number;
        }

        std::string const& get_name() const {
            return name;
        }

        float get_arrival_rate() const {
            return arrival_rate;
        }

        float get_service_rate() const {
            return service_rate;
        }
};

#endif
