#!/bin/bash

if [ $# == 1 ]
then
	if [ -z "$1" ]
	then
		echo "A wildcard argument must be entered!"
		exit 1
	else
		mkdir -p copied && cp $1 copied
	fi
else
	if [ $1 != "-R" ]
	then
		echo "-R option is not entered correctly!"
		exit 2
	else
		# a bug here ({}/$2 does not expand the wildcard)
		find . -type d -not -path ".*copied" -exec mkdir -p {}/copied \; -exec cp {}/$2 {}/copied \;
	fi
fi
