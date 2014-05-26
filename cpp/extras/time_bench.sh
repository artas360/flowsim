#!/bin/bash

# Should be run as super user

chrt -f 99 /usr/bin/time -f '\n***\ntime: %E\ncontext switches: %c\nwaits: %w' $1 $2
