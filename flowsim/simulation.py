from flowsim.result import Result
from flowsim.random_generator import Random_generator
from flowsim.event.event import Event_manager
from flowsim.physical_layer.topology import Topology
from flowsim.flow.flow_controller import Flow_controller
from flowsim.result import Result
from flowsim.config import Config
from flowsim.flowsim_exception import WrongConfig


class Simulation(object):
    def __init__(self, arrival_rate, service_rate, rand_seed=None):
        self.arrival_rate = arrival_rate
        self.service_rate = service_rate
        self.max_arrivals = float('inf')
        self.random_generator = None
        self.rand_seed = rand_seed
        self.result = Result()
        self.topology = None

    def copy(self):
        print("copy(): Broken functionality, to be removed!")
        sim = Simulation(self.arrival_rate, self.service_rate, self.rand_seed)
        sim.topology = self.topology.copy()\
            if self.topology is not None else None
        sim.init_random_generator()
        sim.init_event_manager()
        sim.init_flow_controller()
        return sim

    # Process the conf the local object (not event, topo ...)
    def process_conf(self, param):
        pass

    def load_conf(self, filename):
        try:
            self.config = Config(open(filename))
            self.config.read()
            simualtion_conf = self.config.read_simulation()
            nodes, edges = self.config.read_topology()
            event_conf = self.config.read_events()

            self.process_conf(simualtion_conf)
            self.init_simulation(nodes, edges, event_conf)
            
        except IOError:
            raise
        except WrongConfig:
            raise

    def init_simulation(self, nodes, edges, user_events=[]):
        self.init_topology(nodes, edges)
        self.init_random_generator()
        self.init_event_manager(user_events)
        self.init_flow_controller()

    def init_random_generator(self, arrival_generation_function=None,
                              duration_function=None):
        self.random_generator = Random_generator(self.topology,
                                                 self.rand_seed,
                                                 arrival_generation_function,
                                                 duration_function)

    def init_event_manager(self, user_events=[]):
        self.event_manager = Event_manager(self, self.random_generator, user_events)

    def init_topology(self, nodes, edges):
        # nodes -> list of int
        # edges -> list of (node1, node2) or (node1, node2, capacity)
        # or (node1, node2, capacity, weight)
        self.topology = Topology()
        self.topology.build_topology_from_int(nodes,
                                              edges,
                                              self.arrival_rate,
                                              self.service_rate)

    def import_topology(self, filename):
        self.topology = Topology()
        self.topology.import_topology(filename)

    def init_flow_controller(self):
        self.flow_controller = Flow_controller(self.topology,
                                               self.event_manager,
                                               self)
        self.event_manager.set_flow_controller(self.flow_controller)

    def end(self):
        self.max_arrivals -= 1
        if self.max_arrivals <= 0:
            return True
        return False

    def launch_simulation(self, max_arrivals=float('inf')):
        self.max_arrivals = max_arrivals
        self.event_manager.start_event_processing()
        return self.result.get_results()

    def reset(self, arrival_rate=None, service_rate=None):
        print("reset(): Broken functionality, to be removed!")
        self.topology.reset(arrival_rate, service_rate)
        self.init_random_generator()
        self.init_event_manager()
        self.init_flow_controller()
