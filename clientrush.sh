#!/bin/bash

function rand() {
	echo $((1 + $RANDOM % $1))
}

for i in $(seq 100); do
	./client $(rand $i) $(rand $i) $(rand $i) 9 $i &
done
