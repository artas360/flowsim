#!/usr/bin/python

from threading import Thread
from argparse import ArgumentParser
from cPickle import dump, load
from time import sleep, strftime
from time import time as epoc_time

from flowsim import Simulation
from flowsim.physical_layer.topology import torus2D


class Simulation_thread(Thread):
    def __init__(self, simulation, result):
        # self.simulation = deepcopy(simulation)
        self.simulation = simulation

        self.result = result
        super(self.__class__, self).__init__()

    def run(self):
        self.result.update(self.simulation.launch_simulation())


def float_range(start, stop, step):
    _count = start
    while _count < stop:
        yield _count
        _count += step


def UCL(data, confidence=0.95):
    import scipy.stats
    n = len(data)
    mean = sum(data) / n
    se = scipy.stats.sem(data)
    h = se * scipy.stats.t._ppf((1+confidence)/2., n-1)
    return mean, h


timestr = strftime("%Y%m%d-%H%M%S")

# Parsing args
parser = ArgumentParser(
    description='Run simulation or process results.')

parser.add_argument('--runs-per-sample', type=int, nargs=1, default=6,
                    help='Number of simulation to run for each rate')
parser.add_argument('--arrival-rate-range', nargs=3, default=[.1, 1., .1],
                    help="""arrival rate: first last range\n
                    Last is not is the range""", type=int)
parser.add_argument('--service-rate-range', nargs=3, default=[.1, .2, .1],
                    help="""arrival rate: first last range\n
                    Last is not is the range""", type=int)
parser.add_argument('--torus3D', type=int, nargs=3, default=[16, 16, 16],
                    help="""Run on topology torus 3D with given dimensions""")
parser.add_argument('-o', type=str, dest='filename',
                    default='simulation-'+timestr+'.result',
                    help="""File to use for reading/writing results""")
parser.add_argument('action', metavar="ACTION", nargs='+', default='run',
                    help="""run: run new simulation\n
                    analyze: analyze simulation\n
                    NB: You can combine actions""", type=str)

args = parser.parse_args()

if('run' in args.action):
    print("Launching simulation")

    launch_time = epoc_time()

    runs_per_rate = args.runs_per_sample
    arrival_rate = [f for f in float_range(*args.arrival_rate_range)]
    service_rate = [f for f in float_range(*args.service_rate_range)]

    (nodes, edges) = torus2D(3, 3)

    waiting_delay = 6

    s = Simulation(0, 0)

    s.init_simulation(nodes, edges)

    results = dict([((arr, serv), [{} for i in xrange(runs_per_rate)])
                    for serv in service_rate for arr in arrival_rate])
    threads = range(runs_per_rate)

    for a_rate in arrival_rate:
        for s_rate in service_rate:
            s.reset(a_rate, s_rate)
            for i in xrange(runs_per_rate):
                threads[i] =\
                    Simulation_thread(s.copy(), results[(a_rate, s_rate)][i])
                threads[i].start()
            while any(map(lambda x: x.isAlive(), threads)):
                sleep(waiting_delay)

    # Dumping results
    result_file = open(args.filename, 'w')
    dump(results, result_file, -1)
    result_file.close()

    print("""----------------------------------------------------------------
Simulation lasted: %f s
----------------------------------------------------------------""" %
          (epoc_time() - launch_time))


if("analyze" in args.action):
    print("Analyzing simulation results")

    # Load results if necessary
    if (not "run" in args.action):
        result_file = open(args.filename, 'r')
        results = load(result_file)

    assert (len(results) != 0)

    runs_per_rate = len(results[results.keys()[0]])

#    for rate in results:
#        print(rate, sum([results[rate][run]['Blocking_rate']\
#            for run in range(runs_per_rate)])/runs_per_rate)

    # plotting
    import matplotlib.pyplot as plt
    assert(runs_per_rate > 1)

    points = []
    for rate in results:
        points.append(((rate[0]/rate[1]),
                      (UCL([results[rate][run]['Blocking_rate']
                            for run in range(runs_per_rate)]))))
    points.sort(key=lambda x: x[0])
    x = [p[0] for p in points]
    y = [p[1] for p in points]
    plt.plot(x, [p[0] for p in y], marker='o', color='b')
    [plt.errorbar(x[i], y[i][0], yerr=y[i][1], ecolor='r')
        for i in range(len(x))]
    plt.show()
