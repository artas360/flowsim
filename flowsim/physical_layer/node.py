

class Node(object):
    counter = 0
    def __init__(self, number=-1, name=''):
        self.number = number if number >= 0 else Node.counter
        self.name = name if name != '' else str(id(self))
        Node.counter = self.number + 1

    def __int__(self):
        return self.number

    def get_name(self):
        return self.name


class Entry_node(Node):
    pass


class Exit_node(Node):
    pass

