#!/bin/sh
export G_SLICE=always-malloc
export VALGRIND=1
valgrind --leak-check=full --show-reachable=yes --suppressions=glib.supp --num-callers=50 --gen-suppressions=all "$@" ./indicator-network-service

