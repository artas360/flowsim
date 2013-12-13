#!/usr/bin/python

class Flow(object):
    def __init__(self, edge_list):
        self.edge_list = edge_list

    def get_edges(self):
        return self.edge_list
