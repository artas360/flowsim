from flowsim.result import Result
from flowsim.random_generator import Random_generator
from flowsim.event.event_types import *


class Event_manager:

    def __init__(self, simulation, random_generator):
        self.simulation = simulation
        self.flow_controller = None
        self.EOS = False  # End of simulation

        # reverse sorted list of event (sort key delay_before_handling)
        self.event_list = []

        self.result = self.simulation.result
        self.random_generator = random_generator
        self.convergence_check_interval = 100

    def handle_next_event(self):
        try:
            event = self.event_list.pop()
        except IndexError:
            self.EOS = True
            return
        # Substracting duration of current event to
        # all other events' durations
        time_elapsed = event.delay_before_handling if\
            event.delay_before_handling != float('-inf') else 0.

        for x in self.event_list:
            x.delay_before_handling -= time_elapsed
        self.result.update_computed_value('time_elapsed',
                                          time_elapsed,
                                          update_function=self.result.sum)

        event.handle_event()
        event.automated_update_result()

    def add_event(self, Event_type, event_issuer, **kwargs):
        if not issubclass(Event_type, Event):
            raise TypeError
        self.event_list.append(Event_type(self, event_issuer, **kwargs))
        self.event_list.sort(key=lambda x: x.get_delay(), reverse=True)

    def start_event_processing(self):
        # print("Starting event processing")
        has_converged = False
        counter = 0
        for node in self.flow_controller.get_entry_nodes():
            self.add_event(Arrival_Event,
                           node,
                           arrival_rate=node.get_arrival_rate(),
                           service_rate=node.get_service_rate())
            self.result.\
                add_computed_value('Blocking_rate',
                                   self.result.event_division,
                                   {'key_numerator':
                                    Flow_allocation_failure_Event,
                                    'key_denominator':
                                    Arrival_Event})
        while not self.EOS and not has_converged:
            self.handle_next_event()

            if counter == self.convergence_check_interval:
                has_converged =\
                    self.result.check_convergence('Blocking_rate',
                                                  Flow_allocation_failure_Event,
                                                  100)
                counter = 0
            counter = counter + 1
        self.process_results()

    def set_EOS(self):
        self.EOS = True

    def set_flow_controller(self, flow_controller):
        self.flow_controller = flow_controller

    def flow_allocation_failure(self, source_node, dest_node):
        self.add_event(Flow_allocation_failure_Event,
                       self.flow_controller)

    def get_result(self):
        return self.result

    def get_flow_controller(self):
        return self.flow_controller

    def process_results(self):
        #self.result.print_results()
        pass

    def new_arrivals(self):
        return not self.simulation.end()
