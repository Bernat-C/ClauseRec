#!/bin/bash


for ((i=1; i<5; i++)); do
	for ((j=1; j<=10; j++))	do
        ./checkExec.sh $i $j
        echo "$ ------------------------------------" $i $j
    done
done

for ((i=7; i<9; i++)); do
	for ((j=1; j<=10; j++))	do
        ./checkExec.sh $i $j
        echo "$ ------------------------------------" $i $j
    done
done

for ((i=11; i<13; i++)); do
	for ((j=1; j<=10; j++))	do
        ./checkExec.sh $i $j
        echo "$ ------------------------------------" $i $j
    done
done

for ((i=15; i<20; i++)); do
	for ((j=1; j<=10; j++))	do
        ./checkExec.sh $i $j
        echo "$ ------------------------------------" $i $j
    done
done

for ((i=21; i<28; i++)); do
	for ((j=1; j<=10; j++))	do
        ./checkExec.sh $i $j
        echo "$ ------------------------------------" $i $j
    done
done