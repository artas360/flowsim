from flowsim.flowsim_exception import NoPathError
from flowsim.result import update_mean


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
        self.result.increase_value(self.__class__, "general")
        self.result.increase_value(self.__class__,
                                   self.event_issuer)
        self.update_result()

    def update_result(self):  # To specialize in child class
        pass

    def handle_event(self):  # To specialize in child class
        pass

    def post_handle(self):
        pass

    @staticmethod
    def register_new_result(result):
        pass


class Arrival_Event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.arrival_rate = event_issuer.get_arrival_rate()
        self.service_rate = event_issuer.get_service_rate()
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
                                         self.event_issuer)
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


class Flow_allocation_success_event(Event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = self.immediate_handling()
        self.flow = kwargs.pop('flow')

    @staticmethod
    def register_new_result(result):
        result.add_computed_value('mean_hops',
                                  True,
                                  update_mean,
                                  'mean_hops',
                                  Flow_allocation_success_event)

    def update_result(self):
        self.result.update_computed_value('mean_hops',
                                          "general",
                                          self.flow.length())


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
            try:
                target = int(event_description["event_target"])
            except ValueError:
                target = None
            effect_value = float(event_description["effect_value"])
        except KeyError:
            raise KeyError("Illegal user event: missing field")
        except ValueError as ve:
            raise ValueError("Illegal user event: wrong type")

        if event_description["type"] == "arrival_burst_event":
            self.event_manager.add_event(Arrival_burst_event,
                                         "User",
                                         handling_time=handling_time,
                                         target=target,
                                         effect_value=effect_value)
        elif event_description["type"] == "sample_event":
            self.event_manager.add_event(Sample_event,
                                         "User",
                                         handling_time=handling_time,
                                         time_interval=effect_value)
        elif event_description["type"] == "reconfigure_topology_event":
            self.event_manager.add_event(Reconfigure_topology_event,
                                         "User",
                                         handling_time=handling_time,
                                         treshold=effect_value)
        elif event_description["type"] == "watcher_event":
            self.event_manager.add_event(Watcher_event,
                                         "User",
                                         handling_time=handling_time)
        else:
            raise NotImplemented


class Sample_event(Event):  # Should not be User_event or infinite loop
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = kwargs.pop("handling_time")
        self.time_interval = kwargs.pop("time_interval")

    def handle_event(self):
        self.result.take_snapshot(self.handling_time, True)
        self.event_manager.add_event(self.__class__,
                                     self.event_issuer,
                                     handling_time=(self.handling_time +
                                                    self.time_interval),
                                     time_interval=self.time_interval)


class User_event(Event):
    def __init__(self, event_manager, event_issuer):
        Event.__init__(self, event_manager, event_issuer)
        self.event_manager.new_user_event()

    def post_handle(self):
        self.event_manager.handled_user_event()


# The Watcher on the Wall
class Watcher_event(User_event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = kwargs.pop("handling_time")


class Arrival_burst_event(User_event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = kwargs.pop("handling_time")
        self.target = kwargs.pop("target")
        self.new_arrival_rate = kwargs.pop("effect_value")

    def handle_event(self):
        topo = self.event_manager.get_flow_controller().get_topology()
        topo.swap_node_arr_rate(self.target, self.new_arrival_rate)


class Reconfigure_topology_event(User_event):
    def __init__(self, event_manager, event_issuer, **kwargs):
        super(self.__class__, self).__init__(event_manager, event_issuer)
        self.handling_time = kwargs.pop("handling_time")
        self.treshold = kwargs.pop("treshold")

    def handle_event(self):
        self.event_manager.get_flow_controller().\
            reconfigure_topology(self.result, self.treshold)


Event_type_list = [Arrival_Event,
                   End_flow_Event,
                   End_of_simulation_Event,
                   Flow_allocation_success_event,
                   Flow_allocation_failure_Event,
                   Arrival_burst_event,
                   Sample_event,
                   Watcher_event,
                   Reconfigure_topology_event
                   ]
