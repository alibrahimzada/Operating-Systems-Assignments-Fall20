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

for (( c = 0 ; c < ${#string} ; c++ ))
do
    char=${string:$c:1}
    
    if [ ${#number} = 1 ]
    then
        digit=$number
    else
        digit=${number:$c:1}
    fi
    
    getCharIndex $char
    charIndex=$?
    
    newIdx=`expr $charIndex + $digit`
    newIdx=`expr $newIdx % ${#alphabet[@]}`
    cipheredWord+=${alphabet[$newIdx]}
done

echo $cipheredWord
