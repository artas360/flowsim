import networkx
import sys
from edge import Edge
from node import Node, Entry_node, Exit_node
from flowsim.flowsim_exception import NoSuchEdge,\
    DuplicatedNodeError,\
    NoPathError


class Topology(networkx.DiGraph):

    def __init__(self):
        super(self.__class__, self).__init__()
        self.entry_nodes = []
        self.exit_nodes = []
        self.infinity = float('inf')

    def add_node(self, node):
        super(self.__class__, self).add_node(node)

    def add_nodes(self, nbunch):
        self.add_nodes_from(nbunch)

    def add_edge(self, node1, node2, edge_object, edge_weight=1):
        self.add_edges([(node1, node2, {'object': edge_object})],
                       edge_weight=edge_weight)

    def add_edges(self, edge_list, edge_weight=1):
        # Each edge (node1, node2, {'object': object})
        self.add_edges_from(edge_list, weight=edge_weight)

    def set_edge_unavailable(self, node1, node2):
        try:
            self[node1][node2]['edge_former_weight'] =\
                self[node1][node2]['weight']
            self[node1][node2]['weight'] = self.infinity
        except KeyError:
            raise NoSuchEdge()

    def free_edge(self, node1, node2, flow):
        try:
            if self[node1][node2]['weight'] == self.infinity:
                self[node1][node2]['weight'] =\
                    self[node1][node2]['edge_former_weight']
            if flow is not None:
                self[node1][node2]['object'].free_flow(flow)
        except KeyError:
            raise NoSuchEdge

    def get_edge_object(self, node1, node2):
        return self[node1][node2]['object']

    def shortest_path(self, node1, node2):
        try:
            path = networkx.shortest_path(self, node1, node2, weight='weight')
            if self.infinity in [self[path[i]][path[i + 1]]['weight']
                                 for i in xrange(len(path) - 1)]:
                raise NoPathError
            return path
        except networkx.NetworkXNoPath:
            raise NoPathError

    def build_topology_from_int(self, nodes, edges,
                                arrival_rate=None, service_rate=None):
        # nodes -> list of int or list of (int, str) str->entry,exit
        # edges -> list of (node1, node2) or (node1, node2, capacity)
        # or (node1, node2, capacity, weight)
        # Every edge is both ways if not edge['unidir'] set or set to False
        temp_dict = dict()

        # TODO nodes.sort()
        for node in nodes:
            if type(node) == int:
                node = {'number': node}
            elif type(node) == tuple or type(node) == list:
                if len(node) == 2:
                    node = {'number': node[0], 'type': node[1]}
                else:
                    raise TypeError
            if type(node) == dict:
                node_type = node.pop('type', '')
                if not 'name' in node:
                    node['name'] = node['number']
                if not 'arrival_rate' in node:
                    node['arrival_rate'] = arrival_rate
                if not 'service_rate' in node:
                    node['service_rate'] = service_rate
                if node_type == 'entry':
                    new_node = Entry_node(**node)
                    self.entry_nodes.append(new_node)
                elif node_type == 'exit':
                    new_node = Exit_node(**node)
                    self.exit_nodes.append(new_node)
                else:
                    new_node = Node(**node)
                if node['name'] in temp_dict:
                    raise DuplicatedNodeError
                temp_dict[node['name']] = new_node
                self.add_node(new_node)
            else:
                raise TypeError

        for edge in edges:
            # TODO duplicated node -> increase capacity?
            temp_edge = dict()
            if type(edge) == tuple or type(edge) == list:
                temp_edge['nodes'] = (edge[0], edge[1])
                if len(edge) == 2:
                    pass
                elif len(edge) == 3:
                    temp_edge['capacity'] = edge[2]
                elif len(edge) == 4:
                    temp_edge['capacity'] = edge[2]
                    temp_edge['weight'] = edge[3]
                else:
                    raise TypeError
                edge = temp_edge
            if type(edge) == dict:
                nodes = edge.pop('nodes')
                weight = edge.pop('weight', 1)
                unidir = edge.pop('unidir', False)
                new_edge = Edge(**edge)
                self.add_edge(temp_dict[nodes[0]],
                              temp_dict[nodes[1]],
                              new_edge,
                              weight)
                if not unidir:
                    new_edge = Edge(**edge)
                    self.add_edge(temp_dict[nodes[1]],
                                  temp_dict[nodes[0]],
                                  new_edge,
                                  weight)
            else:
                raise TypeError()

    def get_random_entry_node(self, number):
        if len(self.entry_nodes) == 0:
            #If no entry node, all nodes are entry nodes
            return self.nodes()[number % self.number_of_nodes()]
        return self.entry_nodes[number % len(self.entry_nodes)]

    def get_random_exit_node(self, number):
        if len(self.exit_nodes) == 0:
            #If no exit node, all nodes are exit nodes
            return self.nodes()[number % self.number_of_nodes()]
        return self.exit_nodes[number % len(self.exit_nodes)]

    def get_entry_nodes(self):
        return self.entry_nodes if len(self.entry_nodes) > 0 else self.nodes()

    def import_topology(self, filename, arrival_rate, service_rate):
        #TODO : catch exceptions
        g = networkx.read_graphml(filename)
        nodes = [{'name': node,
                  'arrival_rate': arrival_rate,
                  'service_rate': service_rate} for node in g.nodes()]

        self.build_topology_from_int(nodes, g.edges())

    def reset(self, arrival_rate=None, service_rate=None):
        map(lambda x: x.reset(arrival_rate, service_rate), self.nodes_iter())
        map(lambda x: x[2]['object'].reset(),
            self.edges_iter(data=True))
        map(lambda x: self.free_edge(x[0], x[1], None),
            self.edges_iter(data=True))


def torus2D(x, y, start_index=0):
    if x <= 2 or y <= 2:
        raise NotImplemented
    array = [[start_index + i + j * y for i in xrange(y)] for j in xrange(x)]
    nodes = []
    edges = set()
    for j in xrange(y):
        for i in xrange(x):
            nodes.append(array[i][j])
            edges.add(frozenset((array[i][j], array[(i+1) % x][j])))
            edges.add(frozenset((array[i][j], array[(i-1) % x][j])))
            edges.add(frozenset((array[i][j], array[i][(j-1) % y])))
            edges.add(frozenset((array[i][j], array[i][(j+1) % y])))
    return ([item for sublist in array for item in sublist],
            [tuple(fset) for fset in edges])


def torus3D(x, y, z):
    if x <= 2 or y <= 2 or z <= 2:
        raise NotImplemented

    nodes = []
    edges = []

    for depth in xrange(z):
        (tmp_nodes, tmp_edges) = torus2D(x, y, x * y * depth)
        nodes.extend(tmp_nodes)
        edges.extend(tmp_edges)

    # 3rd Dim edges
    edges.extend([(i, i + x * y) for i in xrange(x * y * (z - 1))])
    edges.extend([(i, i - x * y * (z - 1))
                 for i in xrange(x * y * (z - 1), x * y * z)])

    return (nodes, edges)


def draw_graph(topology):
    import matplotlib.pyplot
    pos = networkx.spring_layout(topology)
    node_labels = dict([(u, u.number) for u in topology.nodes_iter()])
    networkx.draw(topology, pos=pos, hold=True, with_labels=False)
    networkx.draw_networkx_labels(topology, pos, labels=node_labels)
    matplotlib.pyplot.draw()
    matplotlib.pyplot.show()
