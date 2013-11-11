#!/usr/bin/python

class Node(object):
    def __init__(self, ressource_quantity):
        self.ressource_list = [None for i in xrange(ressource_quantity)] # List of references to the flows using each of the ressources

    def has_free_ressource(self):
        try:
            self.ressource_list.index(None)
        except ValueError:
            return False
        return True


class Switch(Node):
    pass

#TODO later 
class Ressource(object):
    pass

class Port(Ressource):
    pass
