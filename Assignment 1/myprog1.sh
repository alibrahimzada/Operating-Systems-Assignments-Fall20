#!/bin/bash

# first we start by checking if the argument is NULL. If so, we exit with an error.
if [ -z $1 ]
then
    echo "Error - No filename is given"
    exit 1
fi

# we maintain two arrays while handling in bound and out bound values
frequency=(0 0 0 0 0 0 0 0 0 0)
outOfBounds=()

# at each iteration of the loop, we read a number from the file
while read line
do
    # first we check if the number from a line is out of bounds (0 > x > 9)
    if [ $line -lt 0 ] || [ $line -gt 9 ]
    then
        # we append the out bound value to the outOfBounds array
        outOfBounds+=($line)
    else
        # if it is an in bound one, we increment its frequency at index number
        frequency[$line]=`expr ${frequency[$line]} + 1`
    fi
done <<< "$(cat $1)"

# on the screen, first we print the out of bound numbers
for i in ${outOfBounds[@]}
do
    echo "Opps!! $i is out of bounds"
done

# then we print the histograms in a sorted order, that is from 0 - 9
for (( i = 0 ; i < ${#frequency[@]} ; i++ ))
do
    multiplexer=$(printf "%${frequency[$i]}s")
    echo "$i ${multiplexer// /*}"
done
