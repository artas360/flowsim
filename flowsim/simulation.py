from flowsim.physical_layer.topology import Topology
from flowsim.flow.flow_controller import Flow_controller
from flowsim.flowsim_exception import WrongConfig
from flowsim.random_generator import Random_generator
from flowsim.event.event import Event_manager
from flowsim.result import Result
from flowsim.config import Config


class Simulation(object):
    def __init__(self, arrival_rate, service_rate, rand_seed=None):
        self.arrival_rate = arrival_rate
        self.service_rate = service_rate
        self.random_generator = None
        self.rand_seed = rand_seed
        self.result = Result()
        self.topology = None

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
            self.init_simulation(nodes, edges, event_conf, simualtion_conf)

        except IOError:
            raise
        except WrongConfig:
            raise

    def init_simulation(self, nodes, edges, user_events, simualtion_conf):
        self.init_topology(nodes, edges)
        self.init_random_generator()
        self.init_event_manager(user_events, simualtion_conf)
        self.init_flow_controller()

    def init_random_generator(self, arrival_generation_function=None,
                              duration_function=None):
        self.random_generator = Random_generator(self.topology,
                                                 self.rand_seed,
                                                 arrival_generation_function,
                                                 duration_function)

    def init_event_manager(self, user_events, simualtion_conf):
        self.event_manager = Event_manager(self,
                                           self.random_generator,
                                           user_events,
                                           simualtion_conf)

    def init_topology(self, nodes, edges):
        # nodes -> list of int
        # edges -> list of (node1, node2) or (node1, node2, capacity)
        # or (node1, node2, capacity, weight)
        self.topology = Topology()
        self.topology.build_topology_from_int(nodes,
                                              edges,
                                              self.arrival_rate,
                                              self.service_rate)

    def init_flow_controller(self):
        self.flow_controller = Flow_controller(self.topology,
                                               self.event_manager,
                                               self)
        self.event_manager.set_flow_controller(self.flow_controller)

    def launch_simulation(self):
        self.event_manager.start_event_processing()
        return self.result.get_results()
