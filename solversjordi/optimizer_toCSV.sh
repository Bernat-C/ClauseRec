#!/bin/bash
: '
Iterates over all files in a folder that fit the structure J30X_Y.RCP.out/.err, have been executed and a solution has been found
and creates a CSV that contains four columns: X, Y, makespan, solving_time
'

FOLDER=$1

if [ "$FOLDER" = "" ]; then 
	echo "Usage: bash verify.sh folder" 
	exit
elif [[ ! -d "$FOLDER" ]]; then
	echo -e "The folder $FOLDER was not found."
	exit
fi

OUT="../solved/solutions_$(basename $(dirname $FOLDER))_$(basename $FOLDER).csv"
touch "$OUT";

for ((i=1; i<=48; i++)); do
	for ((j=1; j<=10; j++))	do

		ERR=${FOLDER}/J30${i}_${j}.RCP.err
		SOLUTION=${FOLDER}/J30${i}_${j}.RCP.out
		
		if test -f "$SOLUTION"; then
			(
			
			printf '%s,' "$i" "$j"

			while IFS= read -r p || [ -n "$p" ]; do
				printf '%s,' "$p"
			done < $ERR | sed 's/^.*c Time://'

			while IFS= read -r p || [ -n "$p" ]; do
				printf '%s ' "$p"
			done < "$SOLUTION" | (grep -oP "s OPTIMUM FOUND o\s+\K\w+" || echo -1);

			) >> "$OUT"
		fi
	done
done
