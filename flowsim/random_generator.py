from random import expovariate, randint, seed


class LoopError(Exception):
    pass


class Random_generator(object):
    max_int = 10000
    def __init__(self, topology, arrival_rate, duration_rate, rand_seed = None, arrival_generation_function=None, duration_function=None):
        self.arrival_rate = arrival_rate
        self.duration_rate = duration_rate
        # TODO change this
        self.next_arrival_func = arrival_generation_function if arrival_generation_function != None else expovariate
        self.duration_function = duration_function if duration_function != None else expovariate
        self.topology=topology
        seed(rand_seed)

    def next_arrival(self):
        return self.next_arrival_func(self.arrival_rate)

    def rand_duration(self):
        return self.duration_function(self.duration_rate)

    def randint(self, minimum=0):
        return randint(minimum, self.__class__.max_int)

    def random_io_nodes(self):
        prevent_loop = 10
        nodes=(None,None)
        while nodes[0] == nodes[1] and prevent_loop > 0:
            nodes = (self.topology.get_random_entry_node(self.randint()), self.topology.get_random_exit_node(self.randint()))
            prevent_loop -= 1

        if prevent_loop <= 0:
            raise LoopError

        return nodes
