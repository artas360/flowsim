
class RessourceAllocationError(Exception):
    def __init__(self):
        super(self.__class__, self).__init__()

class Edge(object):
    def __init__(self, max_flows=1):
        self.max_flows=max_flows
        self.available_flows=self.max_flows
        self.passing_flows=[]

    def allocate_flow(self, flow):
        #TODO : return value or exception??
        if self.available_flows == 0: raise RessourceAllocationError()

        self.available_flows -= 1
        self.passing_flows.append(flow)

    def free_flow(self, flow):
        assert(self.available_flows < self.max_flows)
        self.available_flows += 1
        try:
            self.passing_flows.remove(flow)
        except ValueError:
            raise RessourceAllocationError()
