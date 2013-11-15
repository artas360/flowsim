#!/usr/bin/pyton

import unittest
import networkx
from flowsim.physical_layer.node import Node
from physical_layer.edge import Edge, EdgeAllocationError
from physical_layer.topology import Topology
import physical_layer.topology as pt

class Flow(object):
    pass

class Test_node(unittest.TestCase):
    def test_init(self):
        nodes=[Node() for i in range(3)] + [Node(6)] + [Node(), Node(-1)]
        assert map(int, nodes) == [0,1,2,6,7,8]

class Test_edge(unittest.TestCase):

    def test_get_sont_value(self):
        edge = Edge()
        assert edge.get_const_value('LAST_FLOW_AVAILABLE') == -1

    def test_allocate_flow(self):
        edge = Edge(1)

        flow = Flow()
        assert edge.allocate_flow(flow) == edge.get_const_value('LAST_FLOW_AVAILABLE')
        
        assert flow in edge.passing_flows

        flow = Flow()
        self.assertRaises(EdgeAllocationError, edge.allocate_flow, flow)

        assert not flow in edge.passing_flows

    def test_free_flow(self):
        edge = Edge(1)
        flow = Flow()

        edge.allocate_flow(flow)

        self.assertRaises(EdgeAllocationError, edge.free_flow, Flow())

        edge.free_flow(flow)

        assert not flow in edge.passing_flows

        self.assertRaises(EdgeAllocationError, edge.free_flow, Flow())

class Test_topology(unittest.TestCase):

    def test_add_node(self):
        top = Topology()
        node = Node()
        top.add_node(node)
        assert node in top.nodes()

    def test_add_nodes(self):
        top = Topology()
        nodes = [Node(), Node()]
        top.add_nodes(nodes)
        for node in nodes: assert node in top.nodes()

    def test_add_edge(self):
        top = Topology()
        node1 = Node()
        node2 = Node()
        edge = Edge()

        top.add_edge(node1, node2, edge)
        assert top[node1][node2]['object'] == edge

        edge = Edge()
        top.add_edge(node1, node2, edge, 10)
        assert top[node1][node2]['object'] == edge
        assert top[node1][node2]['weight'] == 10

    def test_add_edges(self):
        top = Topology()
        nodes = [Node() for i in range(4)]
        edges = [Edge() for i in range(3)]
        top.add_edges([(nodes[0], nodes[1], {'object':edges[0]}),
                       (nodes[1], nodes[2], {'object':edges[1]}),
                       (nodes[3], nodes[1], {'object':edges[2]})])

        assert top[nodes[0]][nodes[1]]['object'] == edges[0]
        assert top[nodes[1]][nodes[2]]['object'] == edges[1]
        assert top[nodes[3]][nodes[1]]['object'] == edges[2]

    def test_shortest_path(self):
        topo = Topology()

#       Creating a topology:
#    
#            4 ---------- 5
#             \
#         3----2
#         |   /|
#         |  / |
#         | /  |
#         |/   |
#         0----1
        nodes = [Node(i) for i in xrange(6)]
        topo.add_nodes(nodes)
        topo.add_edges([(nodes[0],nodes[3],{'object':Edge()}),
                        (nodes[0],nodes[1],{'object':Edge()}),
                        (nodes[0],nodes[2],{'object':Edge()}),
                        (nodes[1],nodes[2],{'object':Edge()}),
                        (nodes[3],nodes[2],{'object':Edge()}),
                        (nodes[4],nodes[2],{'object':Edge()}),
                        (nodes[4],nodes[5],{'object':Edge()})],
                        edge_weight = 1)

        print nodes[0] in topo.nodes()
        # test shortest path 1 -> 6
        print map(int, networkx.shortest_path(topo, nodes[0], nodes[5], weight='weight'))
        print pt.topology_shortest_path(topo, nodes[0], nodes[5])
        print topo.shortest_path(nodes[0], nodes[5])
               # == [nodes[0],nodes[2],nodes[4],nodes[5]])

#        # Adding link 4 -> 5
#        topo.add_edge(nodes[4],nodes[5], edge(), edge_weight=1)
#        # Link 3 -> 5 not available
#        edge_dict=topo[nodes[3]][nodes[5]]
#        topo.set_edge_unavailable(nodes[3],nodes[5])
#        assert(networkx.shortest_path(topo, nodes[1], nodes[6], weight='weight') 
#                == [nodes[1],nodes[4],nodes[5],nodes[6]])
#        # Restoring previous link
#        topo.set_edge_available(3,5)
#        topo.remove_edge(4,5)
#        assert (networkx.shortest_path(topo, 1, 6, weight='weight') == [1,3,5,6])
#        assert (topo[3][5] == edge_dict)




