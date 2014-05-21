from flowsim import Simulation
from flowsim.plot import simple_time_plot

sim = Simulation(.1, .2)
sim.load_conf("../config/reoptimization-config.xml")
sim.launch_simulation()
simple_time_plot(sim.result, "Blocking_rate")
