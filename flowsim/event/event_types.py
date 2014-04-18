from flowsim.flowsim_exception import NoPathError
from flowsim.physical_layer.node import Node


class Triggered_events(object):
    def __init__(self, trigger):
        self.trigger = trigger
    # TODO

class Event(object):
    def __init__(self, event_manager, event_issuer, **kwargs):
        self.event_end_time = 0
        self.handling_time = 0
        self.event_issuer = event_issuer
        self.event_manager = event_manager
        self.result = event_manager.get_result()
        self.immediate_handling = self.event_manager.get_elapsed_time

    def get_event_end_time(self):
        return self.event_end_time

    def get_handling_time(self):
        return self.handling_time

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
        return [self.__class__, self.handling_time, self.event_end_time]


class Arrival_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        self.arrival_rate = kwargs.pop('arrival_rate')
        self.service_rate = kwargs.pop('service_rate')
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = self.event_manager.get_elapsed_time() +\
            self.event_manager.random_generator.next_arrival(
                self.arrival_rate)
        self.event_end_time = self.handling_time +\
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
                                         flow=flow)
            self.event_manager.add_event(End_flow_Event,
                                         self.event_issuer,
                                         handling_time=self.event_end_time,
                                         issuer_flow=flow)


class End_flow_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = kwargs.pop('handling_time')
        self.flow = kwargs.pop('issuer_flow')

    def handle_event(self):
        self.event_manager.get_flow_controller().free_flow(self.flow)


class End_of_simulation_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = kwargs.pop('handling_time')

    def handle_event(self):
        self.event_manager.set_EOS()
        self.event_manager.process_results()


class Flow_allocation_success_event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = self.immediate_handling()
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
        self.handling_time = self.immediate_handling()


class User_event_analyzer(object):
    # TODO set event issuer to config for coherency.
    def __init__(self, event_manager, event_issuer):
        self.event_manager = event_manager
        self.event_issuer = event_issuer

    def analyze(self, event_description):
        if event_description.pop("trigger_type", None) == "time":
            self.analyze_time_event(event_description)
        else:
            raise NotImplemented

    def analyze_time_event(self, event_description):
        try:
            handling_time = float(event_description["trigger_value"])
            target = int(event_description["event_target"])
            effect_value = float(event_description["effect_value"])
        except:
            raise ValueError("Bad user event")

        if event_description["type"] == "arrival_burst_event":
            self.event_manager.add_event(Arrival_burst_event,
                                         "User",
                                         handling_time=handling_time,
                                         target=target,
                    					 effect_value=effect_value)
        else:
            raise NotImplemented


class Arrival_burst_event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = kwargs.pop("handling_time")
        self.target = kwargs.pop("target")
        self.new_arrival_rate = kwargs.pop("effect_value")

    def handle_event(self):
        topo = self.event_manager.get_flow_controller().get_topology()
        topo.swap_node_arr_rate(self.target, self.effect_value)
