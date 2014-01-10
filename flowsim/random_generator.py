from random import expovariate, randint, seed
from flowsim.flowsim_exception import LoopError


class Random_generator(object):
    max_int = 10000

    def __init__(self, topology, rand_seed=None,
                 arrival_generation_function=None, duration_function=None):
        # TODO change this
        self.next_arrival_func = arrival_generation_function if\
            arrival_generation_function is not None else expovariate
        self.duration_function = duration_function if\
            duration_function is not None else expovariate
        self.topology = topology
        seed(rand_seed)

    def next_arrival(self, arrival_rate):
        return self.next_arrival_func(arrival_rate)

    def rand_duration(self, service_rate):
        return self.duration_function(service_rate)

    def randint(self, minimum=0):
        return randint(minimum, self.__class__.max_int)

    def random_io_nodes(self):
        prevent_loop = 10
        nodes = (None, None)
        while nodes[0] == nodes[1] and prevent_loop > 0:
            nodes = (self.topology.get_random_entry_node(self.randint()),
                     self.topology.get_random_exit_node(self.randint()))
            prevent_loop -= 1

        if prevent_loop <= 0:
            raise LoopError

        return nodes

    def random_exit_node(self, different_from=None):
        prevent_loop = 100
        node = different_from
        while node == different_from and prevent_loop > 0:
            node = self.topology.get_random_exit_node(self.randint())
            prevent_loop -= 1

        if prevent_loop <= 0:
            raise LoopError

        return node
