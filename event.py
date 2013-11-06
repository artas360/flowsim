#!/usr/bin/python

import random, bisect 


class Event(object):
    def __init__(self, event_manager, event_issuer, **kwargs):
        self.duration = 0
        self.event_manager = event_manager
        self.event_issuer = event_issuer

    def get_duration(self):
        return self.duration

    def handle_event(self): # To overload in child class
        pass

class Arrival_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.rate = kwargs.pop('arrival_rate')
        self.duration = random.expovariate(self.rate)

    def handle_event(self):
        # Generating next Poisson arrival
        self.event_manager.add_event(self.__class__, self.event_issuer, arrival_rate=self.rate)

# TODO : move to flow module
class End_flow_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        # Event issuer should be the flow
        super(self.__class__, self).__init__(event_manager, event_issuer)
        assert (isinstance(event_issuer, Flow))

    def handle_event(self):
        event_issuer.end_flow()


class Event_manager:
    def __init__(self, arrival_rate, rand_seed=None):
        self.event_list=[] # reverse sorted list of event (sort key duration)
        random.seed(rand_seed)
        self.arrival_rate = arrival_rate

    # Should be called by the Simulation_manager
    def handle_next_event(self):
        try:
            event = self.event_list.pop()
        except IndexError:
            pass

        event.handle_event()

        # Substracting duration of current event to all other envents' durations
        #for x in self.event_list: x.duration -= event.duration

    # TODO : improve sorting scheme
    def add_event(self, Event_type, event_issuer, **kwargs):
        if not issubclass(Event_type, Event):
            raise TypeError("Passing type "+str(Event_type)+" instead of Event")
        self.event_list.append(Event_type(self, event_issuer, **kwargs))
        self.event_list.sort(key=lambda x: x.get_duration(), reverse=True)

    def start_event_processing(self):
        self.add_event(Arrival_Event, None, arrival_rate=self.arrival_rate)

if __name__ == '__main__':
    event_manager=Event_manager(0.2)
    event_manager.start_event_processing()
    print [(type(x), x.duration) for x in event_manager.event_list]
    event_manager.handle_next_event()
    print [(type(x), x.duration) for x in event_manager.event_list]


