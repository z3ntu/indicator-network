#!/bin/sh
automake --add-missing && \
autoreconf && \
./configure
