#ifndef __EDGEHPP__
#define __EDGEHPP__

#include <vector>
#include <string>
#include <cassert>
#include <limits>

#include "include.hpp"

template<class flow_key_t>
class Abstract_edge {
    public:
        enum {LAST_FLOW_AVAILABLE};
        virtual void allocate_flow(flow_key_t const&) = 0;
        virtual void free_flow(flow_key_t const&) = 0;
        virtual unsigned get_weight() const = 0;
};

template<class flow_key_t, class weight_t=float, class flow_container=std::vector>
class Edge : public Abstract_edge<flow_key_t> {
    private:
        size_t max_flows_;
        size_t available_flows_;
        weight_t weight_, former_weight_;
        flow_container<flow_key_t const&> passing_flows_;
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
        Edge() noexcept;
        Edge(Edge const&);
        Edge& operator=(Edge const&);
        Edge(size_t capacity = 1, weight_t weight = 1) : max_flows_(capacity),
                                 available_flows_(capacity),
                                 weight_(weight),
                                 former_weight_(infinite_weight_)
        {
            if(weight_ <= 0)
                throw Edge_allocation_error();
        }

        size_t allocate_flow(flow_key_t const& flow) {
            if (available_flows_ == 0)
                throw Edge_allocation_error();

            passing_flows_.push_back(flow);
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

        weight_t  get_weight() const {
            return weight_;
        }
};

#endif
