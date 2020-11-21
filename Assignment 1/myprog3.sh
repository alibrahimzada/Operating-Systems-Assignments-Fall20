#!/bin/bash

#it tests if there is an input given, if not it finds the oldest file in the current woring dir.
if test -z "$1"
then
	oldestFileInTheCurDir=`find $PWD -type f -printf '%T+ %p\n' | sort | head -n 1`
	#Finds the oldest file in the current working directory.
	#find: searches for files
	#$PWD gives the path of the current working directory.
	#type f: finds only regular files
	#-printf '%T+%p\n': prints the last modification date and time of the file separated by + symbol. %p stands for file name.
	#sort |head -n 1: sorts the output and sends it to head command to display the oldest file. -n 1 stands for one file which is the oldest file.

	IFS='/' read -r -a pathArray <<< "$oldestFileInTheCurDir"
	nameOfTheFile="${pathArray[-1]}"
	#splits the output with "/" symbol to find the name of the oldest file.

	IFS=' ' read -r -a splitSpace <<< "$oldestFileInTheCurDir"
	fullPath="${splitSpace[-1]}"
	#splits the output with " " (space) symbol to find the path of the oldest file.

	read -p "Do you want to delete $nameOfTheFile ? (y/n): " answer
	#asks if the user wants to delete the oldest file or not.
	case $answer in

		[yY] )
			rm  $fullPath
			echo "1 file deleted."
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
	#tests if the given path is valid or not.
	if [ ! -d "$1" ]
	then 
		
		echo "THE PATH IS NOT VALID!"	

	else

		oldestFileInThePath=`find "$1" -type f -printf '%T+ %p\n' | sort | head -n 1`
		#finds the oldest file in the given path.
		


		IFS='/' read -r -a path_Array <<< "${oldestFileInThePath}"
		name_of_the_file="${path_Array[-1]}"
		#splits the output with '/' symbol to find the name of the oldest file.

		IFS=' ' read -r -a splitspace <<< "${oldestFileInThePath}"
		#splits the output with ' ' (space) symbol to find the path of the oldest file.	

		#finds if the given directory has a name contains spaces.
		if [ "${#splitspace[@]}" -eq 2 ]
		then

			full_path="${splitspace[-1]}"
				
		else
			full_path="${splitspace[-2]} ${splitspace[-1]}"
		fi

			
		
		read -p "Do you want to delete $name_of_the_file ? (y/n): " answer1
		#asks if the user wants to delete the oldest file or not.

		case $answer1 in


			[yY] )

				rm  "$full_path"
				echo "1 file deleted."
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