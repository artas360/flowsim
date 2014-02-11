#ifndef __FLOW_HPP__
#define __FLOW_HPP__

#include <functional>
#include <vector>

#include "include.hpp"

// Storing refs to keys instead of keys, is it usefull?
// In case keys are not base types
template <class node_key_t, class container_t=std::vector<std::reference_wrapper<node_key_t>>>
class Flow {
    public:
        Flow(container_t const& node_list) : node_list_(node_list) /*copy*/ {}
        Flow(container_t const&& node_list) : node_list_(node_list) /*move*/ {}

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
    std::vector<std::reference_wrapper<int>> vect(keys, keys + sizeof(keys) / sizeof(int));

    Flow<int> flow(vect);
    FTEST(flow.get_nodes()[2] == keys[2]);
    FTEST(flow.length() == sizeof(keys) / sizeof(int));

    Flow<int> flow2(std::vector<std::reference_wrapper<int>> (keys, keys + sizeof(keys) / sizeof(int)));
    FTEST(flow2.get_nodes()[2] == keys[2]);
    FTEST(flow2.length() == sizeof(keys) / sizeof(int));

    keys[2] = 0;

    FTEST(flow.get_nodes()[2] == keys[2]);
    FTEST(flow2.get_nodes()[2] == keys[2]);

    return EXIT_SUCCESS;
}

int main() {
    return test_flow();
}

#endif
