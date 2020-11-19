#!/bin/bash

# first we start by determining the number of arguments given by the user
if [ $# == 1 ]
then
	# if it is 1, we enter this block and then check if a wildcard is given
	if [ -z "$1" ]
	then
		# if a wildcard is not given, we exit with an error
		echo "A wildcard argument must be entered!"
		exit 1
	else
		# otherwise, we create a copied dir and copy eligible files to it ONLY in the present working directory
		eligibleFiles=($1)
		if [ ${eligibleFiles[0]} != "$1" ]
		then
			mkdir -p copied && cp $1 copied
		else
			echo "Sorry, no files could match with your wildcard"
		fi
	fi
else
	# if the number of arguments is more than 1, we assume an -R option will be given by the user
	# first we start to validate that the first argument is indeed an -R (case sensitive)
	if [ $1 != "-R" ]
	then
		# if its not given as -R, we exit the program with an error
		echo "-R option is not entered correctly!"
		exit 2
	else
		# if an -R option along with a wildcard is provided correctly, we do the following:
		# 1) find all directories and subdirectories living under present working directory
		# 2) loop over each one of them, and change directory to the loop variable
		# 3) validate there is at least 1 file matching with wildcard
		# 4) if so, we create a dir named copied and copy the matching files
		find "$(pwd)" -type d | while read dir; 
								do 
									cd "$dir";
									eligibleFiles=($2)
									if [ ${eligibleFiles[0]} != "$2" ]
									then
										mkdir -p copied && cp $2 copied;
									else
										echo "Sorry, no files could match with your wildcard"
									fi
								done
	fi
fi
