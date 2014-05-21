import networkx
from flowsim.physical_layer.edge import Edge, Meta_edge
from flowsim.physical_layer.node import Node, Entry_node, Exit_node
from flowsim.flowsim_exception import NoSuchEdge,\
    DuplicatedNodeError,\
    NoPathError,\
    NoSuchNode,\
    EdgePlugError


class Topology(networkx.DiGraph):

    def __init__(self):
        super(self.__class__, self).__init__()
        self.entry_nodes = []
        self.exit_nodes = []
        self.infinity = float('inf')
        self.id_to_node = {}

    def add_node(self, node):
        super(self.__class__, self).add_node(node)

    def add_nodes(self, nbunch):
        self.add_nodes_from(nbunch)

    def add_edge(self, node1, node2, edge_object, edge_weight=None):
        if edge_weight is None:
            edge_weight = edge_object.weight
        try:
            edge = self[node1][node2]['object']
        except KeyError:
            if isinstance(edge_object, Edge):
                edge_object = Meta_edge(edge_object)
            elif isinstance(edge_object, Meta_edge):
                pass
            else:
                raise ValueError
            try:
                node1.plug_in_edge(tx=True)
                node2.plug_in_edge(tx=False)
            except EdgePlugError:
                raise
            super(self.__class__, self).add_edge(node1, node2,
                                                 {'object': edge_object,
                                                  'weight': edge_weight})
        else:
            if isinstance(edge, Meta_edge):
                edge.add_edge(edge_object)
            elif isinstance(edge, Edge):
                edge_object = Meta_edge(edge_object)
                edge_object.add_edge(self[node1][node2])
                self.remove_edge(node1, node2)
                try:
                    node1.plug_in_edge(tx=True)
                    node2.plug_in_edge(tx=False)
                except EdgePlugError:
                    raise
                super(self.__class__, self).add_edge(node1, node2,
                                                     {'object': edge_object,
                                                      'weight': edge_weight})
            else:
                print type(edge)
                raise TypeError

    def add_edges(self, edge_list, edge_weight=1):
        # Each edge (node1, node2, {'object': object})
        self.add_edges_from(edge_list, weight=edge_weight)

    def allocate_flow(self, node1, node2, flow):
        try:
            self[node1][node2]['object'].allocate_flow(flow)
            self[node1][node2]['weight'] = self[node1][node2]['object'].weight
        except:
            raise

    def free_edge(self, node1, node2, flow):
        if flow is not None:
            self[node1][node2]['object'].free_flow(flow)
            self[node1][node2]['weight'] = self[node1][node2]['object'].weight

    # Will remove ONE of the edges in Meta_edge
    def remove_edge(self, node1, node2, force=False):
        edge = self[node1][node2]['object'].remove_least_busy_edge(force)
        if edge is not None:
            node1.plug_out_edge(tx=True)
            node2.plug_out_edge(tx=False)

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

    def build_topology_from_int(self, nodes, edges, ghost_topo,
                                arrival_rate=None, service_rate=None):
        # Node identifier is _id, name is just for plots !
        # nodes -> list of int or list of (int, str) str->entry,exit
        # edges -> list of (node1, node2) or (node1, node2, capacity)
        # or (node1, node2, capacity, weight)
        # Every edge is both ways if not edge['unidir'] set or set to False

        node_list = []
        edge_list = []

        for node in nodes:
            if type(node) == int or type(node) == str:
                node = {'_id': node}
            elif type(node) == tuple or type(node) == list:
                if len(node) == 2:
                    node = {'_id': node[0], 'type': node[1]}
                else:
                    raise TypeError
            elif type(node) == dict:
                pass
            else:
                raise TypeError("Unsuported node description type, " +
                                str(type(node)))
            node_list.append(node)

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
                temp_edge['enabled'] = True
            elif type(edge) == dict:
                temp_edge = edge
            else:
                raise TypeError("Unsuported edge description type, " +
                                str(type(edge)))
            edge_list.append(temp_edge)
        self.build_topology(node_list, edge_list,
                            ghost_topo, arrival_rate, service_rate)

    def build_topology(self, nodes, edges, ghost_topo,
                       arrival_rate=None, service_rate=None,):
        temp_dict = dict()

        for node in nodes:
            if type(node) != dict:
                raise TypeError

            node_type = node.pop('type', '')
            if not '_id' in node:
                try:
                    node['_id'] = node['name']
                except KeyError:
                    raise ValueError("Invalid Node, should have an "
                                     "_id or name")

            if not 'name' in node:
                node['name'] = node['_id']
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
            if node['_id'] in temp_dict:
                raise DuplicatedNodeError
            temp_dict[node['_id']] = new_node
            self.add_node(new_node)
            ghost_topo.add_node(new_node)
        self.id_to_node.update(temp_dict)

        for edge in edges:
            if type(edge) != dict:
                raise TypeError

            try:
                nodes = edge.pop('nodes')
            except KeyError:
                print edge
            weight = edge.get('weight', 1)
            unidir = edge.pop('unidir', False)

            enabled = edge.pop('enabled')

            new_edge = Edge(**edge)

            # Dispatch enabled/disabled edges
            ghost_topo.add_edge(temp_dict[nodes[0]],
                                temp_dict[nodes[1]],
                                new_edge,
                                weight)
            if enabled:
                self.add_edge(temp_dict[nodes[0]],
                              temp_dict[nodes[1]],
                              new_edge.copy(),
                              weight)
            if not unidir:
                new_edge = Edge(**edge)
                ghost_topo.add_edge(temp_dict[nodes[1]],
                                    temp_dict[nodes[0]],
                                    new_edge,
                                    weight)
                if enabled:
                    self.add_edge(temp_dict[nodes[1]],
                                  temp_dict[nodes[0]],
                                  new_edge.copy(),
                                  weight)


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

    def swap_node_arr_rate(self, node__id, new_value=None):
        try:
            self.id_to_node[node__id].swap_arr_rate(new_value)
        except KeyError:
            raise NoSuchNode

    def get_entry_nodes(self):
        return self.entry_nodes if len(self.entry_nodes) > 0 else self.nodes()


# TODO Frozenset useless each node has exactly two edges for itself
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
    node_labels = dict([(u, u.get_name()) for u in topology.nodes_iter()])
    networkx.draw(topology, pos=pos, hold=True, with_labels=False)
    networkx.draw_networkx_labels(topology, pos, labels=node_labels)
    matplotlib.pyplot.draw()
    matplotlib.pyplot.show()
