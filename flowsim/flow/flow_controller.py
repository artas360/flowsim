#!/usr/bin/python

import flow as flow_module

class NoPathError(Exception):
    pass

class NotRegisteredFlow(Exception):
    pass

# TODO : list of input nodes, output nodes somewhere
class Flow_controller(object):
    def __init__(self, topology, event_manager, simulation):
        self.topology = topology
        self.event_manager = event_manager
        self.simulation = simulation

        self.flows = []

    def allocate_flow(self, node1, node2):
        #TODO : have to loop
        nodes = self.topology.shortest_path(node1, node2)
        if nodes == []:
            raise NoPathError()
        edges=[self.topology.get_edge_object(nodes[i],nodes[i+1]) for i in xrange(len(nodes)-1)]
        flow = flow_module.Flow(edges)
        try:
            for edge in edges:
                ret=edge.allocate_flow(flow)
                if ret == edge.get_const_value('LAST_FLOW_AVAILABLE'):
                    self.topology.set_edge_unavailable(node[i], node[i+1])
        # If we except the node should be unavailable and we need to free the first allocations of the flow
        except flow_module.RessourceAllocationError:
            self.topology.set_edge_unavailable(node[i], node[i+1])
            for edge in edges[:i-1]:
                edge.free_flow()
        else:
            self.flows.append(flow)

    def free_flow(self, flow):
        try:
            self.flows.remove(flow)
        except:
            raise NotRegisteredFlow()
        for edge in flow.get_edges():
            topology.free_edge(edge, flow)

    def handle_flow_allocation_failure(self, source_node, dest_node):
        self.event_manager.flow_allocation_failure(source_node, dest_node)
