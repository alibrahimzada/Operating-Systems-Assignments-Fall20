#!/bin/bash

# program will raise an error if first argument is null
if [ -z $1 ]   
then
    echo "Please provide a file name as an argument!"
    exit 1
fi

# each String representation of digit placed into an array relatively
numbers=(zero one two three four five six seven eight nine)
newtext=""

# defining regular expression for numbers from 0 to 9
re="^[0-9]+$"

# IFS preserves white-spaces and -n1 reads the file character by character
while IFS= read -n1 char
do
  # in the case that char is not a number, add it to 'newtext' directly.
  if ! [[ $char =~ $re ]]
  then
    newtext+=$char
    
  # if it is a number get the corresponding string representation
  # of digit from numbers array
  else
    newtext+=${numbers[$char]}
  fi
done < $1

# Overwriting the newtext to existing file given as argument
echo $newtext > $1
