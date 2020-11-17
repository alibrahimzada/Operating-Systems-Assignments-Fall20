#!/bin/bash
while :
  do
     clear
     echo ""
     echo " Please select an option: "
     echo ""
     echo "1. Create histogram"
     echo "2. Encryption"
     echo "3. Delete oldest"
     echo "4. Convert numbers"
     echo "5. Organized files"
     echo "6. Exit"
     echo -n "Enter your menu choice [1-6]: "
     read choice
     case $choice in
        1) read -p "Please enter the filename: " filename1 ;
           ./myprog1.sh $filename1 ; read -p "Press Enter to go to Main Menu..." ;;

        2) read -p "Please enter the string: " string ; read -p "Please enter the number: " number ;
           ./myprog2.sh $string $number ; read -p "Press Enter to go to Main Menu..." ;;

        3) read -p "Please enter a path name: " pathName ;
           ./myprog3.sh $pathName ; read -p "Press Enter to go to Main Menu..." ;;
         
        4) read -p "Please enter the filename: " filename4 ;
           ./myprog4.sh $filename4 ; read -p "Press Enter to go to Main Menu..." ;;

        5) read -p "Please enter -R option, if any: " rOption ; 
           read -p "Please enter a wildcard: " wildCard ;
           ./myprog5.sh $rOption $wildCard ; read -p "Press Enter to go to Main Menu..." ;;

        6) exit 0 ;;

 	     *) echo "You have entered an invalid number!!! Please enter one of 1,2,3,4,5 or 6";
           read -p "Press Enter to go to Main Menu..." ;;
     esac
  done