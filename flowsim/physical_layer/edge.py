from flowsim.flowsim_exception import EdgeAllocationError


class Edge(object):
    infinite_weight = float("inf")

    def __init__(self, capacity=1, name='', weight=1., enabled=True):
        self.max_flows = capacity
        self.available_flows = self.max_flows
        self.passing_flows = []
        self.name = name if name != '' else str(id(self))
        self.weight = weight
        self.weight_backup = self.weight
        self.enabled = enabled
        if(not enabled):
            self.disable()

    def allocate_flow(self, flow):
        # Flow_manager should not call in that case
        if self.available_flows == 0:
            raise EdgeAllocationError()
        elif self.available_flows == 1:
            self.weight = Edge.infinite_weight

        self.available_flows -= 1
        self.passing_flows.append(flow)

    def free_flow(self, flow):
        if not (self.available_flows < self.max_flows):
            raise EdgeAllocationError()
        try:
            self.passing_flows.remove(flow)
        except ValueError:
            raise EdgeAllocationError()
        self.available_flows += 1
        # No need to check since backup always != inf
        self.weight = self.weight_backup

    def enable(self):
        if not self.enabled:
            self.weight = self.weight_backup

    def disable(self):
        if len(self.passing_flows) > 0:
            raise EdgeAllocationError
        if self.enabled:
            self.weight_backup = self.weight
            self.weight = Edge.infinite_weight

    def get_name(self):
        return self.name


class Meta_edge(object):  # Interface for mutiple edges between 2 nodes
    def __init__(self, edge):
        self.edge_list = [edge]
        self.weight = edge.weight
        self.flow_to_edge = dict()

    def add_edge(self, edge):
        assert(edge not in self.edge_list)
        self.edge_list.append(edge)
        self.weight = min(self.weight, edge.weight)

    def remove_least_busy_edge(self, force):
        # Try to find the least busy Edge
        selected_edge = min(self.edge_list, key=lambda x: x.passing_flows)
        if not force and selected_edge.passing_flows > 0:
            return None
        else:
            self.remove_edge(selected_edge)
            return edge

    def remove_edge(self, edge):
        try:
            self.edge_list.remove(edge)
        except ValueError:
            raise EdgeAllocationError
        if(len(self.edge_list) >= 1):
            self.weight = min(self.edge_list, key=lambda x: x.weight).weight
            return (len(self.edge_list))
        else:
            # This link should be removed from the topo anyway...
            self.weight = Edge.infinite_weight
            return 0

    def allocate_flow(self, flow):
        try:
            edge = min(self.edge_list, key=lambda x: x.weight)
            edge.allocate_flow(flow)
        except EdgeAllocationError():
            raise
        else:
            self.flow_to_edge[flow] = edge
            self.weight = min(self.edge_list, key=lambda x: x.weight).weight

    def free_flow(self, flow):
        try:
            edge = self.flow_to_edge.pop(flow)
        except KeyError:
            raise EdgeAllocationError
        edge.free_flow(flow)
        self.weight = min(self.edge_list, key=lambda x: x.weight).weight
