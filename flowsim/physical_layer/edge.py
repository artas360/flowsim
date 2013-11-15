#!/usr/bin/python

class EdgeAllocationError(Exception):
    pass

class Edge(object):
    constants={'LAST_FLOW_AVAILABLE':-1}
    def __init__(self, max_flows=1):
        self.max_flows=max_flows
        self.available_flows=self.max_flows
        self.passing_flows=[]

    def allocate_flow(self, flow):
        ret_value = 0
        if self.available_flows == 0: raise EdgeAllocationError() # Flow_manager should not call in that case
        elif self.available_flows == 1: ret_value = Edge.constants['LAST_FLOW_AVAILABLE']

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
