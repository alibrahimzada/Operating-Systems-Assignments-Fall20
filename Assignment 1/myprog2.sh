#!/bin/bash

alphabet=('a' 'b' 'c' 'd' 'e' 'f' 'g' 'h' 'i' 
          'j' 'k' 'l' 'm' 'n' 'o' 'p' 'q' 'r'
          's' 't' 'u' 'v' 'w' 'x' 'y' 'z')

cipheredWord=""
string=$1
number=$2

getCharIndex () {
    for (( i = 0 ; i < ${#alphabet[@]} ; i++ ))
    do
        if [ ${alphabet[$i]} = $1 ]
        then
            return $i
        fi
    done
}

for (( i = 0 ; i < ${#string} ; i++ ))
do
    char=${string:$i:1}
    # echo $char
    # when there are only these two above lines, the loop runs fine.
    getCharIndex $char
    charIndex=$?
    echo $charIndex
done
