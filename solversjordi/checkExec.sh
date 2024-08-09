#!/bin/bash

cd ..

./solversjordi/bin/release/prcpsp2dimacs ./instances/j30rcp/J30${1}_${2}.RCP -s=maple --use-assumptions=0 --print-nonoptimal=0 -o=ub > ./solversjordi/temp/J30${1}_${2}.custom.out
./solversjordi/bin/release/checkprcpsp -V=1 ./instances/j30rcp/J30${1}_${2}.RCP ./solversjordi/temp/J30${1}_${2}.custom.out

./solversjordi/bin/release/mrcpsp2smt ./instances/j30rcp/J30${1}_${2}.RCP -s=maple --use-assumptions=0 --prcpsp-dummy=1 --encoding=satorder --print-nonoptimal=0 -o=ub > ./solversjordi/temp/J30${1}_${2}.dummy.out
./solversjordi/bin/release/checkprcpsp -V=1 ./instances/j30rcp/J30${1}_${2}.RCP ./solversjordi/temp/J30${1}_${2}.dummy.out

cd solversjordi