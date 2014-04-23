class Node(object):
    def __init__(self, arrival_rate, service_rate, _id, name=None):
        self._id = _id
        self.name = name if name is not None else self._id
        self.arrival_rate = arrival_rate
        self.backup_arr_rate = arrival_rate
        self.service_rate = service_rate

    def __int__(self):
        return self._id

    def get_name(self):
        return self.name

    def get_arrival_rate(self):
        return self.arrival_rate

    def get_service_rate(self):
        return self.service_rate

    def swap_arr_rate(self, new_rate):
        if new_rate is None:
            self.arrival_rate = self.backup_arr_rate
        elif new_rate > 0:
            self.arrival_rate = new_rate
        else:
            raise ValueError


def foo_node():
    node = Node(0., 0., 0)
    return node


class Entry_node(Node):
    pass


class Exit_node(Node):
    pass
