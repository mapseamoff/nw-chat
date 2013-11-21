#!/bin/bash

if (( $# != 3 )); then
	echo "ERROR. Usage: ./run.sh <host> <port> <number of clients>"
	exit
fi

for i in $(seq 1 $3); do
	userName="user"$i
	./console_client "$1" "$2" "$userName" 1 &
	if (( i > 600 )); then
		sleep 5
	fi
done

