#!/usr/bin/python

import flow.flow_controller as fc
import event.event as em
import physical_layer.topology as tp

class Simulation(object):
    def __init__(self):
        pass

    def init_simulation(self):
        pass

    def init_event_manager(self, arrival_rate, seed=None):
        self.event_manager = em.Event_manager(self, arrival_rate, seed)

    def init_topology(self, nodes, edges):
        # nodes -> list of int
        # edges -> list of (node1, node2) or (node1, node2, capacity)
        # or (node1, node2, capacity, weight)
        self.topology = tp.Topology()
        self.topology.build_topology_from_int(nodes,edges)

    def import_topology(self, filename):
        self.topology = tp.Topology()
        self.topology.import_topology(filename)

    def init_flow_controller(self):
        self.flow_controller = fc.Flow_controller(self.topology, self.event_manager, self)
        self.event_manager.set_flow_controller(self.flow_controller)




