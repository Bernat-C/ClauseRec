#!/bin/bash

: '
Verifies the solution found for all files that fit the structure J30X_Y.RCP and that have been solved in a specific folder.
Uses checkprcpsp from solversjordi.
'

FOLDER=$1

if [ "$FOLDER" = "" ]; then 
	echo "Usage: bash verify.sh folder" 
	exit
elif ! (test -d "$FOLDER";) then
	echo -e "The folder $FOLDER was not found."
	exit
fi

for ((i=0; i<48; i++)); do
	for ((j=0; j<=10; j++))	do
		FILE=${FOLDER}/J30${i}_${j}.RCP
		SOLUTION=${FOLDER}/J30${i}_${j}.RCP.out
		if test -f "$SOLUTION"; then
		    if test -f "$FILE"; then
				~/pfg/solversjordi/bin/release/checkprcpsp -V=1 $FILE $SOLUTION
		    else
				echo -e "The file $FILE was not found."
		    fi
		fi
	done
done
