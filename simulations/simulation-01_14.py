#!/usr/bin/python

import cPickle as pik
import flowsim as fs
from copy import deepcopy
import time

timestr = time.strftime("%Y%m%d-%H%M%S")
result_file = open('simulation-'+timestr+'.result', 'w')

(nodes, edges) = fs.physical_layer.topology.torus3D(16, 16, 16)

# Same for all runs
service_rate = .5
arrival_rate = [.1, .2, .3, .4, .5, .6, .7, .8, .9]

runs_per_rate = 6

s = fs.Simulation(arrival_rate[0], service_rate)

s.init_simulation(nodes, edges)

results = dict([(j, [None for i in xrange(runs_per_rate)])
                for j in arrival_rate])

for rate in arrival_rate:
    for i in xrange(runs_per_rate):
        s.reset(rate)
        results[rate][i] = deepcopy(s.launch_simulation())

print results
pik.dump(results, result_file, -1)

result_file.close()

#result_file = open('simulation-'+timestr+'.result', 'r')
#results = pik.load(result_file)
