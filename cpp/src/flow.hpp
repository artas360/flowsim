#ifndef __FLOW_HPP__
#define __FLOW_HPP__

#include <vector>

#include "include.hpp"

template <class node_key_t, class container_t=std::vector<node_key_t>>
class Flow {
    public:
        Flow(container_t & node_list) : node_list_(std::move(node_list)) {}
        Flow(container_t && node_list) : node_list_(std::move(node_list)) {}

        container_t const& get_nodes() const {
            return node_list_;
        }

        typename container_t::size_type length() const {
            return node_list_.size();
        }

    private:
        const container_t node_list_;
};

#endif

#if TEST

int test_flow() {
    int keys[] = {1, 2, 5, 8};
    std::vector<int> vect(keys, keys + sizeof(keys) / sizeof(int));

    Flow<int> flow(vect);
    assert(flow.get_nodes()[2] == keys[2]);
    assert(flow.length() == sizeof(keys) / sizeof(int));

    Flow<int> flow2(std::vector<int> (keys, keys + sizeof(keys) / sizeof(int)));
    assert(flow.get_nodes()[2] == keys[2]);
    assert(flow.length() == sizeof(keys) / sizeof(int));

    return EXIT_SUCCESS;
}

int main() {
    return test_flow();
}

#endif
