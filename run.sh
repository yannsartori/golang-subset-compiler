#!/bin/bash
MODE=$1
FILE=$2
if ! [ -f $FILE ]; then
	echo "Error: File $FILE does not exist."
	exit 1
fi

if [ "$MODE" = "scan" ] || [ "$MODE" = "tokens" ] || [ "$MODE" = "parse" ] || [ "$MODE" = "pretty" ] || [ "$MODE" = "symbol" ] || [ "$MODE" = "typecheck" ]; then
	./src/golitec "$MODE" < "$FILE"
else
	echo "Error: Invalid mode $MODE"
	exit 1
fi
