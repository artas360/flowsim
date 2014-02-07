import unittest
from flowsim import Simulation


class Test_Simulation(unittest.TestCase):

    def test_constructor(self):
        sim = Simulation(0.2, 0.9)
        sim = Simulation(0.2, 0.9, 21312)

    def test_copy(self):
        sim = Simulation(0.2, 0.9, 21312)
        sim.init_simulation([0, 1], [(0, 1)])
        sim2 = sim.copy()
        assert (not sim is sim2)
        assert (sim.arrival_rate == sim2.arrival_rate and
                sim.service_rate == sim2.service_rate and
                sim.rand_seed == sim2.rand_seed and
                not sim.topology is sim2.topology)

    def test_init_event_manager(self):
        sim = Simulation(0.2, 0.9, 21312)

        sim.init_event_manager()

    def test_init_topology(self):
        sim = Simulation(0.2, 0.9, 21312)
        nodes = range(4)
        edges = [(1, 2), (2, 0)]

        sim.init_topology(nodes, edges)

    def test_init_flow_controller(self):
        sim = Simulation(0.2, 0.9, 21312)
        sim.init_event_manager()
        nodes = range(4)
        edges = [(1, 2), (2, 0)]
        sim.init_topology(nodes, edges)

        sim.init_flow_controller()

    def test_init_simulation(self):
        sim = Simulation(0.2, 0.9, 21312)
        nodes = range(4)
        edges = [(1, 2), (2, 0)]
        sim.init_simulation(nodes, edges)

    def test_launch_simulation(self):
        sim = Simulation(0.1, 0.9, 21312)
        nodes = range(4)
        edges = [(1, 2), (2, 0)]
        sim.init_simulation(nodes, edges)

        sim.launch_simulation(50)

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

        sim.init_simulation(nodes, edges)

        sim.launch_simulation()

    def test_result_simulation(self):
        sim = Simulation(0.9, 0.9)
        sim.init_simulation([0, 1], [(0, 1)])
        res = sim.launch_simulation()
        assert (abs(res['Blocking_rate'] - 0.5) < 0.05)

    def test_result_simulation2(self):
        sim = Simulation(0.9, 0.9)
        nodes = [0, 1, 2]
        edges = [(0, 1), (1, 2), (2, 0)]

        sim.init_simulation(nodes, edges)
        res = sim.launch_simulation()
        # Is it realy the expected result?
        assert (abs(res['Blocking_rate'] - 0.29) < 0.05)

    def test_reset_simulation(self):
        sim = Simulation(0.9, 0.9)
        nodes = [0, 1, 2]
        edges = [(0, 1), (1, 2), (2, 0)]

        sim.init_simulation(nodes, edges)
        res = sim.launch_simulation()

        sim.reset()
        sim.reset(0.5, 0.4)
