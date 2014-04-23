from unittest import TestCase

from flowsim.simulation import Simulation


class Test_Simulation(TestCase):

    def test_constructor(self):
        sim = Simulation(0.2, 0.9)
        sim = Simulation(0.2, 0.9, 21312)

    def test_init_event_manager(self):
        sim = Simulation(0.2, 0.9, 21312)

        sim.init_event_manager([], {})

    def test_init_topology(self):
        sim = Simulation(0.2, 0.9, 21312)
        nodes = range(4)
        edges = [(1, 2), (2, 0)]

        sim.init_topology(nodes, edges)

    def test_init_flow_controller(self):
        sim = Simulation(0.2, 0.9, 21312)
        sim.init_event_manager([], {})
        nodes = range(4)
        edges = [(1, 2), (2, 0)]
        sim.init_topology(nodes, edges)

        sim.init_flow_controller()

    def test_init_simulation(self):
        sim = Simulation(0.2, 0.9, 21312)
        nodes = range(4)
        edges = [(1, 2), (2, 0)]
        sim.init_simulation(nodes, edges, [], {})

    def test_launch_simulation(self):
        sim = Simulation(0.1, 0.9, 21312)
        nodes = range(4)
        edges = [(1, 2), (2, 0)]
        sim.init_simulation(nodes, edges, [], {})

        sim.launch_simulation()

    def test_launch_simulation2(self):
        sim = Simulation(0.5, 0.5, 21312)
        nodes = range(6)
        edges = [(nodes[0], nodes[3]),
                 (nodes[0], nodes[1]),
                 (nodes[0], nodes[2]),
                 (nodes[1], nodes[2]),
                 (nodes[3], nodes[2]),
                 (nodes[4], nodes[2]),
                 (nodes[4], nodes[5])]

        sim.init_simulation(nodes, edges, [], {})

        sim.launch_simulation()

    def test_simulation_file_config(self):
        sim = Simulation(0.8, 0.9)
        sim.load_conf("../config/config-sample.xml")
        res = sim.launch_simulation()
        # TODO check config propagation

    def test_result_simulation(self):
        sim = Simulation(0.9, 0.9)
        sim.init_simulation([0, 1], [(0, 1)], [], {})
        res = sim.launch_simulation()['latest']["general"]
        assert (abs(res['Blocking_rate'] - 0.5) < 0.05)

    def test_result_simulation2(self):
        sim = Simulation(0.9, 0.9)
        nodes = [0, 1, 2]
        edges = [(0, 1), (1, 2), (2, 0)]

        sim.init_simulation(nodes, edges, [], {})
        res = sim.launch_simulation()['latest']["general"]
        # Is it realy the expected result?
        assert (abs(res['Blocking_rate'] - 0.29) < 0.05)
