#ifndef __FLOW_HPP__
#define __FLOW_HPP__

#include <vector>

#include "include.hpp"

template <class edge_key_t, class container_t=std::vector<edge_key_t>>
class Flow {
    public:
        typedef typename container_t::size_type size_type;
        typedef typename container_t::const_iterator const_iterator;
        Flow(container_t const& edge_list) : edge_list_(edge_list) /*copy*/ {}
        Flow(container_t const&& edge_list) : edge_list_(edge_list) /*move*/ {}

        container_t const& get_edges() const {
            return edge_list_;
        }

        size_type length() const {
            return edge_list_.size();
        }

    private:
        const container_t edge_list_;
};


#if TEST_FLOW

#include <iostream>

int test_flow() {
    int keys[] = {1, 2, 5, 8};
    std::vector<int> vect(keys, keys + sizeof(keys) / sizeof(int));

    Flow<int> flow(vect);
    FTEST(flow.get_edges()[2] == keys[2]);
    FTEST(flow.length() == sizeof(keys) / sizeof(int));

    Flow<int> flow2(std::vector<int> (keys, keys + sizeof(keys) / sizeof(int)));
    FTEST(flow2.get_edges()[2] == keys[2]);
    FTEST(flow2.length() == sizeof(keys) / sizeof(int));

    keys[2] = 0;

    FTEST(flow.get_edges()[2] == 5);
    FTEST(flow2.get_edges()[2] == 5);

    return EXIT_SUCCESS;
}

int main() {
    return test_flow();
}

#endif
#endif
