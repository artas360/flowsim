#!/usr/bin/pyton

import unittest
import networkx
from flowsim.physical_layer.node import Node
from flowsim.physical_layer.edge import Edge, EdgeAllocationError
from flowsim.physical_layer.topology import Topology
from flowsim.physical_layer.topology import draw_graph
from flowsim.physical_layer.topology import torus2D
from flowsim.physical_layer.topology import NoSuchEdge
from flowsim.physical_layer.topology import DuplicatedNodeError


class Flow(object):

    pass


class Test_node(unittest.TestCase):

    def setUp(self):
        Node.counter = 0
        self.arrival_rate = 0.5
        self.service_rate = 0.5

    def test_init(self):
        nodes =\
            [Node(self.arrival_rate, self.service_rate) for i in range(3)] +\
            [Node(self.arrival_rate, self.service_rate, 6)] +\
            [Node(self.arrival_rate, self.service_rate),
             Node(self.arrival_rate, self.service_rate, -1)]
        assert set(map(int, nodes)) == set([0, 1, 2, 6, 7, 8])


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

    def setUp(self):
        Node.counter = 0
        self.arrival_rate = 0.5
        self.service_rate = 0.5

    def test_add_node(self):
        top = Topology()
        node = Node(self.arrival_rate, self.service_rate)
        top.add_node(node)
        assert node in top.nodes()

    def test_add_nodes(self):
        top = Topology()
        nodes = [Node(self.arrival_rate, self.service_rate),
                 Node(self.arrival_rate, self.service_rate)]
        top.add_nodes(nodes)
        for node in nodes:
            assert node in top.nodes()

    def test_add_edge(self):
        top = Topology()
        node1 = Node(self.arrival_rate, self.service_rate)
        node2 = Node(self.arrival_rate, self.service_rate)
        edge = Edge()

        top.add_edge(node1, node2, edge)
        assert top[node1][node2]['object'] == edge

        edge = Edge()
        top.add_edge(node1, node2, edge, 10)
        assert top[node1][node2]['object'] == edge
        assert top[node1][node2]['weight'] == 10

    def test_add_edges(self):
        topo = Topology()
        nodes = [Node(self.arrival_rate, self.service_rate) for i in range(4)]
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
        nodes = [Node(self.arrival_rate, self.service_rate, i)
                 for i in xrange(6)]
        topo.add_nodes(nodes)
        topo.add_edges([(nodes[0], nodes[3], {'object': Edge()}),
                        (nodes[0], nodes[1], {'object': Edge()}),
                        (nodes[0], nodes[2], {'object': Edge()}),
                        (nodes[1], nodes[2], {'object': Edge()}),
                        (nodes[3], nodes[2], {'object': Edge()}),
                        (nodes[2], nodes[4], {'object': Edge()}),
                        (nodes[4], nodes[5], {'object': Edge()})],
                       edge_weight=1)

        # test shortest path 0 -> 5
        assert topo.shortest_path(nodes[0], nodes[5]) ==\
            [nodes[0], nodes[2], nodes[4], nodes[5]]

        # Adding link 3 -> 4
        topo.add_edge(nodes[3], nodes[4], Edge(), edge_weight=1)
        # Link 2 -> 4 not available
        topo.set_edge_unavailable(nodes[2], nodes[4])
        assert(networkx.shortest_path(topo,
                                      nodes[0],
                                      nodes[5],
                                      weight='weight') ==
               [nodes[0], nodes[3], nodes[4], nodes[5]])

    def test_set_edge_unavailable(self):
        topo = Topology()
        nodes = [Node(self.arrival_rate, self.service_rate, i)
                 for i in xrange(6)]
        topo.add_nodes(nodes)
        topo.add_edges([(nodes[0], nodes[3], {'object': Edge()}),
                        (nodes[0], nodes[1], {'object': Edge()}),
                        (nodes[0], nodes[2], {'object': Edge()}),
                        (nodes[1], nodes[2], {'object': Edge()}),
                        (nodes[3], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[5], {'object': Edge()})],
                       edge_weight=1)
        edge = topo[nodes[1]][nodes[2]]

        topo.set_edge_unavailable(nodes[1], nodes[2])

        assert topo[nodes[1]][nodes[2]]['weight'] == topo.infinity

        self.assertRaises(NoSuchEdge,
                          topo.set_edge_unavailable,
                          nodes[0],
                          nodes[5])

    def test_build_topology_from_int(self):

        topo = Topology()
        # Node int, edge (int, int)
        nodes = range(4)
        edges = [(0, 1), (2, 1)]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        assert set(map(int, topo.nodes())) == set(nodes)
        assert set([frozenset([node1.get_name(), node2.get_name()])
                    for node1, node2 in topo.edges()])\
            == set([frozenset([node1, node2]) for node1, node2 in edges])

        topo = Topology()
        # Node int, edge (int, int)
        nodes = [1, 1, 2]
        edges = [(2, 1)]
        self.assertRaises(DuplicatedNodeError,
                          topo.build_topology_from_int,
                          nodes,
                          edges,
                          self.arrival_rate,
                          self.service_rate)

        topo = Topology()
        # Node (int, type), edge (int, int)
        nodes = [(0, 'entry'), (1, 'exit'), (2), (3, 'exit'), (4, 'entry')]
        edges = [(0, 1), (2, 1)]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        assert set(map(int, topo.nodes())) == set(range(5))
        assert set(map(int, topo.entry_nodes)) == set([0, 4])
        assert set(map(int, topo.exit_nodes)) == set([1, 3])

        topo = Topology()
        # Node by name {'name'}
        nodes = [{'name': 'n1'}, {'name': 'n2'}]
        edges = [('n1', 'n2')]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        assert set(map(lambda x: x.get_name(), topo.nodes())) ==\
            set(['n1', 'n2'])

        topo = Topology()
        # Node by name {'name', 'number'}
        nodes = [{'name': 'n1', 'number': 2}, {'name': 'n2', 'number': 3}]
        edges = [('n1', 'n2')]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        assert set(map(lambda x: x.get_name(), topo.nodes())) ==\
            set(['n1', 'n2'])

        topo = Topology()
        # Node by name {'name', 'number'}
        nodes = [{'name': 'n1', 'number': 2}, {'name': 'n2', 'number': 2}]
        edges = [('n1', 'n2')]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        assert set(map(lambda x: x.get_name(), topo.nodes())) ==\
            set(['n1', 'n2'])

        topo = Topology()
        # Edge (node, node, capacity)
        topo.build_topology_from_int([0, 1],
                                     [(0, 1, 2)],
                                     self.arrival_rate,
                                     self.service_rate)
        nodes = topo.nodes()
        assert topo[nodes[0]][nodes[1]]['object'].max_flows == 2

        topo = Topology()
        # Edge (node, node, capacity)
        topo.build_topology_from_int([0, 1],
                                     [(0, 1, 2, 3)],
                                     self.arrival_rate,
                                     self.service_rate)
        nodes = topo.nodes()
        assert topo[nodes[0]][nodes[1]]['object'].max_flows == 2
        assert topo[nodes[0]][nodes[1]]['weight'] == 3

        topo = Topology()
        # Node by name {'name', 'number'}
        nodes = [{'name': 'n1', 'number': 2}, {'name': 'n2', 'number': 2}]
        edges = [{'nodes': ('n1', 'n2')}]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        assert set(map(lambda x: x.get_name(), topo.edges()[0])) ==\
            set(['n1', 'n2'])

    def test_import_topology(self):
        filename = './graph0_yed.graphml'
        imported_graph = networkx.read_graphml(filename)
        topo = Topology()
        topo.import_topology(filename, self.arrival_rate, self.service_rate)
        assert set(map(lambda x: x.get_name(), topo.nodes())) ==\
            set(imported_graph.nodes())
        assert set([frozenset([node1.get_name(), node2.get_name()])
                    for node1, node2 in topo.edges()]) ==\
            set([frozenset([node1, node2])
                 for node1, node2 in imported_graph.edges()])

    @unittest.skip('Pauses tests')
    def test_draw_graph(self):
        topo = Topology()
        nodes = range(4)
        edges = [(0, 1), (2, 1)]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        draw_graph(topo)

    def test_free_edge(self):
        topo = Topology()
        nodes = [Node(self.arrival_rate, self.service_rate, i)
                 for i in xrange(6)]
        topo.add_nodes(nodes)
        topo.add_edges([(nodes[0], nodes[3], {'object': Edge()}),
                        (nodes[0], nodes[1], {'object': Edge()}),
                        (nodes[0], nodes[2], {'object': Edge()}),
                        (nodes[1], nodes[2], {'object': Edge()}),
                        (nodes[3], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[2], {'object': Edge()}),
                        (nodes[4], nodes[5], {'object': Edge()})],
                       edge_weight=1)

        edge = topo[nodes[1]][nodes[2]]['object']

        flow = Flow()
        edge.allocate_flow(flow)

        topo.set_edge_unavailable(nodes[1], nodes[2])

        topo.free_edge(nodes[1], nodes[2], flow)

        assert not topo[nodes[1]][nodes[2]]['weight'] == topo.infinity

        assert flow not in edge.passing_flows

    def test_torus2D(self):
        # TODO: check if realy torus
        topo = Topology()
        tmp = torus2D(4, 3)
        topo.build_topology_from_int(tmp[0], tmp[1],
                                     self.arrival_rate,
                                     self.service_rate)
