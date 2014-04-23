from flowsim.result import Result, event_division, update_mean
from flowsim.random_generator import Random_generator
from flowsim.event.event_types import *


class Event_manager:

    def __init__(self, simulation, random_generator, user_events=[], simulation_conf={}):
        self.simulation = simulation
        self.flow_controller = None
        self.EOS = False  # End of simulation
        self.elapsed_time = 0.  # Virtual time elapsed in simulation

        # reverse sorted list of event (sort key delay_before_handling)
        self.event_list = []
        self.user_events = user_events
        self.remaining_user_events = 0

        self.result = self.simulation.result
        self.random_generator = random_generator

        # Convergence
        self.convergence_check_interval = 100
        self.convergence_number_samples = 6
        self.convergence_epsilon = 1e-3
        self.parse_simulation_config(simulation_conf)

        self.import_user_events()

    def parse_simulation_config(self, simulation_conf):
        convergence = simulation_conf.pop("Convergence", {})
        self.convergence_check_interval = \
            int(float(convergence.pop("check_interval",
                            self.convergence_check_interval)))

        self.convergence_number_samples= \
            int(float(convergence.pop("number_samples",
                            self.convergence_number_samples)))

        self.convergence_epsilon= \
            float(convergence.pop("epsilon",
                            self.convergence_epsilon))

    def import_user_events(self):
        analyzer = User_event_analyzer(self, "USER")
        for uevent in self.user_events:
            try:
                analyzer.analyze(uevent)
            except:
                raise

    def handle_next_event(self):
        try:
            event = self.event_list.pop()
        except IndexError:
            self.EOS = True
            return

        self.elapsed_time = event.get_handling_time()

        self.result.record_value('elapsed_time',
                                 "general",
                                 self.elapsed_time)

        event.handle_event()
        event.automated_update_result()
        event.post_handle()

    def add_event(self, Event_type, event_issuer, **kwargs):
        if not issubclass(Event_type, Event):
            raise TypeError
        self.event_list.append(Event_type(self, event_issuer, **kwargs))
        self.event_list.sort(key=lambda x: x.get_handling_time(), reverse=True)

    def init_run(self):
        for node in self.flow_controller.get_entry_nodes():
            self.add_event(Arrival_Event,
                           node)
        self.result.\
            add_computed_value('Blocking_rate',
                                True,
                                event_division,
                                Flow_allocation_failure_Event,
                                Arrival_Event,
                                True)

        self.result.register_convergence("Blocking_rate",
                                         "general",
                                         self.convergence_number_samples,
                                         self.convergence_epsilon)
        for event_type in Event_type_list:
            event_type.register_new_result(self.result)

    def start_event_processing(self):
        self.init_run()

        has_converged = False
        counter = 0

        while not self.EOS and \
            not (has_converged and self.remaining_user_events <= 0):
            self.handle_next_event()

            if counter == self.convergence_check_interval:
                has_converged =\
                    self.result.check_convergence('Blocking_rate',
                                                  'general',
                                                  True)
                counter = 0
            counter += 1

    def new_user_event(self):
        self.remaining_user_events += 1

    def handled_user_event(self):
        self.remaining_user_events -= 1

    def set_EOS(self):
        self.EOS = True

    def set_flow_controller(self, flow_controller):
        self.flow_controller = flow_controller

    def get_result(self):
        return self.result

    def get_flow_controller(self):
        return self.flow_controller

    def get_elapsed_time(self):
        return self.elapsed_time

    def new_arrivals(self):
        return not self.simulation.end()
