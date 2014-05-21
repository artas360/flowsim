from networkx import DiGraph
from networkx import all_shortest_paths as _all_shortest_paths


class Ghost_topology(DiGraph):

    def __init__(self):
        super(self.__class__, self).__init__()

    def get_edge(self, src, dst):
        if self.has_edge(src, dst):
            return self[src][dst]
        else:
            return None

    def add_edge(self, node1, node2, edge_obj, weight):
        super(self.__class__, self).add_edge(node1, node2,
                                             {'weight': weight,
                                              'object':edge_obj})

    def all_shortest_paths(self, src, dst):
        return [p for p in _all_shortest_paths(self, src, dst, weight="weight")]


class Foo_ghost_topology(object):

    def get_edge(self, src, dst):
        return None

    def add_edge(self, node1, node2, edge_obj, weight):
        pass

    def add_node(self, node):
        pass

    def all_shortest_paths(self, src, dst):
        # We expect src->dst[weight] = infinity
        # Graph fully connected so [src, x, dst] for x in nodes
        return [[src, x, dst] for x in self.nodes_iter()
                if x != src and x!= dst]
