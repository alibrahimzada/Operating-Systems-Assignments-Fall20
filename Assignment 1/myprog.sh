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
     read yourch
     case $yourch in
        1) echo "histogram will be created, press a key. . ." ; read ;;
 	    *) echo "You have entered an invalid number!!! Please enter 1,2,3,4,5 or 6";
           echo "Press a key. . ." ; read ;;
     esac
  done