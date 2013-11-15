#!/usr/bin/python

import networkx
from edge import Edge

class Foo_edge(object):
    pass

def topology_shortest_path(topology, src_node, dst_node):
    networkx.shortest_path(topology, src_node, dst_node, weight='weight')

class Topology(networkx.Graph):
    def __init__(self):
        super(self.__class__, self).__init__()
        self.edges_unavailable=dict()
    
    def add_node(self, node):
        super(self.__class__, self).add_node(node)

    def add_nodes(self, nbunch):
        self.add_nodes_from(nbunch)

    def add_edge(self, node1, node2, edge_object, edge_weight = 1):
        self.add_edges([(node1, node2, {'object':edge_object})], edge_weight = edge_weight)

    def add_edges(self, edge_list, edge_weight = 1):
        # Each edge (node1, node2, {'object':object})
        self.add_edges_from(edge_list, weight=edge_weight)

    def set_edge_unavailable(self, node1, node2):
        edge=self[node1][node2] # it's a dict() ! Keeping it all
        self.edges_unavailable[(node1, node2)] = edge
        self.remove_edge(node1, node2)

    def free_edge(self, edge, flow):
        try:
            (node1, node2)= self.edges_unavailable.keys[edges_unavailable.values().index(edge)]
            self.set_edge_available(node1, node2)

        except ValueError:
            pass

        finally:
            edge.free_flow(flow)


    def set_edge_available(self, node1, node2):
        try:
            edge=self.edges_unavailable.pop((node1,node2)) 
        except IndexError:
            return
        super(self.__class__, self).add_edge(node1, node2)
        self[node1][node2] = edge

    def get_edge_object(self, node1, node2):
        return self[node1][node2]['object']

    def shortest_path(self, node1, node2):
        networkx.shortest_path(self, node1, node2, weight='weight')

    def build_topology_from_int(self, nodes, edges):
        # nodes -> list of int
        # edges -> list of (node1, node2) or (node1, node2, capacity)
        # or (node1, node2, capacity, weight)
        nodes.sort()
        self.add_nodes([Node(i) for i in nodes])
        for edge in edges:
            if len(edge) == 2:
                self.add_edge(edge[0],edge[1], Edge())
            elif len(edge) == 3:
                self.add_edge(edge[0],edge[1], Edge(edge[2]))
            elif len(edge) == 4:
                self.add_edge(edge[0],edge[1], Edge(edge[2]), edge[3])
            else:
                raise TypeError()

    def import_topology(self, filename):
        # TODO
        pass

    def draw_graph(self):
        # TODO
        pass



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
    topo.set_edge_unavailable(3,5)
    assert(networkx.shortest_path(topo, 1, 6, weight='weight') == [1,4,5,6])
    print topology_shortest_path(topo, 1, 6)
    # Restoring previous link
    topo.set_edge_available(3,5)
    topo.remove_edge(4,5)
    assert (networkx.shortest_path(topo, 1, 6, weight='weight') == [1,3,5,6])
    assert (topo[3][5] == edge_dict)

