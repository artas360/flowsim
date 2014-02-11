#ifndef __EDGEHPP__
#define __EDGEHPP__

#include <list>
#include <limits>
#include <functional>

#include "include.hpp"

template<class flow_key_t, class weight_t, class flow_container_t>
class Abstract_edge {
    public:
        enum {LAST_FLOW_AVAILABLE};
        virtual size_t allocate_flow(flow_key_t const&) = 0;
        virtual void free_flow(flow_key_t const&) = 0;
        virtual weight_t get_weight() const = 0;
};

template<class flow_key_t, class weight_t=float, class flow_container_t=std::list<std::reference_wrapper<const flow_key_t>>>
class Edge : public Abstract_edge<flow_key_t, weight_t, flow_container_t> {
    private:
        size_t max_flows_;
        size_t available_flows_;
        weight_t weight_, former_weight_;
        flow_container_t passing_flows_;
        const weight_t infinite_weight_ = std::numeric_limits<weight_t>::infinity();

        void switch_weight() {
            if (weight_ == infinite_weight_) {
                weight_ = former_weight_;
            }
            else {
                former_weight_ = weight_;
                weight_ = infinite_weight_;
            }
        }

    public:
        Edge(Edge<flow_key_t, weight_t, flow_container_t> const& edge) {
            *this = edge;
        }

        Edge(size_t capacity = 1, weight_t weight = 1) : max_flows_(capacity),
                                                         available_flows_(capacity),
                                                         weight_(weight),
                                                         former_weight_(infinite_weight_),
                                                         passing_flows_() {
            if(max_flows_ == 0 or weight_ <= 0.)
                throw Edge_allocation_error();
        }

        Edge& operator=(Edge const& other) {
            if(this != &other) {
                max_flows_ = other.max_flows_;
                available_flows_ = other.available_flows_;
                weight_ = other.weight_;
                former_weight_ = other.former_weight_;
                passing_flows_ = other.passing_flows_;
            }

            return *this;
        }

        size_t allocate_flow(flow_key_t const& flow) {
            if (available_flows_ == 0)
                throw Edge_allocation_error();

            passing_flows_.emplace_back(flow);
            if (available_flows_ == 1)
                switch_weight();

            return --available_flows_;
        }

        void free_flow(flow_key_t const& flow) {
            assert(available_flows_ < max_flows_);
            if (weight_ == infinite_weight_)
                switch_weight();
            passing_flows_.remove(flow);
            ++available_flows_;
        }

        weight_t get_weight() const {
            return weight_;
        }

        flow_container_t const& get_flows() const {
            return passing_flows_;
        }
};

#endif

#if TEST

#include <iostream>

int test_edge() {
    Edge<float> a, b(2, 3);

    b.allocate_flow(2);
    FTEST((b.get_flows().cbegin())->get() == 2);
    
    Edge<float> c(b);
    FTEST((c.get_flows().cbegin())->get() == 2);

    b.free_flow(2);
    FTEST(b.get_flows().empty());
    FTEST((c.get_flows().cbegin())->get() == 2);

    FTEST(c.get_weight() == 3 and b.get_weight() == 3);

    ASSERT_RAISES(Edge<float>, 0, 1);

    b.allocate_flow(1);
    b.allocate_flow(2);
    ASSERT_RAISES(b.allocate_flow, 3);
    ASSERT_NOT_RAISES(b.free_flow, 5);

    return EXIT_SUCCESS;
}

int main() {
    return test_edge();
}

#endif
