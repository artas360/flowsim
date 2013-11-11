#!/usr/bin/python

import networkx

class Foo_edge(object):
    pass

class Topology(networkx.Graph):
    def __init__(self):
        super(self.__class__, self).__init__()
        self.busy_edges=dict()
    
    def add_node(self, node):
        self.add_node(node)

    def add_nodes(self, nbunch):
        self.add_nodes_from(nbunch)

    def add_edge(self, node1, node2, edge_object, edge_weight = 1):
        self.add_edges([(node1, node2, {'object':edge_object})], edge_weight = edge_weight)

    def add_edges(self, edge_list, edge_weight = 1):
        # Each edge (node1, node2, {'object':object})
        self.add_edges_from(edge_list, weight=edge_weight)

    def busy_edge(self, node1, node2):
        edge=self[node1][node2] # it's a dict() ! Keeping it all
        self.busy_edges[(node1, node2)] = edge
        self.remove_edge(node1, node2)

    def restore_edge(self, node1, node2):
        edge=self.busy_edges.pop((node1,node2)) 
        super(self.__class__, self).add_edge(node1, node2)
        self[node1][node2] = edge

    def get_edge_object(self, node1, node2):
        return self[node1][node2]['object']

if __name__ == "__main__":

    topo = Topology()

#   Creating a topology:
#
#        5 ---------- 6
#         \
#     4----3
#     |   /|
#     |  / |
#     | /  |
#     |/   |
#     1----2

    topo.add_nodes(xrange(1,6))
    topo.add_edges([(1,4,{'object':Foo_edge()}),(1,2,{'object':Foo_edge()}),(1,3,{'object':Foo_edge()}),(2,3,{'object':Foo_edge()}),(4,3,{'object':Foo_edge()}),(5,3,{'object':Foo_edge()}),(5,6,{'object':Foo_edge()})], edge_weight = 1)

    # test shortest path 1 -> 6
    assert (networkx.shortest_path(topo, 1, 6, weight='weight') == [1,3,5,6])

    # Adding link 4 -> 5
    topo.add_edge(4,5, Foo_edge(), edge_weight=1)
    # Link 3 -> 5 not available
    edge_dict=topo[3][5]
    topo.busy_edge(3,5)
    assert(networkx.shortest_path(topo, 1, 6, weight='weight') == [1,4,5,6])
    # Restorng previous link
    topo.restore_edge(3,5)
    topo.remove_edge(4,5)
    assert (networkx.shortest_path(topo, 1, 6, weight='weight') == [1,3,5,6])
    assert (topo[3][5] == edge_dict)

