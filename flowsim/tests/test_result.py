import unittest
from flowsim import result
from math import isnan


class Test_result(unittest.TestCase):
    def setUp(self):
        self.result = result.Result()
        
    def test_increase_value(self):
        self.result.increase_value("counter", "node1")
        self.assertTrue(self.result.get("counter", "node1") == 1)
        self.result.increase_value("counter", "node1", 3)
        self.assertTrue(self.result.get("counter", "node1") == 4)

        self.result.increase_value("counter2", "node1")
        self.assertTrue(self.result.get("counter2", "node1") == 1)
        self.assertTrue(self.result.get("counter", "node1") == 4)

        self.result.increase_value("counter", "node2")
        self.assertTrue(self.result.get("counter", "node1") == 4)
        self.assertTrue(self.result.get("counter", "node2") == 1)

    def test_register_value(self):
        self.result.record_value("counter", "node1", 1.)
        self.assertTrue(self.result.get("counter", "node1") == 1.)
        self.result.record_value("counter", "node1", 4.)
        self.assertTrue(self.result.get("counter", "node1") == 4.)

        self.result.record_value("counter2", "node1", 1.)
        self.assertTrue(self.result.get("counter2", "node1") == 1.)
        self.assertTrue(self.result.get("counter", "node1") == 4.)

        self.result.record_value("counter", "node2", 1.)
        self.assertTrue(self.result.get("counter", "node1") == 4.)
        self.assertTrue(self.result.get("counter", "node2") == 1.)

    def test_computed_value(self):
        self.result.record_value("counter", "node1", 1.)
        self.result.add_computed_value("val1", False, result.update_mean, "val1", "counter", False)
        self.assertTrue(self.result.get("val1", "node1") == 0.)
        self.result.update_computed_value("val1", "node1", 5.)
        self.assertTrue(self.result.get("val1", "node1") == 5.)
        self.result.record_value("counter", "node1", 2.)
        self.result.update_computed_value("val1", "node1", 7.)
        self.assertTrue(self.result.get("val1", "node1") == 6)

        self.result.record_value("arrivals", "general", 9)
        self.result.record_value("failures", "general", 2)
        self.result.add_computed_value("BR", True, result.event_division, "failures", "arrivals", True)
        self.assertTrue(self.result.get("BR", "general") == 2./9.)
        self.result.record_value("failures", "general", 3)
        self.assertTrue(self.result.get("BR", "general") == 3./9.)

    def test_process_node_value(self):
        self.result.record_value("counter", "node1", 1.)
        self.result.record_value("counter", "node2", 3.)
        self.result.record_value("counter", "node3", 5.)
        self.result.record_value("counter", "node4", 7.)
        self.assertTrue(self.result.process_node_value(None, None, "counter", None, sum) == 16.)
        self.result.record_value("counter", "general", 1.)
        self.assertTrue(self.result.process_node_value(None, None, "counter", None, sum) == 16.)

    def test_sample_convergence(self):
        self.assertRaises(KeyError, self.result.check_convergence, "conv")
        self.result.register_convergence("conv", 3, .1)
        self.assertFalse(self.result.check_convergence("conv"))
        self.result.check_convergence("conv", 9)
        self.assertFalse(self.result.check_convergence("conv"))
        self.result.check_convergence("conv", 3.1)
        self.assertFalse(self.result.check_convergence("conv"))
        self.result.check_convergence("conv", 3)
        self.assertFalse(self.result.check_convergence("conv"))
        self.result.check_convergence("conv", 3)
        self.assertTrue(self.result.check_convergence("conv"))

    def test_get_results(self):
        self.result.record_value("counter", "node1", 1.)
        self.result.add_computed_value("val1", False, result.update_mean, "val1", "counter", False)
        self.result.update_computed_value("val1", "node1", 5.)

class Test_sample_container(unittest.TestCase):
    def setUp(self):
        self.sc = result.Sample_container(3, .1)

    def test_standard_deviation(self):
        self.sc.update_samples(0.)
        self.sc.update_samples(3.)
        self.assertTrue(isnan(self.sc.standard_deviation()))
        self.sc.update_samples(6)
        self.assertTrue(self.sc.standard_deviation() == 6**0.5)

    def test_has_converged(self):
        self.sc.update_samples(9.)
        self.sc.update_samples(3.)
        self.assertFalse(self.sc.has_converged())
        self.sc.update_samples(3.1)
        self.assertFalse(self.sc.has_converged())
        self.sc.update_samples(3.)
        self.assertTrue(self.sc.has_converged())
