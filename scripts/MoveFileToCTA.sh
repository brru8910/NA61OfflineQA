#!/bin/bash

if [ "$#" -ne 2 ]
then
    echo "Incorrect amount of input arguments. Usage: MoveFileToCTA.sh [pathToFile] [CTADirectory (with prefix)]"                         
    exit 1
fi

#Transfer arguments.
inputFile=$1
inputDirectory=$2

#Copy.
xrdcp $inputFile  $inputDirectory

if [ $? -ne 0 ]
then
  echo "[ERROR] xrdcp of file "$inputDirectory/$inputFile" failed!"
  exit 1
fi

#Remove file.
rm $inputFile

exit 0
