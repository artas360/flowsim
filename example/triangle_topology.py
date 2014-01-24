import flowsim as fs

s = fs.Simulation(0.9, 0.9)
s.init_simulation([0, 1, 2], [(0, 1), (0, 2), (1, 2)])
# fs.physical_layer.topology.draw_graph(s.topology)
print s.launch_simulation()
