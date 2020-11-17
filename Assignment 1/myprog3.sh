#!/bin/bash

if test -z "$1"
then
	oldestFileInTheCurDir=`find $PWD -printf '%T+ %p\n' | sort | head -n 1`

	IFS='/' read -r -a pathArray <<< "$oldestFileInTheCurDir"
	nameOfTheFile="${pathArray[-1]}"

	IFS=' ' read -r -a splitSpace <<< "$oldestFileInTheCurDir"
	fullPath="${splitSpace[-1]}"

	read -p "Do you want to delete $nameOfTheFile ? (y/n): " answer
	case $answer in

		[yY] )
			rm  $fullPath
			;;

		[nN] )

			echo "You did not delete the file."
			;;
		*)
			echo "Invalid Input";
			exit 1
			;;
	esac


else
	if [ ! -d "$1" ]
	then 
		echo "THIS PATH IS NOT VALID!"
	else
		oldestFileInThePath=`find $1 -printf '%T+ %p\n' | sort | head -n 1`
		IFS='/' read -r -a path_Array <<< "${oldestFileInThePath}"
		name_of_the_file="${path_Array[-1]}"

		IFS=' ' read -r -a splitspace <<< "${oldestFileInThePath}"
		full_path="${splitspace[-1]}"



		read -p "Do you want to delete $name_of_the_file ? (y/n): " answer1
		case $answer1 in

			[yY] )

				rm  $full_path
				;;

			[nN] )

				echo "You did not delete the file."
				;;
			*)
				echo "Invalid Input";
				exit 1
				;;
		esac
	fi
fi