#!/bin/bash

# TODO: add support for given pathname.

getOldestFile () {
    filename=""
    while read line 
    do
        lineArray=($line)
        type=${lineArray[0]}
        mode=${type:0:1}
        if [ $mode = "-" ]
        then
            if [ "$filename" = "" ]
            then
                filename=${lineArray[8]}
            fi

            if [ ${lineArray[8]} -ot $filename ]
            then
                filename=${lineArray[8]}
            fi
        fi    
    done <<< "$(ls -l $1)"
}

if [ -z $1 ]
then
    pathname=""
    getOldestFile $pathname
    oldestFileName=$filename
    read -p "$oldestFileName is the oldest file in your current working directory. Do you want to delete it?: " answer
else
    pathname=$1
fi

case $answer in
    [yY])
        rm $oldestFileName
        echo "1 file deleted!"
        ;;

    [nN])
        echo "You did not delete the file."
        ;;

    *)
        echo "Invalid Input";
        exit 1
        ;;
esac
