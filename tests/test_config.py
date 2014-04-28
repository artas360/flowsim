from unittest import TestCase, skip

from xml.parsers.expat import ExpatError
from flowsim.config import Config
from tempfile import TemporaryFile


class Test_config(TestCase):
    def setUp(self):
        self.conf = TemporaryFile()
        self.config = Config(self.conf)
        self.conf.Rclose = self.conf.close
        self.conf.close = lambda: None

    def tearDown(self):
        self.conf.Rclose()

    def dump_conf(self, conf):
        self.conf.seek(0, 0)
        self.conf.write(''.join(conf))
        self.conf.truncate()
        self.conf.flush()
        self.conf.seek(0, 0)

    def test_openXML(self):
        conf = ['<?xml version="1.0" encoding="UTF-8"?>',
                '<Flowsim datetime="2014-04-15 10:09" version="develop">',
                '<Topology>',
                '<Nodes>',
                ('<Node id="0" name="node85" arrival_rate=".3" '
                 'service_rate=".4"/>'),
                ('<Node id="1" name="node86" arrival_rate=".3" '
                 'service_rate=".5"/>'),
                '</Nodes>',
                '</Topology>',
                '</Flowsim>']
        self.dump_conf(conf)

    @skip("Parser doesn't support DTD")
    def test_openXML_DTD(self):
        conf = ['<?xml version="1.0" encoding="UTF-8"?>',
                '<!DOCTYPE Flowsim SYSTEM "../Flowsim_config.dtd">',
                '<Flowsim datetime="2014-04-15 10:09" version="test">',
                '<Topology>',
                '<Nodes>',
                ('<Node id="0" name="node85" arrival_rate=".3" '
                 'service_rate=".4"/>'),
                ('<Node id="0" name="node86" arrival_rate=".3" '
                 'service_rate=".5"/>'),
                '</Nodes>',
                '</Topology>',
                '</Flowsim>']

        # Same ID twice
        self.dump_conf(conf)
        self.assertRaises(self.config.read(), ExpatError)

        # TODO test same node id, wrong node idrefin link...

    def test_read_topology(self):
        conf = ['<?xml version="1.0" encoding="UTF-8"?>',
                '<Flowsim datetime="2014-04-15 10:09" version="test">',
                '<Topology>',
                '<Nodes>',
                '<Default name="" rx_slot="1" tx_slot="1"/>',
                ('<Node id="0" name="node1" arrival_rate=".3" '
                 'service_rate=".4"/>'),
                ('<Node id="1" name="node2" arrival_rate=".3" '
                 'service_rate=".5"/>'),
                '</Nodes>',
                '<Links>',
                '<Default weight="1." capacity="1" unidirectional="True"/>',
                ('<Link source_id="0" destination_id="1" weight="1" '
                 'capacity="1" unidirectional="True"/>'),
                ('<Link source_id="1" destination_id="0" weight="9" '
                 'capacity="2" unidirectional="False"/>'),
                '</Links>',
                '</Topology>',
                '</Flowsim>']
        self.dump_conf(conf)
        self.config.read()
        nodes, links = self.config.read_topology()
        node1 = {"_id": 0,
                 "name": "node1",
                 "arrival_rate": .3,
                 "service_rate": .4,
                 "tx_slot": 1,
                 "rx_slot": 1}
        node2 = {"_id": 1,
                 "name": "node2",
                 "arrival_rate": .3,
                 "service_rate": .5,
                 "tx_slot": 1,
                 "rx_slot": 1}
        self.assertTrue(node1 in nodes and node2 in nodes)
        link1 = {"nodes": (0, 1), "weight": 1., "capacity": 1, "unidir": True}
        link2 = {"nodes": (1, 0), "weight": 9., "capacity": 2, "unidir": False}
        self.assertTrue(link1 in links and link2 in links)

    def test_read_events(self):
        conf = ['<?xml version="1.0" encoding="UTF-8"?>',
                '<Flowsim datetime="2014-04-15 10:09" version="test">',
                '<Events>',
                ('<Event type="Arrival_burst" event_target="1" '
                 'trigger_type="time" trigger_value="5"/>'),
                '</Events>',
                '</Flowsim>']
        self.dump_conf(conf)
        self.config.read()
        res = self.config.read_events()
        self.assertTrue(len(res) == 1)
        event1 = {'type': 'Arrival_burst',
                  'event_target': '1',
                  'trigger_type': 'time',
                  'trigger_value': '5'}
        self.assertTrue(event1 == res[0])

    def test_read_simulation(self):
        conf = ['<?xml version="1.0" encoding="UTF-8"?>',
                '<Flowsim datetime="2014-04-15 10:09" version="test">',
                '<Simulation>',
                ('<Convergence number_samples="15" epsilon="1e-3" '
                 'check_interval="1e3"/>'),
                '</Simulation>',
                '</Flowsim>']
        self.dump_conf(conf)
        self.config.read()
        res = self.config.read_simulation()
        self.assertTrue(len(res) == 1)
        simulation1 = {'number_samples': '15',
                       'epsilon': '1e-3',
                       'check_interval': "1e3"}
        self.assertTrue(simulation1 == res['Convergence'])

    def test_read_nodes(self):
        conf = ['<?xml version="1.0" encoding="UTF-8"?>',
                '<Flowsim datetime="2014-04-15 10:09" version="test">',
                '<Topology>',
                '<Nodes>',
                '<Default name="" rx_slot="1" tx_slot="1"/>',
                ('<Node id="0" name="node1" arrival_rate=".3" '
                 'service_rate=".4"/>'),
                ('<Node id="1" name="node2" arrival_rate=".3" '
                 'service_rate=".5"/>'),
                '<Default name=""/>',
                '</Nodes>',
                '</Topology>',
                '</Flowsim>']
        self.dump_conf(conf)
        # Loading conf
        self.config.read()
        # Reading conf
        nodes, ids = self.config.read_nodes(self.config.topology_conf[0])
        node1 = {"_id": 0,
                 "name": "node1",
                 "arrival_rate": .3,
                 "service_rate": .4,
                 "tx_slot": 1,
                 "rx_slot": 1}
        node2 = {"_id": 1,
                 "name": "node2",
                 "arrival_rate": .3,
                 "service_rate": .5,
                 "tx_slot": 1,
                 "rx_slot": 1}
        # Not sure about the order :/
        self.assertTrue(node1 == nodes[0] or node1 == nodes[1])
        self.assertTrue(node2 == nodes[0] or node2 == nodes[1])
        self.assertTrue(node1 != node2)
        self.assertTrue(len(ids) == len(set(ids)))

        # Test optional name:
        conf[6] = '<Node id="0" arrival_rate=".3" service_rate=".4"/>'
        conf[5] = ''
        self.dump_conf(conf)
        # Loading conf
        self.config.read()
        # Reading conf
        nodes, ids = self.config.read_nodes(self.config.topology_conf[0])
        self.assertTrue(len(nodes) == 1)
        self.assertTrue(nodes[0]['name'] == '')

    def test_read_links(self):
        conf = ['<?xml version="1.0" encoding="UTF-8"?>',
                '<Flowsim datetime="2014-04-15 10:09" version="test">',
                '<Topology>',
                '<Nodes>',
                ('<Node id="0" name="node1" arrival_rate=".3" '
                 'service_rate=".4"/>'),
                ('<Node id="1" name="node2" arrival_rate=".3" '
                 'service_rate=".5"/>'),
                '</Nodes>',
                '<Links>',
                '<Default weight="1." capacity="1" unidirectional="True"/>',
                ('<Link source_id="0" destination_id="1" weight="1" '
                 'capacity="1" unidirectional="True"/>'),
                ('<Link source_id="1" destination_id="0" weight="9" '
                 'capacity="2" unidirectional="False"/>'),
                '</Links>',
                '</Topology>',
                '</Flowsim>']
        self.dump_conf(conf)
        # Loading conf
        self.config.read()
        # Reading conf
        links = self.config.read_links(self.config.topology_conf[0], [0, 1])
        link1 = {"nodes": (0, 1), "weight": 1., "capacity": 1, "unidir": True}
        link2 = {"nodes": (1, 0), "weight": 9., "capacity": 2, "unidir": False}
        # Not sure about the order :/
        self.assertTrue(link1 == links[0] or link1 == links[1])
        self.assertTrue(link2 == links[0] or link2 == links[1])
        self.assertTrue(link1 != link2)
        self.assertTrue(len(links) == 2)

        # Test optional weight and capacity:
        conf[10] = ('<Link source_id="0" destination_id="1" '
                    'unidirectional="True"/>')
        self.dump_conf(conf)
        # Loading conf
        self.config.read()
        # Reading conf
        links = self.config.read_links(self.config.topology_conf[0], [0, 1])
        self.assertTrue(links[0]['capacity'] == 1 or links[1]['capacity'] == 1)
        self.assertTrue(links[0]['weight'] == 1. or links[1]['weight'] == 1.)
