class Sample_container(object):
    def __init__(self, number_samples, epsilon):
        self.counter = 0
        self.epsilon = epsilon
        self.infinity = float("inf")
        self.samples = [self.infinity for i in range(number_samples)]

    def update_samples(self, sample):
        self.samples[self.counter] = sample
        self.counter += 1
        self.counter %= len(self.samples)

    def standard_deviation(self):
        return (sum(map(lambda x: x**2, self.samples)) / float(len(self.samples)) -\
                (sum(self.samples) / float(len(self.samples)))**2)**0.5

    def has_converged(self):
        assert self.epsilon > 0.
        return self.standard_deviation() < self.epsilon


class Result(object):
    def __init__(self):
        self.results = dict()
        self.function_map = dict()
        self.convergence_map = dict()
        self.general_key = "general"

    def init_result(self, value_name, source_object):
        if not source_object in self.results:
            self.results[source_object] = dict()
        if not value_name in self.results[source_object]:
            self.results[source_object][value_name] = 0.

    def increase_value(self, value_name, source_object, increment = 1):
        try:
            self.results[source_object][value_name]
        except KeyError:
            self.init_result(value_name, source_object)
        self.results[source_object][value_name] += increment

    def record_value(self, value_name, source_object, value):
        try:
            self.results[source_object][value_name] 
        except KeyError:
            self.init_result(value_name, source_object)
        self.results[source_object][value_name] = value 

    def add_computed_value(self, value_name, is_general_value, function, key1, key2, update_on_get = False):
        self.function_map[value_name] = [is_general_value,
                                         update_on_get,
                                         function,
                                         key1,
                                         key2]
        if is_general_value:
            self.update_computed_value(value_name, self.general_key, 0.)
        else:
            [self.update_computed_value(value_name, x, 0.) for x in self.results]

    def update_computed_value(self, value_name, source_object, new_elt):
        try:
            self.results[value_name][source_object]
        except KeyError:
            self.init_result(value_name, source_object)
        # function record
        f = self.function_map[value_name]
        self.results[source_object][value_name] = f[2](self.results[source_object], new_elt, *f[3:]) 
    
    def get_computed_value(self, value_name, source_object):
        if self.function_map[value_name][1]:
            self.update_computed_value(value_name, source_object, 0)
        return self.results[source_object][value_name]
        
    def register_convergence(self, value_name, number_samples, epsilon):
        self.convergence_map[value_name] = Sample_container(number_samples, epsilon)

    def check_convergence(self, value_name, new_sample = None):
        if new_sample is not None:
            self.convergence_map[value_name].update_samples(new_sample)
        return self.convergence_map[value_name].has_converged()

    def get(self, value_name, source_object):
        if value_name in self.function_map:
            return self.get_computed_value(value_name, source_object)
        try:
            return self.results[source_object][value_name]
        except KeyError:
            raise

    def process_node_value(self, foo1, foo2, value_name, foo3, process_function):
        values = []
        for source_object in self.results:
            if source_object != self.general_key:
                values.append(self.get(value_name, source_object))
        return process_function(values)

def update_mean(submap, new_element, mean_key, denominator_key):
    try:
        number_of_values = float(submap[denominator_key])
        assert(number_of_values != 0.)
    except KeyError:
        return float('nan')
    return (submap[mean_key] * (number_of_values - 1) + new_element) / number_of_values

def event_division(submap, foo1, numerator_key, denominator_key):
    assert(submap[denominator_key] != 0.)
    try:
        return submap[numerator_key] / float(submap[denominator_key])
    except KeyError:
        return float('nan')
