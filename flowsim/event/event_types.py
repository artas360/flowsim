from flowsim.flowsim_exception import NoPathError
from flowsim.physical_layer.node import Node


class Event(object):
    def __init__(self, event_manager, event_issuer, **kwargs):
        self.duration = 0
        self.delay_before_handling = 0
        self.event_issuer = event_issuer
        self.event_manager = event_manager
        self.result = event_manager.get_result()

    def get_duration(self):
        return self.duration

    def get_delay(self):
        return self.delay_before_handling

    def automated_update_result(self):
        self.result.increase_event_counter(self.__class__)
        # TODO : new class NodeEvent
        if isinstance(self.event_issuer, Node):
            self.result.increase_event_counter(self.__class__,
                                               self.event_issuer)
        self.update_result()

    def update_result(self):  # To specialize in child class
        pass

    def handle_event(self):  # To specialize in child class
        pass

    def get_debug(self):
        return [self.__class__, self.delay_before_handling, self.duration]


class Arrival_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        self.arrival_rate = kwargs.pop('arrival_rate')
        self.service_rate = kwargs.pop('service_rate')
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.delay_before_handling =\
            self.event_manager.random_generator.next_arrival(
                self.arrival_rate)
        self.duration =\
            self.event_manager.random_generator.rand_duration(
                self.service_rate)

    def handle_event(self):
        if self.event_manager.new_arrivals():
            # Generating next Poisson arrival
            self.event_manager.add_event(self.__class__,
                                         self.event_issuer,
                                         arrival_rate=self.arrival_rate,
                                         service_rate=self.service_rate)
        #(src_node, dst_node) =\
        #    self.event_manager.random_generator.random_io_nodes()
        src_node = self.event_issuer
        dst_node = self.event_manager.random_generator.\
            random_exit_node(self.event_issuer)

        assert src_node != dst_node

        # Asking flow_manager to allocate the flow
        try:
            flow = self.event_manager.get_flow_controller().\
                allocate_flow(src_node, dst_node)
        except NoPathError:
            # Generating Flow_allocation_failure_Event
            self.event_manager.add_event(Flow_allocation_failure_Event,
                                         self.event_issuer)
        else:
            # Generating End_flow_event
            assert flow is not None
            self.event_manager.add_event(Flow_allocation_success_event,
                                         self.event_issuer,
                                         delay=self.duration,
                                         flow=flow)
            self.event_manager.add_event(End_flow_Event,
                                         self.event_issuer,
                                         delay=self.duration,
                                         issuer_flow=flow)


class End_flow_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.delay_before_handling = kwargs.pop('delay')
        self.flow = kwargs.pop('issuer_flow')

    def handle_event(self):
        self.event_manager.get_flow_controller().free_flow(self.flow)


class End_of_simulation_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        #Not always: delay_before_handling = float('-inf')
        self.delay_before_handling = kwargs.pop('delay')

    def handle_event(self):
        self.event_manager.set_EOS()
        self.event_manager.process_results()


class Flow_allocation_success_event(Event):

    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.delay_before_handling = float('-inf')  # Immediate handling
        self.flow = kwargs.pop('flow')

    def update_result(self):
        self.result.update_computed_value('mean_nodes_per_flow',
                                          self.flow.length(),
                                          None,
                                          None,
                                          event_type=self.__class__)


class Flow_allocation_failure_Event(Event):

    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.delay_before_handling = float('-inf')  # Immediate handling
