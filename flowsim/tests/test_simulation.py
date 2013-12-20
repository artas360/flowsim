import unittest
from flowsim import Simulation

class Test_Simulation(unittest.TestCase):

    def test_constructor(self):
        sim = Simulation(0.2, 0.9)
        sim = Simulation(0.2, 0.9, 21312)

    def test_init_event_manager(self):
        sim = Simulation(0.2, 0.9, 21312)

        sim.init_event_manager()

    def test_init_topology(self):
        sim = Simulation(0.2, 0.9, 21312)
        nodes = range(4)
        edges=[(1,2),(2,0)]

        sim.init_topology(nodes, edges)

    def test_init_flow_controller(self):
        sim = Simulation(0.2, 0.9, 21312)
        sim.init_event_manager()
        nodes = range(4)
        edges=[(1,2),(2,0)]
        sim.init_topology(nodes, edges)

        sim.init_flow_controller()

    def test_init_simulation(self):
        sim = Simulation(0.2, 0.9, 21312)
        nodes = range(4)
        edges=[(1,2),(2,0)]
        sim.init_simulation(nodes, edges)

    def test_launch_simulation(self):
        sim = Simulation(0.1, 0.9, 21312)
        nodes = range(4)
        edges=[(1,2),(2,0)]
        sim.init_simulation(nodes, edges)

        sim.launch_simulation(50)
        
        print sim.event_manager.event_list

    def test_launch_simulation2(self):
        print 'cplxe topo'
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

        sim.launch_simulation(50)
