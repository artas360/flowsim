#ifndef __EDGE_HPP__
#define __EDGE_HPP__

#include <list>
#include <limits>
#include <functional>

#include "include.hpp"

// wrapper for passing the value of infinite weight to the Edges
template <typename weight_t>
struct infinity_wrapper_std {
    // TODO test if static works
    weight_t operator()() {
        return std::numeric_limits<weight_t>::infinity();
    }
};

template<class flow_key_t, class weight_t, class flow_container_t>
class Abstract_edge {
    public:
        enum {LAST_FLOW_AVAILABLE};
        virtual size_t allocate_flow(flow_key_t const&) = 0;
        virtual void free_flow(flow_key_t const&) = 0;
        virtual weight_t const& get_weight() const = 0;
};

/**
 * \class Edge
 * \brief Simple implementation of the Abstract_edge class.
 */
template<class flow_key_t, class _weight=float, class _infinity_wrapper=infinity_wrapper_std<_weight>, class flow_container_t=std::list<flow_key_t>>
class Edge : public Abstract_edge<flow_key_t, _weight, flow_container_t> {
    public:
        typedef _weight weight_t;
        typedef size_t capacity_t;
        typedef _infinity_wrapper infinity_wrapper_t;
    private:
        // Maximum number of flows going through the link.
        size_t max_flows_;

        // Available slots for flows.
        size_t available_flows_;

        // Weight and its backup. Backup is used when setting edge weight to infinity.
        weight_t weight_, backup_weight_;

        // Cointainer of the passing flows keys.
        flow_container_t passing_flows_;

        // Value to be set when no more flows can be allocated on this edge.
        const weight_t infinite_weight_ = infinity_wrapper_t()();

    public:
        /**
            \brief Copy constructor using operator= overload
         */
        Edge(Edge const& edge) noexcept {
            *this = edge;
        }

        Edge(size_t capacity = 1, weight_t weight = 1) : max_flows_(capacity),
                                                         available_flows_(capacity),
                                                         weight_(weight),
                                                         backup_weight_(weight),
                                                         passing_flows_() {
            if(max_flows_ == 0 or weight_ <= 0.)
                throw Edge_allocation_error();
        }

        Edge& operator=(Edge const& other) {
            if(this != &other) {
                max_flows_ = other.max_flows_;
                available_flows_ = other.available_flows_;
                weight_ = other.weight_;
                backup_weight_ = other.backup_weight_;
                passing_flows_ = other.passing_flows_;
            }

            return *this;
        }

        /**
            \brief Registers flow, allocate ressources and sets the weight accordingly.
            \param Key of the flow to be added.
            \return number of remaning flow slots after allocation success.
            \warning throws!
         */
        size_t allocate_flow(flow_key_t const& flow) {
            if (available_flows_ == 0)
                throw Edge_allocation_error();

            passing_flows_.emplace_back(flow);
            if (available_flows_ == 1)
                weight_ = infinite_weight_;

            return --available_flows_;
        }

        /**
            \brief Frees ressources of the edge and sets its weight accordingly.
            \param key of the flow to be removed.
         */
        void free_flow(flow_key_t const& flow) {
            assert(available_flows_ < max_flows_);
            if (weight_ == infinite_weight_)
                weight_ = backup_weight_;
            passing_flows_.remove(flow);
            ++available_flows_;
        }

        /**
            \warning Needs to return a reference for compatibility w/ topology.
            \return reference to the stored weight.
         */
        weight_t const& get_weight() const {
            return weight_;
        }

        /**
            \brief returns a const ref to the container the keys the flows passing through the edge.
            \return const ref to the container of the keys of pasing flows.
         */
        flow_container_t const& get_flows() const {
            return passing_flows_;
        }
};

#endif
