#!/bin/sh
# ps.sh

tasklist >win_ps.out; sed -f tasklist.sed win_ps.out | sort
