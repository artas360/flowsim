

class Result(object):
    def __init__(self):
        self.event_counter=dict()
        self.computed_values=dict()
        self.results=dict()

    def increase_event_counter(self, key):
        try:
            self.event_counter[key] += 1
        except KeyError:
            self.event_counter[key] = 1

    #TODO : find a better scheme ... to enable node/edge statistics

    def update_computed_value(self, value_key, update_param, update_function=None, **kwargs):
        update_function = update_function if update_function != None else self.mean
        if not self.computed_values.has_key(value_key):
            self.computed_values[value_key] = 0.

        self.computed_values[value_key]=update_function(value_key, update_param, **kwargs)

    def mean(self, key, value, **kwargs):
        event_type = kwargs.pop('event_type', key)
        number_of_values = self.event_counter[event_type]
        return (self.computed_values[key] * (number_of_values - 1) + value) / number_of_values

    def event_division(self, key, update_param, key_numerator, key_denominator, **kwargs):
        try:
            return float(self.event_counter[key_numerator]) / self.event_counter[key_denominator]
        except:
            return float('nan')

    def get_results(self):
        self.results.update(self.event_counter)
        self.results.update(self.computed_values)
        return self.results

    def print_results(self):
        results=self.get_results()
        for key, value in results.iteritems():
            print key, ': ', value

