class Result(object):

    def __init__(self):
        self.data = dict()
        self.data['general'] = dict()
        self.data['general']['event_counter'] = dict()
        self.data['general']['computed_values'] = dict()
        self.convergence = dict()
        self.computed_values_fcts = dict()
        self.results = dict()
        self.check_samples = 5

    def init_node_data(self, node):
        if node not in self.data:
            self.data[node] = dict()
            self.data[node]['event_counter'] = dict()
            self.data[node]['computed_values'] = dict()

    def init_convergence_data(self, key, trigger_type):
        self.convergence[key] = dict()
        self.convergence[key]['counter'] = cntr = 0
        self.convergence[key]['samples'] =\
            [float('-inf') for i in xrange(self.check_samples)]
        self.convergence[key]['trigger_type'] = trigger_type
        self.convergence[key]['last_check_count'] =\
            self.data['general']['event_counter'][trigger_type]

    def increase_event_counter(self, key, node='general'):
        try:
            self.data[node]['event_counter'][key] += 1
        except KeyError:
            self.init_node_data(node)
            self.data[node]['event_counter'][key] = 1

    def check_convergence(self, key, trigger_type, expected_diff):
        try:
            if abs(self.data['general']['event_counter'][trigger_type] -
                self.convergence[key]['last_check_count']) >=\
                expected_diff:
                    return self.check_mean_convergence(key)
        except KeyError:
            self.init_convergence_data(key, trigger_type)
        return False

    def add_computed_value(self, key, update_function, function_args):
        self.computed_values_fcts[key] = [update_function, function_args]

    def update_computed_value(self, value_key, update_param, node=None,
                              update_function=None, **kwargs):

        if update_function is None and value_key in self.computed_values_fcts:
            update_function = self.computed_values_fcts[value_key][0]
            kwargs = self.computed_values_fcts[value_key][1]
        elif update_function is not None:
            self.add_computed_value(value_key, update_function, kwargs)
        else:
            update_function = self.mean
            self.add_computed_value(value_key, update_function, kwargs)

        node = node if node is not None else 'general'

        try:
            self.data[node]['computed_values']
        except KeyError:
            self.init_node_data(node)

        if not value_key in self.data[node]['computed_values']:
            self.data[node]['computed_values'][value_key] = 0.

        self.data[node]['computed_values'][value_key] = \
            update_function(self.data[node], value_key,
                            update_param, **kwargs)

    def mean(self, dictionary, key, value, **kwargs):
        event_type = kwargs.pop('event_type', key)
        try:
            number_of_values = dictionary['event_counter'][event_type]
        except KeyError:
            return float('nan')
        return (dictionary['computed_values'][key] *
                (number_of_values - 1) + value) / number_of_values

    def sum(self, dictionary, key, value, **kwargs):
        return dictionary['computed_values'][key] + value

    def event_division(self, dictionary, key, update_param, key_numerator,
                       key_denominator, **kwargs):
        try:
            return (float(dictionary['event_counter'].get(key_numerator, 0.)) /
                    dictionary['event_counter'][key_denominator])
        except KeyError:
            return float('nan')

    def process_node_value(self, key, process_function, **kwargs):
        nodes = self.data.keys()
        nodes.remove('general')
        [self.update_computed_value(key, None, node, None) for node in nodes]
        values = [self.data[node]['computed_values'][key] for node in nodes]
        return process_function(values, **kwargs)

    def sem(self, data):
        n = len(data)
        mean = float(sum(data))/n
        return ((1./n)*sum([(sample - mean)**2 for sample in data]))**0.5

    def check_mean_convergence(self, key, epsilon=1.e-2):
        try:
            cntr = self.convergence[key]['counter']
        except KeyError:
            self.init_convergence_data(key, self.convergence[key]['trigger_type'])
        self.convergence[key]['samples'][cntr] =\
            self.process_node_value(key, lambda x: float(sum(x)) / len(x))
        self.convergence[key]['last_check_count'] =\
            (self.data['general']['event_counter']
             [self.convergence[key]['trigger_type']])
        self.convergence[key]['counter'] = (cntr + 1) % self.check_samples
        if self.sem(self.convergence[key]['samples']) < epsilon:
            return True
        return False

    def get_results(self):
        self.results.update(self.data['general']['event_counter'])
        self.results.update(self.data['general']['computed_values'])
        for key in self.convergence:
            self.results[key] =\
                self.convergence[key]['samples'][
                    self.convergence[key]['counter']]
        return self.results

    def print_results(self):
        results = self.get_results()
        for key, value in results.iteritems():
            print key, ':', value
