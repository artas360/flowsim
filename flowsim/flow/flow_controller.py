#!/usr/bin/python

import flow as flow_module


class NoPathError(Exception):
    pass


class NotRegisteredFlow(Exception):
    pass


class RessourceAllocationError(Exception):
    pass


# TODO : list of input nodes, output nodes somewhere
class Flow_controller(object):
    def __init__(self, topology, event_manager, simulation):
        self.topology = topology
        self.event_manager = event_manager
        self.simulation = simulation

        self.flows = []

    def allocate_flow(self, node1, node2):
        if node1 == None or node2 == None:
            raise TypeError
        #TODO : have to loop
        try:
            nodes = self.topology.shortest_path(node1, node2)
            #print 'nodes in flow ', map(int,nodes)
            #print 'Nodes, before edges', [(int(nodes[i]), int(nodes[i+1])) for i in xrange(len(nodes)-1)]
        except:
            raise NoPathError()
        edges=[self.topology.get_edge_object(nodes[i], nodes[i+1]) for i in xrange(len(nodes)-1)]
        flow = flow_module.Flow(edges)
        try:
            for edge in edges:
                ret=edge.allocate_flow(flow)
                if ret == edge.get_const_value('LAST_FLOW_AVAILABLE'):
                    # TODO useless we already know i
                    i = edges.index(edge)
                    self.topology.set_edge_unavailable(nodes[i], nodes[i+1])
        # If we except the node should be unavailable and we need to free the first allocations of the flow
        except RessourceAllocationError:
            self.topology.set_edge_unavailable(node[i], node[i+1])
            for edge in edges[:i-1]:
                edge.free_flow()
        else:
            self.flows.append(flow)
            return flow

    def free_flow(self, flow):
        try:
            self.flows.remove(flow)
        except:
            raise NotRegisteredFlow()
        for edge in flow.get_edges():
            self.topology.free_edge(edge, flow)

    def handle_flow_allocation_failure(self, source_node, dest_node):
        self.event_manager.flow_allocation_failure(source_node, dest_node)

