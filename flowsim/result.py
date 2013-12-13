#!/usr/bin/python

class Result(object):
    def __init__(self):
        self.results=dict()

    def increase_result(self, key):
        try:
            self.results[key] += 1
        except KeyError:
            self.results[key] = 1

    #TODO : find a better scheme ... to enable node/edge statistics


