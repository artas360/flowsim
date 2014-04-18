class Node(object):
    def __init__(self, arrival_rate, service_rate, _id, name=None):
        self._id = _id
        self.name = name if name is not None else self._id
        self.arrival_rate = arrival_rate
        self.service_rate = service_rate

    def copy(self):
        node = foo_node()
        node._id = self._id
        node.name = self.name
        node.arrival_rate = self.arrival_rate
        node.service_rate = self.service_rate
        return node

    def __int__(self):
        return self._id

    def get_name(self):
        return self.name

    def get_arrival_rate(self):
        return self.arrival_rate

    def get_service_rate(self):
        return self.service_rate

    def reset(self, arrival_rate=None, service_rate=None):
        if arrival_rate is not None:
            self.arrival_rate = arrival_rate
        if service_rate is not None:
            self.service_rate = service_rate


def foo_node():
    node = Node(0., 0., 0)
    return node


class Entry_node(Node):
    pass


class Exit_node(Node):
    pass
