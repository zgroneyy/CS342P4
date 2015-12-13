#!/bin/bash

SEED=$2
SIZE=$1
TIMES=100
SUM=0


for i in $(seq 1 1 $TIMES)
do
    RUN=$(./app $SIZE $SEED)
    #echo "$i : $RUN"
    SUM=$(bc -l <<< "$SUM + $RUN")
done

AVG=$(bc -l <<< "$SUM / $TIMES")
echo $AVG