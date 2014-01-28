from flowsim.flowsim_exception import EdgeAllocationError


class Edge(object):
    constants = {'LAST_FLOW_AVAILABLE': -1}

    def __init__(self, capacity=1, name=''):
        self.max_flows = capacity
        self.available_flows = self.max_flows
        self.passing_flows = []
        self.name = name if name != '' else str(id(self))

    def allocate_flow(self, flow):
        ret_value = 0
        # Flow_manager should not call in that case
        if self.available_flows == 0:
            raise EdgeAllocationError()
        elif self.available_flows == 1:
            ret_value = Edge.constants['LAST_FLOW_AVAILABLE']

        self.available_flows -= 1
        self.passing_flows.append(flow)
        return ret_value

    def free_flow(self, flow):
        if not (self.available_flows < self.max_flows):
            raise EdgeAllocationError()
        try:
            self.passing_flows.remove(flow)
        except ValueError:
            raise EdgeAllocationError()
        self.available_flows += 1

    def get_const_value(self, key):
        return Edge.constants[key]

    def get_name(self):
        return self.name

    def reset(self):
        self.passing_flows = []
        self.available_flows = self.max_flows
