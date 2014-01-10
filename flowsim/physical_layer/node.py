class Node(object):
    counter = 0

    def __init__(self, arrival_rate, service_rate, number=-1, name=''):
        self.number = number if number >= 0 else Node.counter
        self.name = name if name != '' else str(id(self))
        Node.counter = self.number + 1

        self.arrival_rate = arrival_rate
        self.service_rate = service_rate

    def __int__(self):
        return self.number

    def get_name(self):
        return self.name

    def get_arrival_rate(self):
        return self.arrival_rate

    def get_service_rate(self):
        return self.service_rate


class Entry_node(Node):
    pass


class Exit_node(Node):
    pass
