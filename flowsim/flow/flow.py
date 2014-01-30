class Flow(object):

    def __init__(self, node_list):
        self.node_list = node_list

    def get_nodes(self):
        return self.node_list

    def length(self):  # length in nodes not edges
        return len(self.node_list)
