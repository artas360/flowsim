#!/usr/bin/python

class NodeAllocationError(Exception):
    pass

class Node(object):
    counter = 0
    def __init__(self, number=-1):
        self.number = number if number >= 0 else Node.counter
        Node.counter=self.number+1

    def __int__(self):
        return self.number


class Switch(Node):
    pass

class Input_node(Node):
    pass

class Output_node(Node):
    pass

#TODO later 
class Ressource(object):
    pass

class Port(Ressource):
    pass
