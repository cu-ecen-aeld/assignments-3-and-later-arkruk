#!/bin/bash

if [ $# -eq 0 ]; then
    echo "path and text string not defined"
    exit 1
elif  [ $# -eq 1 ]; then
    echo "text string not defined"
    exit 1
elif [ -d $1 ]; then
    files_number="$(ls $1 | wc -l)"
    files_number="$(grep -r $2 | wc -l)"
    echo "The number of files are $files_number and the number of matching lines are "
    exit 0
else
    echo "$1 is not directory"
    exit 1
fi
