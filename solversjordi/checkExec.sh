#!/bin/bash

./bin/release/prcpsp2dimacs ~/pfg/instances/j30rcp/J30${1}_${2}.RCP -s=maple --use-assumptions=0 --print-nonoptimal=0 -o=ub > temp.out
./bin/release/checkprcpsp -V=1 ../instances/j30rcp/J30${1}_${2}.RCP temp.out

./bin/release/mrcpsp2smt ~/pfg/instances/j30rcp/J30${1}_${2}.RCP -s=maple --use-assumptions=0 --prcpsp-dummy=1 --encoding=satorder --print-nonoptimal=0 -o=ub > temp.out
./bin/release/checkprcpsp -V=1 ../instances/j30rcp/J30${1}_${2}.RCP temp.out