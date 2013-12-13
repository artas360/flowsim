#!/usr/bin/python

from flowsim.result import Result
from flowsim.random_generator import Random_generator
from flowsim.event.event import Event_manager
from flowsim.physical_layer.topology import Topology
from flowsim.flow.flow_controller import Flow_controller
from flowsim.result import Result


class Simulation(object):
    def __init__(self, arrival_rate, duration_rate, rand_seed=None):
        self.arrival_rate = arrival_rate
        self.duration_rate = duration_rate
        self.rand_seed=rand_seed
        self.result = Result()
        self.random_generator=None
        self.max_event=float('inf')

    def init_simulation(self, nodes, edges):
        self.init_topology(nodes, edges)
        self.init_random_generator()
        self.init_event_manager()
        self.init_flow_controller()

    def init_random_generator(self, arrival_generation_function=None, duration_function=None):
        self.random_generator = Random_generator(self.topology, self.arrival_rate, self.duration_rate, self.rand_seed, arrival_generation_function, duration_function)

    def init_event_manager(self):
        self.event_manager = Event_manager(self, self.random_generator)

    def init_topology(self, nodes, edges):
        # nodes -> list of int
        # edges -> list of (node1, node2) or (node1, node2, capacity)
        # or (node1, node2, capacity, weight)
        self.topology = Topology()
        self.topology.build_topology_from_int(nodes, edges)

    def import_topology(self, filename):
        self.topology = Topology()
        self.topology.import_topology(filename)

    def init_flow_controller(self):
        self.flow_controller = Flow_controller(self.topology, self.event_manager, self)
        self.event_manager.set_flow_controller(self.flow_controller)

    def end(self):
        self.max_event -= 1
        if self.max_event <= 0:
            return True
        return False

    def launch_simulation(self, max_event=float('inf')):
        self.max_event=max_event
        self.event_manager.start_event_processing()


