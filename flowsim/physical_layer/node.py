class Node(object):
    counter = 0

    def __init__(self, arrival_rate, service_rate, number=-1, name=''):
        self.number = number if number >= 0 else Node.counter
        self.name = name if name != '' else str(id(self))
        Node.counter = self.number + 1

        self.arrival_rate = arrival_rate
        self.service_rate = service_rate

    def copy(self):
        node = foo_node()
        node.number = self.number
        node.name = self.name
        node.arrival_rate = self.arrival_rate
        node.service_rate = self.service_rate
        return node

    def __int__(self):
        return self.number

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
    tmp_counter = Node.counter
    node = Node(0., 0.)
    Node.counter = tmp_counter
    return node


class Entry_node(Node):
    pass


class Exit_node(Node):
    pass
