#!/bin/sh
speaker-test -t sine -f 880 -l 1 & sleep 1 && kill -9 $!
