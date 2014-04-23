#!/usr/bin/pyton

from unittest import TestCase, skip

from networkx import shortest_path

from flowsim.physical_layer.topology import Topology
from flowsim.physical_layer.topology import draw_graph
from flowsim.physical_layer.topology import torus2D
from flowsim.physical_layer.topology import torus3D
from flowsim.physical_layer.node import Node
from flowsim.physical_layer.edge import Edge
from flowsim.flowsim_exception import NoSuchEdge
from flowsim.flowsim_exception import NoSuchNode
from flowsim.flowsim_exception import DuplicatedNodeError
from flowsim.flowsim_exception import EdgeAllocationError


class Flow(object):

    pass


class Test_node(TestCase):

    def setUp(self):
        Node.counter = 0
        self.arrival_rate = 0.5
        self.service_rate = 0.5

    def test_init(self):
        nodes =\
            [Node(self.arrival_rate, self.service_rate, i) for i in range(3)]
        assert set(map(int, nodes)) == set([0, 1, 2])
        self.assertTrue(nodes[0].backup_arr_rate == self.arrival_rate)

    def test_swap(self):
        node1 = Node(1.2, 1.3, 0)
        node1.swap_arr_rate(None)
        self.assertTrue(node1.arrival_rate == 1.2)
        node1.swap_arr_rate(1.4)
        self.assertTrue(node1.arrival_rate == 1.4)
        node1.swap_arr_rate(None)
        self.assertTrue(node1.arrival_rate == 1.2)
        self.assertRaises(ValueError, node1.swap_arr_rate, -1)
        self.assertTrue(node1.arrival_rate == 1.2)


class Test_edge(TestCase):

    def test_get_const_value(self):
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


class Test_topology(TestCase):

    def setUp(self):
        Node.counter = 0
        self.arrival_rate = 0.5
        self.service_rate = 0.5

    def test_add_node(self):
        top = Topology()
        node = Node(self.arrival_rate, self.service_rate, 0)
        top.add_node(node)
        assert node in top.nodes()

    def test_add_nodes(self):
        top = Topology()
        nodes = [Node(self.arrival_rate, self.service_rate, 0),
                 Node(self.arrival_rate, self.service_rate, 1)]
        top.add_nodes(nodes)
        for node in nodes:
            assert node in top.nodes()

    def test_add_edge(self):
        top = Topology()
        node1 = Node(self.arrival_rate, self.service_rate, 0)
        node2 = Node(self.arrival_rate, self.service_rate, 1)
        edge = Edge()

        top.add_edge(node1, node2, edge)
        assert top[node1][node2]['object'] == edge

        edge = Edge()
        top.add_edge(node1, node2, edge, 10)
        assert top[node1][node2]['object'] == edge
        assert top[node1][node2]['weight'] == 10

    def test_add_edges(self):
        topo = Topology()
        nodes = [Node(self.arrival_rate, self.service_rate, 0)
                 for i in range(4)]
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
        assert(shortest_path(topo,
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

    def test_swap_node_arr_rate(self):
        topo = Topology()
        # Node int, edge (int, int)
        nodes = range(2)
        edges = [(0, 1)]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        topo.swap_node_arr_rate(1, 15)
        self.assertTrue(topo.id_to_node[1].arrival_rate == 15)
        topo.swap_node_arr_rate(1, 11)
        self.assertTrue(topo.id_to_node[1].arrival_rate == 11)
        topo.swap_node_arr_rate(1)
        self.assertTrue(topo.id_to_node[1].arrival_rate == self.arrival_rate)
        self.assertRaises(NoSuchNode, topo.swap_node_arr_rate, 5)

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

        # testing id_to_node dict
        for node in topo.nodes():
            self.assertTrue(node is topo.id_to_node[int(node)])

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
        # Node by name {'name', '_id'}
        nodes = [{'name': 'n1', '_id': 2}, {'name': 'n2', '_id': 3}]
        edges = [(2, 3)]
        topo.build_topology_from_int(nodes, edges,
                                     self.arrival_rate,
                                     self.service_rate)
        assert set(map(lambda x: x.get_name(), topo.nodes())) ==\
            set(['n1', 'n2'])

        topo = Topology()
        # Node by name {'name', '_id'}
        nodes = [{'name': 'n1', '_id': 2}, {'name': 'n2', '_id': 2}]
        edges = [(2, 3)]
        self.assertRaises(DuplicatedNodeError,
                          topo.build_topology_from_int,
                          nodes, edges,
                          self.arrival_rate,
                          self.service_rate)

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
        # Node by name {'name', '_id'}
        nodes = [{'name': 'n1', '_id': 2}, {'name': 'n2', '_id': 2}]
        edges = [{'nodes': ('n1', 'n2')}]
        self.assertRaises(DuplicatedNodeError,
                          topo.build_topology_from_int,
                          nodes, edges,
                          self.arrival_rate,
                          self.service_rate)

    @skip('Pauses tests')
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
        topo = Topology()
        tmp = torus2D(4, 3)
        assert(tmp[0] == range(4 * 3))
        # self.assert(tmp[1] == TODO )
        topo.build_topology_from_int(tmp[0], tmp[1],
                                     self.arrival_rate,
                                     self.service_rate)

    def test_torus3D(self):
        # TODO: check if realy torus
        topo = Topology()
        tmp = torus3D(4, 3, 3)
        assert(tmp[0] == range(4 * 3 * 3))
        topo.build_topology_from_int(tmp[0], tmp[1],
                                     self.arrival_rate,
                                     self.service_rate)
        # Visual check
        # draw_graph(topo)
