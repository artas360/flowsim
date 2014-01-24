#ifndef __EDGEH__
#define __EDGEH__

#include <vector>
#include <string>
#include <cassert>
#include <limits>

#include "exception.hpp"

template<class Flow>
class Abstract_edge {
    public:
        enum {LAST_FLOW_AVAILABLE};
        virtual void allocate_flow(Flow const&) = 0;
        virtual void free_flow(Flow const&) = 0;
        virtual unsigned get_weight() const = 0;
};

template<class Flow>
class Edge : public Abstract_edge<Flow> {
    private:
        int max_flows_;
        int available_flows_;
        float weight_, former_weight_;
        std::vector<Flow*> passing_flows_;
        const float infinite_weight_ = std::numeric_limits<float>::infinity();

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
        Edge(int capacity = 1, float weight = 1) : max_flows_(capacity),
                                 available_flows_(capacity),
                                 weight_(weight),
                                 former_weight_(infinite_weight_)
        {
            if(weight_ <= 0)
                throw Edge_allocation_error();
        }

        int allocate_flow(Flow const& flow) {
            if (available_flows_ == 0)
                throw Edge_allocation_error();

            passing_flows_.push_back(&flow);
            if (available_flows_ == 1)
                switch_weight();

            return --available_flows_;
        }

        void free_flow(Flow const& flow) {
            assert(available_flows_ < max_flows_);
            if (weight_ == infinite_weight_)
                switch_weight();
            passing_flows_.remove(&flow);
            ++available_flows_;
        }

        float get_weight() const {
            return weight_;
        }
};

#endif
