#!/usr/bin/python

import networkx
from edge import Edge
from node import Node


class NoSuchEdge(Exception):

    pass


class Foo_edge(object):

    pass


class Topology(networkx.Graph):

    def __init__(self):
        super(self.__class__, self).__init__()
        self.edges_unavailable=dict()
        self.entry_nodes=[]
        self.exit_nodes=[]
    
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
        try:
            edge=self[node1][node2] # it's a dict() ! Keeping it all
            self.edges_unavailable[(node1, node2)] = edge
            self.remove_edge(node1, node2)
        except KeyError:
            raise NoSuchEdge()

    def free_edge(self, edge, flow):
        try:
            (node1, node2)=self.edges_unavailable.keys()[[x['object'] for x in self.edges_unavailable.values()].index(edge)]
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
        self.add_edge(node1, node2, edge['object'], edge['weight'])
        self[node1][node2] = edge

    def get_edge_object(self, node1, node2):
        return self[node1][node2]['object']

    def shortest_path(self, node1, node2):
        return networkx.shortest_path(self, node1, node2, weight='weight')

    def build_topology_from_int(self, nodes, edges):
        # nodes -> list of int or list of (int, str) str->entry,exit
        # edges -> list of (node1, node2) or (node1, node2, capacity)
        # or (node1, node2, capacity, weight)
        temp_dict=dict()

        # nodes.sort()
        for node in nodes:
            if type(node) == int:
                new_node=Node(node)
                temp_dict[node] = new_node
                self.add_node(new_node)
            elif len(node) == 2:
                new_node=Node(node[0])
                self.add_node(new_node)
                temp_dict[node[0]] = new_node
                if node[1] == 'entry':
                    self.entry_nodes.append(new_node)
                elif node[1] == 'exit':
                    self.exit_nodes.append(new_node)

        for edge in edges:
            if len(edge) == 2:
                self.add_edge(temp_dict[edge[0]], temp_dict[edge[1]], Edge())
            elif len(edge) == 3:
                self.add_edge(temp_dict[edge[0]], temp_dict[edge[1]], Edge(edge[2]))
            elif len(edge) == 4:
                self.add_edge(temp_dict[edge[0]], temp_dict[edge[1]], Edge(edge[2]), edge[3])
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

    def import_topology(self, filename):
        # TODO
        #detect if node is entry or exit
        raise NotImplemented()

    def draw_graph(self):
        # TODO
        raise NotImplemented()
