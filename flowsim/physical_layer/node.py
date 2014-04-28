from flowsim.flowsim_exception import EdgePlugInError
from flowsim.flowsim_exception import EdgePlugOutError


class Node(object):
    def __init__(self, arrival_rate, service_rate, _id, name=None,
                 tx_slot=2, rx_slot=2):
        self._id = _id
        self.name = name if name is not None else self._id
        self.arrival_rate = arrival_rate
        self.backup_arr_rate = arrival_rate
        self.service_rate = service_rate
        self.tx_slot = tx_slot
        self.tx_slot_max = tx_slot
        self.rx_slot = rx_slot
        self.rx_slot_max = rx_slot

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

    def plug_in_edge(self, tx=True):
        if tx:
            if(self.tx_slot <= 0):
                raise EdgePlugInError
            self.tx_slot -= 1
        else:
            if(self.rx_slot <= 0):
                raise EdgePlugInError
            self.rx_slot -= 1

    def plug_out_edge(self, tx=True):
        if tx:
            if(self.tx_slot >= self.tx_slot_max):
                raise EdgePlugOutError
            self.tx_slot += 1
        else:
            if(self.rx_slot >= self.rx_slot_max):
                raise EdgePlugOutError
            self.rx_slot += 1



def foo_node():
    node = Node(0., 0., 0)
    return node


class Entry_node(Node):
    pass


class Exit_node(Node):
    pass
