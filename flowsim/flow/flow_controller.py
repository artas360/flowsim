from flowsim.flow.flow import Flow
from flowsim.physical_layer.edge import Edge
from flowsim.flowsim_exception import NoPathError,\
    NotRegisteredFlow,\
    RessourceAllocationError


class Flow_controller(object):
    def __init__(self, topology, ghost_topo, event_manager, simulation):
        self.topology = topology
        self.ghost_topo= ghost_topo
        self.flows = []

    def allocate_flow(self, node1, node2):
        if node1 is None or node2 is None:
            raise TypeError
        try:
            nodes = self.topology.shortest_path(node1, node2)
        except NoPathError:
            raise NoPathError()
        flow = Flow(nodes)

        for i in xrange(len(nodes)-1):
            self.topology.allocate_flow(nodes[i], nodes[i+1], flow)

        self.flows.append(flow)
        return flow

    def free_flow(self, flow):
        try:
            self.flows.remove(flow)
        except:
            raise NotRegisteredFlow()
        nodes = flow.get_nodes()
        for i in xrange(len(nodes) - 1):
            self.topology.free_edge(nodes[i], nodes[i + 1], flow)

    def get_entry_nodes(self):
        return self.topology.get_entry_nodes()

    def get_topology(self):
        return self.topology

    def disconnect_edge(self, src, dst):
        self.topology.remove_edge(src, dst, force=True)
        return 1

    def reconfigure_topology(self, results, treshold):
        # TODO: switch on algorithm
        print
        for node1, node2 in self.topology.edges():
            print int(node1), "->", int(node2), " : ", self.edge_load(node1, node2)
        self.simple_reconfigure(treshold)
        for node1, node2 in self.topology.edges():
            print int(node1), "->", int(node2), " : ", self.edge_load(node1, node2)

    def simple_reconfigure(self, treshold):
        edge_loads = dict()
        for src, dst, data in self.topology.edges_iter(data=True):
            edge_loads[(src, dst)] = self.edge_load(src, dst)

        modified_topology = True
        overloaded_edge_list =\
            self.overloaded_edges(edge_loads, treshold)
        underloaded_edge_list =\
            self.underloaded_edges(edge_loads, treshold)

        for src, dst in overloaded_edge_list:
            if modified_topology:
                overloaded_edge_list =\
                    self.underloaded_edges(edge_loads, treshold)
                modified_topology = False

            # Easy case: add direct edge
            if(src.available_tx() and dst.available_rx()):
                self.topology.add_edge(src, dst,
                                       self.ghost_topo[src][dst]["object"].
                                       copy())
                print "Adding_edge"
                continue
            
            backup_weight = self.ghost_topo[src][dst]['weight']
            self.ghost_topo[src][dst]['weight'] = Edge.infinite_weight
            close_paths = self.ghost_topo.all_shortest_paths(src, dst)
            self.ghost_topo[src][dst]['weight'] = backup_weight 

            # No direct edge available, try to add edge close.
            for path in close_paths:
                if self.add_edge_along(path, underloaded_edge_list) != 0:
                    modified_topology = True
                    print "Adding close edge"
                    break

            if(modified_topology):
                continue

            # If we couldn't allocate any edge so far, try to dealocate one
            for path in close_paths:
                if self.break_edge_along(path, underloaded_edge_list) != 0:
                    modified_topology = True
                    print "breaking edge"
                    assert(self.add_edge_along(path) != 0)
                    print "adding edge"
                    break

    def add_edge_along(self, path, underloaded_edge_list):
        # Edges in the path but not in real topology
        ghost_edges = [(path[i], path[i + 1]) for i in xrange(len(path) - 1)
                       if not self.topology.has_edge(path[i], path[i + 1])]
        if len(ghost_edges) == 0:
            for i in xrange(len(path) - 1):
                if (path[i].available_tx() and path[i + 1].available_rx()):
                    print "adding edge: ", int(path[i]), int(path[i+1])
                    self.topology.add_edge(path[i], path[i + 1],
                                           self.ghost_topo[path[i]]\
                                           [path[i + 1]]["object"].copy())
                    return 1
            return 0
        else:
            # Trying to add a ghost edge
            for src, dst in ghost_edges:
                if (src.available_tx() and dst.available_rx()):
                    self.topology.add_edge(src, dst,
                                           self.ghost_topo[src][dst]["object"].
                                           copy())
                    return 1
            # The path still contains a ghost edge
            if not src.available_tx():
                for edge in self.topology.out_edges(src):
                    if edge in underloaded_edge_list:
                        if(self.disconnect_edge(edge[0], edge[1]) == 1):
                            print "Breaking_edge ", int(edge[0]), int(edge[1])
                            return 1
            if not dst.available_rx():
                for edge in self.topology.in_edges(dst):
                    if edge in underloaded_edge_list:
                        if(self.disconnect_edge(edge[0], edge[1]) == 1):
                            print "Breaking_edge ", int(edge[0]), int(edge[1])
                            return 1
            # Trying to add a ghost edge
            for src, dst in ghost_edges:
                if (src.available_tx() and dst.available_rx()):
                    self.topology.add_edge(src, dst,
                                           self.ghost_topo[src][dst]["object"].
                                           copy())
                    return 1
            return 0

    def break_edge_along(self, path, underloaded_edge_list):
        for i in xrange(len(path) - 1):
            print "checking : ", int(path[i]), int(path[i+1])
            for edge in self.topology.out_edges(path[i]):
                if edge[1] != path[i + 1]:
                    if (edge) in underloaded_edge_list:
                        if(self.disconnect_edge(edge[0], edge[1]) == 1):
                            print "Breaking_edge ", int(edge[0]), int(edge[1])
                            return 1
        return 0

    def underloaded_edges(self, edge_load_map, treshold):
        res = []
        mean = float(sum(edge_load_map.values())) / len(edge_load_map)
        for edge in edge_load_map:
            if edge_load_map[edge] - mean < -treshold:
                res.append(edge)
        return res

    def overloaded_edges(self, edge_load_map, treshold):
        res = []
        mean = float(sum(edge_load_map.values())) / len(edge_load_map)
        print "avg load: ", mean
        for edge in edge_load_map:
            if edge_load_map[edge] - mean > treshold:
                res.append(edge)
        return res

    def edge_load(self, src, dst):
        tot_capacity = sum(out_edge[2]['object'].get_capacity() for
                           out_edge in self.topology.out_edges(src, data=True))
        return src.arrival_rate / (tot_capacity * dst.service_rate) 

