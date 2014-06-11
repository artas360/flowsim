#!/usr/bin/pyton

from unittest import TestCase

from flowsim.event.event_types import *
from flowsim.random_generator import Random_generator
from flowsim.event.event import Event_manager, Event
from flowsim.result import Result
from heapq import heappop


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

    def get_topology(self):
        return Topo()


class Node(object):

    def __int__(self):
        return 0

    def get_arrival_rate(self):
        return .1

    def get_service_rate(self):
        return .1


class Topo(object):

    def get_random_entry_node(self, number):
        return Node()

    def get_random_exit_node(self, number):
        return Node()

    def swap_node_arr_rate(self, foo1=None, foo2=None):
        pass


class Test_Event_manager(TestCase):

    def setUp(self):
        self.rand_gen = Random_generator(Topo(), None)

    def create_events(self, event_manager):
        event_count = 0

        event_manager.add_event(Arrival_Event,
                                Node(),
                                arrival_rate=0.5,
                                service_rate=0.5)
        event_count += 1

        event_manager.add_event(End_flow_Event,
                                Node(),
                                handling_time=1234,
                                issuer_flow=None)
        event_count += 1

        event_manager.add_event(End_of_simulation_Event,
                                Node(),
                                handling_time=1234)
        event_count += 1

        event_manager.add_event(Flow_allocation_failure_Event,
                                Node())
        event_count += 1

        event_manager.add_event(Flow_allocation_success_event,
                                Node(),
                                flow=Flow())
        event_count += 1

        event_manager.add_event(Arrival_burst_event,
                                "User",
                                handling_time=15.,
                                target=Node(),
                                effect_value=.2)
        event_count += 1

        event_manager.add_event(Sample_event,
                                "User",
                                handling_time=0,
                                time_interval=9,
                                target_value='Blocking_rate')
        event_count += 1

        event_manager.add_event(Watcher_event,
                                "User",
                                handling_time=0)
        event_count += 1

        # Checking that all events are tested
        self.assertEqual(event_count, len(Event_type_list))

        return Event_type_list

    def test_add_event(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        event_manager.add_event(Arrival_Event,
                                Node())
        assert isinstance(heappop(event_manager.event_list)[1],
                          Arrival_Event)

        self.assertRaises(AssertionError,
                          event_manager.add_event,
                          str, event_manager.flow_controller)

    def test_event_types(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list = self.create_events(event_manager)

        [type_list.index(type(e[1])) for e in event_manager.event_list]

    def test_user_event_types(self):
        user_event = {"type": "arrival_burst_event",
                      "trigger_type": "time",
                      "trigger_value": "15",
                      "event_target": "1",  # this is a node _id
                      "effect_value": ".6"}
        event_manager = Event_manager(Simu(), self.rand_gen, [user_event])

    def test_handle_event(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list = self.create_events(event_manager)

        for _type in type_list:
            _type.register_new_result(event_manager.result)
        for _type in type_list:
            event_manager.handle_next_event()

    def test_queueing(self):
        event_manager = Event_manager(Simu(), self.rand_gen)
        event_manager.set_flow_controller(Flow_controller())

        type_list = self.create_events(event_manager)

        tmp = None
        tmp2 = heappop(event_manager.event_list)[1]
        for i in xrange(len(event_manager.event_list)):
            tmp = tmp2
            tmp2 = heappop(event_manager.event_list)[1]
            self.assertTrue(tmp2.handling_time >= tmp.handling_time)

    def test_sample_event(self):
        event_manager = Event_manager(Simu(), self.rand_gen)

        event_manager.add_event(Sample_event,
                                "User",
                                handling_time=0,
                                time_interval=9,
                                target_value='Blocking_rate')

        event_manager.handle_next_event()
        event_manager.handle_next_event()
        gen_key = event_manager.result.general_key
        sshots = event_manager.result.get_snapshots('Blocking_rate', gen_key)
        self.assertEqual(map(lambda x: x[0], sshots), [0, 9])
