#!/usr/bin/pyton

import unittest
import networkx
from flowsim.physical_layer.node import Node
from flowsim.physical_layer.edge import Edge, EdgeAllocationError
from flowsim.physical_layer.topology import Topology
from flowsim.physical_layer.topology import NoSuchEdge


class Flow(object):

    pass


class Test_node(unittest.TestCase):

    def test_init(self):
        nodes = [Node() for i in range(3)] + [Node(6)] + [Node(), Node(-1)]
        assert map(int, nodes) == [0, 1, 2, 6, 7, 8]


class Test_edge(unittest.TestCase):

    def test_get_sont_value(self):
        edge = Edge()
        assert edge.get_const_value('LAST_FLOW_AVAILABLE') == -1

    def test_allocate_flow(self):
        edge = Edge(1)

        flow = Flow()
        assert edge.allocate_flow(flow) ==\
               edge.get_const_value('LAST_FLOW_AVAILABLE')

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
        for node in nodes:
            assert node in top.nodes()

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
        topo = Topology()
        nodes = [Node() for i in range(4)]
        edges = [Edge() for i in range(3)]
        topo.add_edges([(nodes[0], nodes[1], {'object': edges[0]}),
                       (nodes[1], nodes[2], {'object': edges[1]}),
                       (nodes[3], nodes[1], {'object': edges[2]})])

        assert topo[nodes[0]][nodes[1]]['object'] == edges[0]
        assert topo[nodes[1]][nodes[2]]['object'] == edges[1]
        assert topo[nodes[3]][nodes[1]]['object'] == edges[2]

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
        topo.add_edges([(nodes[0], nodes[3], {'object': Edge()}),
                        (nodes[0], nodes[1], {'object': Edge()}),
                        (nodes[0], nodes[2], {'object': Edge()}),
                        (nodes[1], nodes[2], {'object': Edge()}),
                        (nodes[3], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[5], {'object': Edge()})],
                        edge_weight=1)

        # test shortest path 0 -> 5
        assert topo.shortest_path(nodes[0], nodes[5]) \
                == [nodes[0], nodes[2], nodes[4], nodes[5]]

        # Adding link 3 -> 4
        topo.add_edge(nodes[3], nodes[4], Edge(), edge_weight=1)
        # Link 2 -> 4 not available
        topo.set_edge_unavailable(nodes[2], nodes[4])
        assert(networkx.shortest_path(
                topo, nodes[0],
                nodes[5], weight='weight')
                == [nodes[0], nodes[3], nodes[4], nodes[5]])

    def test_set_edge_unavailable(self):
        topo = Topology()
        nodes = [Node(i) for i in xrange(6)]
        topo.add_nodes(nodes)
        topo.add_edges([(nodes[0], nodes[3], {'object': Edge()}),
                        (nodes[0], nodes[1], {'object': Edge()}),
                        (nodes[0], nodes[2], {'object': Edge()}),
                        (nodes[1], nodes[2], {'object': Edge()}),
                        (nodes[3], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[5], {'object': Edge()})],
                        edge_weight=1)
        edge=topo[nodes[1]][nodes[2]]

        topo.set_edge_unavailable(nodes[1], nodes[2])

        self.assertRaises(KeyError, topo.get_edge_object, nodes[1], nodes[2])

        assert edge in topo.edges_unavailable.values()

        self.assertRaises(NoSuchEdge, topo.set_edge_unavailable, nodes[0], nodes[5])

    def test_set_edge_available(self):

        topo = Topology()
        nodes = [Node(i) for i in xrange(6)]
        topo.add_nodes(nodes)
        topo.add_edges([(nodes[0], nodes[3], {'object': Edge()}),
                        (nodes[0], nodes[1], {'object': Edge()}),
                        (nodes[0], nodes[2], {'object': Edge()}),
                        (nodes[1], nodes[2], {'object': Edge()}),
                        (nodes[3], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[5], {'object': Edge()})],
                        edge_weight=1)
        edge=topo[nodes[1]][nodes[2]]

        topo.set_edge_unavailable(nodes[1], nodes[2])

        self.assertRaises(KeyError, topo.get_edge_object, nodes[1], nodes[2])

        topo.set_edge_available(nodes[1], nodes[2])

        assert topo[nodes[1]][nodes[2]] == edge

        assert len(topo.edges_unavailable) == 0

    @unittest.skip('Not implemented yet')
    def test_build_topology_from_int(self):
        assert False

    @unittest.skip('Not implemented yet')
    def test_import_topology(self):
        assert False

    def test_free_edge(self):
        topo = Topology()
        nodes = [Node(i) for i in xrange(6)]
        topo.add_nodes(nodes)
        topo.add_edges([(nodes[0], nodes[3], {'object': Edge()}),
                        (nodes[0], nodes[1], {'object': Edge()}),
                        (nodes[0], nodes[2], {'object': Edge()}),
                        (nodes[1], nodes[2], {'object': Edge()}),
                        (nodes[3], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[5], {'object': Edge()})],
                        edge_weight=1)

        edge=topo[nodes[1]][nodes[2]]['object']

        flow=Flow()
        edge.allocate_flow(flow)

        topo.set_edge_unavailable(nodes[1], nodes[2])

        topo.free_edge(edge, flow)

        assert not topo[nodes[1]][nodes[2]] in topo.edges_unavailable.values()

        assert flow not in edge.passing_flows



