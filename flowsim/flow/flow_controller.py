from flowsim.flow.flow import Flow
from flowsim.flowsim_exception import NoPathError,\
    NotRegisteredFlow,\
    RessourceAllocationError


class Flow_controller(object):
    def __init__(self, topology, event_manager, simulation):
        self.topology = topology
        self.flows = []

    def allocate_flow(self, node1, node2):
        if node1 is None or node2 is None:
            raise TypeError
        try:
            nodes = self.topology.shortest_path(node1, node2)
        except NoPathError:
            raise NoPathError()
        flow = Flow(nodes)

        for i in xrange(len(nodes)-1):
            self.topology.allocate_flow(nodes[i], nodes[i+1], flow)

        self.flows.append(flow)
        return flow

    def free_flow(self, flow):
        try:
            self.flows.remove(flow)
        except:
            raise NotRegisteredFlow()
        nodes = flow.get_nodes()
        for i in xrange(len(nodes) - 1):
            self.topology.free_edge(nodes[i], nodes[i + 1], flow)

    def get_entry_nodes(self):
        return self.topology.get_entry_nodes()

    def get_topology(self):
        return self.topology

    def reconfigure_topology(self, results, algorithm=""):
        # TODO: switch on algorithm
        pass
