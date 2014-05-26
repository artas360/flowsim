CC=clang++
#CC=~/intel/bin/icpc
CFLAGS=-std=c++11 -Wall -Wextra -Werror -march=native
LDFLAGS=-lxerces-c
DEBUGFLAGS=-g3 -DTEST
SOURCES=simulation.cpp config.cpp edge.cpp flow.cpp flow_controller.cpp node.cpp random_generator.cpp result.cpp event.cpp topology.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLES=flowsim test_config test_edge test_flow test_flow_controller test_node test_random_generator test_result test_event test_topology

all: flowsim 

test: config flow flow_controller random_generator result event topology

debug: flowsim-debug test

flowsim: $(SOURCES)
		$(CC) $(CFLAGS) -DSIMULATION -DNDEBUG -O3 simulation.cpp $(LDFLAGS) -o flowsim 

flowsim-debug: $(SOURCES)
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -DSIMULATION -DNDEBUG simulation.cpp $(LDFLAGS) -o flowsim

config: config.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -DTEST_CONFIG config.cpp $(LDFLAGS) -o test_config

event: event.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -DTEST_EVENT -DBOOST_DISABLE_THREADS event.cpp -o test_event

flow: flow.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -DTEST_FLOW flow.cpp -o test_flow

flow_controller: flow_controller.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -DTEST_FLOW_CONTROLLER flow_controller.cpp -o test_flow_controller

random_generator: random_generator.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -DTEST_RANDOM_GENERATOR random_generator.cpp -o test_random_generator

result: result.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -DTEST_RESULT result.cpp -o test_result

topology: topology.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -DTEST_TOPOLOGY topology.cpp -o test_topology

clean:
		rm -rf *o $(EXECUTABLES)
