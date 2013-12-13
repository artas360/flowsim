#!/usr/bin/python

from flowsim.result import Result
from flowsim.random_generator import Random_generator
import flowsim.event.event_types as e_types


class Event_manager:
    def __init__(self, simulation, random_generator):
        self.simulation = simulation
        self.flow_controller = None
        self.EOS = False #End of simulation

        # reverse sorted list of event (sort key delay_before_handling)
        self.event_list=[]

        self.result = self.simulation.result
        self.random_generator = random_generator

    def handle_next_event(self):
        try:
            event = self.event_list.pop()
        except IndexError:
            pass
        # Substracting duration of current event to all other events' durations
        for x in self.event_list:
            x.delay_before_handling -= event.delay_before_handling
        #print 'Event List: ',self.event_list

        event.handle_event()
        event.update_result()

    # TODO : improve sorting scheme
    def add_event(self, Event_type, event_issuer, **kwargs):
        if not issubclass(Event_type, e_types.Event):
            raise TypeError("Passing type "+str(Event_type)+" instead of Event")
        self.event_list.append(Event_type(self, event_issuer, **kwargs))
        self.event_list.sort(key=lambda x: x.get_delay(), reverse=True)
        #print 'Event List sorted: ', [(evt.__class__, evt.delay_before_handling) for evt in self.event_list]

    def start_event_processing(self):
        print("Starting event processing")
        self.add_event(e_types.Arrival_Event, self.flow_controller)
        while not (self.EOS or self.simulation.end()):
            self.handle_next_event()
        self.process_results()

    def set_EOS(self):
        self.EOS = True

    def set_flow_controller(self, flow_controller):
        self.flow_controller = flow_controller

    def flow_allocation_failure(self, source_node, dest_node):
        self.add_event(e_types.Flow_allocation_failure_Event, self.flow_controller)

    def increase_result(self, key):
        self.result.increase_result(key)

    def process_results(self):
        pass#TODO 
