class NoPathError(Exception):
    pass


class NotRegisteredFlow(Exception):
    pass


class RessourceAllocationError(Exception):
    pass


class NoSuchEdge(Exception):
    pass


class NoSuchNode(Exception):
    pass


class DuplicatedNodeError(Exception):
    pass


class DuplicatedEdgeError(Exception):
    pass


class LoopError(Exception):
    pass


class EdgeAllocationError(Exception):
    pass


class WrongConfig(Exception):
    pass
