#!/bin/bash

# in main menu, we enter an inifinte loop until the user ends the program
while :
do
   # first we clear the terminal and show the available menu items
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

   # then we read the user input. Some choices take optional arguments, so we assume the user 
   # will enter RETURN without typing anything when prompted by the program
   read choice
   case $choice in
      # if the user enters 1, we first take an argument, and then run the corresponding program
      1) read -p "Please enter the filename: " filename1 ;
         ./myprog1.sh $filename1 ; read -p "Press Enter to go to Main Menu..." ;;

      # if the user enters 2, we take 2 arguments and then run the corresponding program
      2) read -p "Please enter the string: " string ; read -p "Please enter the number: " number ;
         ./myprog2.sh $string $number ; read -p "Press Enter to go to Main Menu..." ;;

      # if the user enters 3, we first take an argument, and then run the corresponding program
      3) read -p "Please enter a path name: " pathName ;
         ./myprog3.sh $pathName ; read -p "Press Enter to go to Main Menu..." ;;
      
      # if the user enters 4, we first take an argument, and then run the corresponding program
      4) read -p "Please enter the filename: " filename4 ;
         ./myprog4.sh $filename4 ; read -p "Press Enter to go to Main Menu..." ;;

      # if the user enters 5, we first take 2 arguments, and then run the corresponding program
      5) read -p "Please enter -R option, if any: " rOption ;
         read -p "Please enter a wildcard: " wildCard ;
         if [ -z $rOption ]
         then
            ./myprog5.sh "$wildCard" ; read -p "Press Enter to go to Main Menu..."
         else
            ./myprog5.sh $rOption "$wildCard" ; read -p "Press Enter to go to Main Menu..."
         fi
         ;;

      # we exit when user enters 6
      6) exit 0 ;;

      # if the entered menu item is not valid, we let the user know about it
      *) echo "You have entered an invalid number!!! Please enter one of 1,2,3,4,5 or 6";
         read -p "Press Enter to go to Main Menu..." ;;
   esac
done
