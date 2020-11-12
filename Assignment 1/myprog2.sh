#!/bin/bash

# we first store alphabet in an array and later use their indices for 
# creating the cipher word our script only works with lowercase alphabet
# since handling uppercase is not mentioned in the manual
alphabet=('a' 'b' 'c' 'd' 'e' 'f' 'g' 'h' 'i' 
          'j' 'k' 'l' 'm' 'n' 'o' 'p' 'q' 'r'
          's' 't' 'u' 'v' 'w' 'x' 'y' 'z')

# an empty cipher string at the beginning which gets appended later
# moreover we also store the given arguments in variables
cipheredWord=""
string=$1
number=$2

# a function which finds the index of a specific character
getCharIndex () {
    # loop over each element of array until we find the desired character
    for (( i = 0 ; i < ${#alphabet[@]} ; i++ ))
    do
        # once found, return its index
        if [ ${alphabet[$i]} = $1 ]
        then
            return $i
        fi
    done
}

# this loop is responsible for going over each character in the given argument
# and handle the necessary ciphering processes
for (( c = 0 ; c < ${#string} ; c++ ))
do
    char=${string:$c:1}

    # checking if the length of given number is 1 or equal to the length of string  
    if [ ${#number} = 1 ]
    then
        digit=$number
    else
        digit=${number:$c:1}
    fi
    
    # get the index of the character from the array
    getCharIndex $char
    charIndex=$?
    
    # calculate the step and check if it overflows from 26 (number of alphabets)
    newIdx=`expr $charIndex + $digit`
    newIdx=`expr $newIdx % ${#alphabet[@]}`
    cipheredWord+=${alphabet[$newIdx]}
done

echo $cipheredWord
