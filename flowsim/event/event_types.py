
from flowsim.flow.flow_controller import NoPathError


class Event(object):
    def __init__(self, event_manager, event_issuer, **kwargs):
        self.duration = 0
        self.delay_before_handling = 0
        self.event_manager = event_manager
        self.event_issuer = event_issuer

    def get_duration(self):
        return self.duration

    def get_delay(self):
        return self.delay_before_handling

    def update_result(self): # To specialize in child class
        self.event_manager.increase_result(str(self.__class__))

    def handle_event(self): # To specialize in child class
        pass

    def get_debug(self):
        return [self.__class__, self.delay_before_handling, self.duration]


class Arrival_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        # Event issuer should be flow_manager
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.delay_before_handling = self.event_manager.random_generator.next_arrival()
        self.duration = self.event_manager.random_generator.rand_duration()

    def handle_event(self):
        # Generating next Poisson arrival
        self.event_manager.add_event(self.__class__, self.event_issuer)
        # Asking flow_manager to allocate the flow
        (src_node, dst_node)=self.event_manager.random_generator.random_io_nodes()
        #print [(int(a), int(b)) for (a,b) in self.event_issuer.topology.edges()]
            
        try:
            #print 'Trying path', int(src_node), ' -> ', int(dst_node)
            flow=self.event_issuer.allocate_flow(src_node, dst_node)
        except NoPathError as te:
            # Generating Flow_allocation_failure_Event
            self.event_manager.add_event(Flow_allocation_failure_Event, self.event_issuer)
            #print 'No path', int(src_node), ' -> ', int(dst_node)
        else:
            # Generating End_flow_event
            self.event_manager.add_event(End_flow_Event, self.event_issuer, delay=self.duration, issuer_flow=flow)


class End_flow_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.delay_before_handling = kwargs.pop('delay')
        self.flow = kwargs.pop('issuer_flow')

    def handle_event(self):
        #print [(int(a), int(b)) for (a,b) in self.event_issuer.topology.edges()]
        self.event_issuer.free_flow(self.flow)
        #print [(int(a), int(b)) for (a,b) in self.event_issuer.topology.edges()]


class End_of_simulation_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        #Not always: delay_before_handling = float('-inf')
        self.delay_before_handling = kwargs.pop('delay')

    def handle_event(self):
        self.event_manager.set_EOS()
        self.event_manager.process_results()


class Flow_allocation_failure_Event(Event):

    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.delay_before_handling=float('-inf') #Immediate handling

    def handle_event(self):
        pass
