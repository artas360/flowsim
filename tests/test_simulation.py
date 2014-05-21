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
        n = [{'_id': 0, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 1,
              'tx_slot': 1},
             {'_id': 1, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 1,
              'tx_slot': 1},
             {'_id': 2, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 2,
              'tx_slot': 2}]
        edges = [(1, 2), (2, 0)]

        sim.init_topology(n, edges, False)

    def test_init_flow_controller(self):
        sim = Simulation(0.2, 0.9, 21312)
        sim.init_event_manager([], {})
        n = [{'_id': 0, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 1,
              'tx_slot': 1},
             {'_id': 1, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 1,
              'tx_slot': 1},
             {'_id': 2, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 2,
              'tx_slot': 2}]
        edges = [(1, 2), (2, 0)]
        sim.init_topology(n, edges, False)

        sim.init_flow_controller()

    def test_init_simulation(self):
        sim = Simulation(0.2, 0.9, 21312)
        n = [{'_id': 0, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 1,
              'tx_slot': 1},
             {'_id': 1, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 1,
              'tx_slot': 1},
             {'_id': 2, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 2,
              'tx_slot': 2}]
        edges = [(1, 2), (2, 0)]
        sim.init_simulation(n, edges, False, [], {})

    def test_launch_simulation(self):
        sim = Simulation(0.1, 0.9, 21312)
        n = [{'_id': 0, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 1,
              'tx_slot': 1},
             {'_id': 1, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 1,
              'tx_slot': 1},
             {'_id': 2, 'arrival_rate': .1, 'service_rate': .9, 'rx_slot': 2,
              'tx_slot': 2}]
                
        edges = [(1, 2), (2, 0)]
        sim.init_simulation(n, edges, False, [], {})

        sim.launch_simulation()

    def test_launch_simulation2(self):
        sim = Simulation(0.5, 0.5, 21312)
        n = [{'_id': 0, 'arrival_rate': .5, 'service_rate': .5, 'rx_slot': 3,
              'tx_slot': 3},
             {'_id': 1, 'arrival_rate': .5, 'service_rate': .5, 'rx_slot': 2,
              'tx_slot': 2},
             {'_id': 2, 'arrival_rate': .5, 'service_rate': .5, 'rx_slot': 4,
              'tx_slot': 4},
             {'_id': 3, 'arrival_rate': .5, 'service_rate': .5, 'rx_slot': 2,
              'tx_slot': 2},
             {'_id': 4, 'arrival_rate': .5, 'service_rate': .5, 'rx_slot': 2,
              'tx_slot': 2},
             {'_id': 5, 'arrival_rate': .5, 'service_rate': .5, 'rx_slot': 1,
              'tx_slot': 1}]
        edges = [(n[0]['_id'], n[3]['_id']),
                 (n[0]['_id'], n[1]['_id']),
                 (n[0]['_id'], n[2]['_id']),
                 (n[1]['_id'], n[2]['_id']),
                 (n[3]['_id'], n[2]['_id']),
                 (n[4]['_id'], n[2]['_id']),
                 (n[4]['_id'], n[5]['_id'])]

        sim.init_simulation(n, edges, False, [], {})

        sim.launch_simulation()

    def test_simulation_file_config(self):
        sim = Simulation(0.8, 0.9)
        sim.load_conf("../config/config-sample.xml")
        res = sim.launch_simulation()
        # TODO check config propagation

    def test_result_simulation(self):
        sim = Simulation(0.9, 0.9)
        sim.init_simulation([0, 1], [(0, 1)], False, [], {})
        res = sim.launch_simulation()['latest']["general"]
        assert (abs(res['Blocking_rate'] - 0.5) < 0.05)

    def test_result_simulation2(self):
        sim = Simulation(0.9, 0.9)
        nodes = [0, 1, 2]
        edges = [(0, 1), (1, 2), (2, 0)]

        sim.init_simulation(nodes, edges, False, [], {})
        res = sim.launch_simulation()['latest']["general"]
        # Is it realy the expected result?
        assert (abs(res['Blocking_rate'] - 0.29) < 0.05)

    def test_reconfigure(self):
        sim = Simulation(0.8, 0.9)
        sim.load_conf('..\config\\reoptimization-config.xml')
        res = sim.launch_simulation()

