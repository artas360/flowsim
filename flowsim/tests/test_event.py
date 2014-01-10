#!/usr/bin/pyton

import unittest
from flowsim.event.event import *
from flowsim.random_generator import Random_generator


class Simu(object):
    def __init__(self):
        self.result = Result()

    def end(self):
        return False


class Flow(object):

    def length(self):
        return 1


class Flow_controller(object):

    def end_flow(set_Event_managerf):
        pass

    def allocate_flow(self, node1, node2):
        return Flow()

    def free_flow(self, flow):
        return


class Node(object):

    def __int__(self):
        return 0


class Topo(object):

    def get_random_entry_node(self, number):
        return Node()

    def get_random_exit_node(self, number):
        return Node()


class Test_Event_manager(unittest.TestCase):

    def setUp(self):
        self.rand_gen = Random_generator(Topo(), None)

    def create_events(self, event_manager):
        type_list = []

        event_manager.add_event(Arrival_Event,
                                Node(),
                                arrival_rate=0.5,
                                service_rate=0.5)
        type_list.append(Arrival_Event)

        event_manager.add_event(End_flow_Event,
                                Node(),
                                delay=1234,
                                issuer_flow=None)
        type_list.append(End_flow_Event)

        event_manager.add_event(End_of_simulation_Event,
                                Node(),
                                delay=1234)
        type_list.append(End_of_simulation_Event)

        event_manager.add_event(Flow_allocation_failure_Event,
                                Node())
        type_list.append(Flow_allocation_failure_Event)

        return type_list

    def test_add_event(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        event_manager.add_event(Arrival_Event,
                                event_manager.flow_controller,
                                arrival_rate=0.5,
                                service_rate=0.5)
        assert isinstance(event_manager.event_list.pop(),
                          Arrival_Event)

        self.assertRaises(TypeError,
                          event_manager.add_event,
                          str, event_manager.flow_controller)

    def test_event_types(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list = self.create_events(event_manager)

        [type_list.index(type(event)) for event in event_manager.event_list]

    def test_flow_allocation_failure(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        event_manager.flow_allocation_failure(Node(), Node())

    def test_handle_event(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list = self.create_events(event_manager)

        for type in type_list:
            event_manager.handle_next_event()

    def test_queueing(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list = self.create_events(event_manager)

        for i in xrange(len(event_manager.event_list)-1):
            assert event_manager.event_list[i].delay_before_handling >=\
                event_manager.event_list[i].delay_before_handling
