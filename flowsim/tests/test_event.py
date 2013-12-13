#!/usr/bin/pyton

import unittest
from flowsim.event.event import *
from flowsim.random_generator import Random_generator


class Simu(object):
    def __init__(self):
        self.result = Result()

    def end():
        return False


class Flow_controller(object):

    def end_flow(set_Event_managerf):
        pass

    def allocate_flow(self, node1, node2):
        return

    def free_flow(self, flow):
        return


class Node(object):
    pass


class Topo(object):

    def get_random_entry_node(self, numer):
        return 

    def get_random_exit_node(self, numer):
        return 


class Test_Event_manager(unittest.TestCase):
    def setUp(self):
        self.rand_gen = Random_generator(Topo() , 0.2, 0.2, None)

    def create_events(self, event_manager):
        type_list=[]

        event_manager.add_event(
                e_types.Arrival_Event,
                event_manager.flow_controller)
        type_list.append(e_types.Arrival_Event)

        event_manager.add_event(
                e_types.End_flow_Event,
                event_manager.flow_controller,
                delay=1234,
                issuer_flow=None)
        type_list.append(e_types.End_flow_Event)

        event_manager.add_event(
                e_types.End_of_simulation_Event,
                event_manager.flow_controller,
                delay=1234)
        type_list.append(e_types.End_of_simulation_Event)

        event_manager.add_event(
                e_types.Flow_allocation_failure_Event,
                event_manager.flow_controller)
        type_list.append(e_types.Flow_allocation_failure_Event)

        return type_list


    def test_add_event(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        event_manager.add_event(
                e_types.Arrival_Event,
                event_manager.flow_controller)
        assert isinstance(event_manager.event_list.pop(), e_types.Arrival_Event)

        self.assertRaises(TypeError, event_manager.add_event,
                str,
                event_manager.flow_controller)

    def test_event_types(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list=self.create_events(event_manager)

        print 'BIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII'
        print type_list
        print event_manager.event_list
        [type_list.index(type(event)) for event in event_manager.event_list]

    def test_flow_allocation_failure(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        event_manager.flow_allocation_failure(Node(), Node())

    def test_handle_event(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list=self.create_events(event_manager)

        for type in type_list:
            event_manager.handle_next_event()

    def test_queueing(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list=self.create_events(event_manager)

        for i in xrange(len(event_manager.event_list)-1):
            assert event_manager.event_list[i].delay_before_handling >= event_manager.event_list[i].delay_before_handling


