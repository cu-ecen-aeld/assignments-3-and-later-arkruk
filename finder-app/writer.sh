#!/bin/bash

if [ $# -eq 0 ]; then
    echo "path and text string not defined"
    exit 1
elif [ $# -eq 1 ]; then
    echo "text string not defined"
    exit 1
else
    dir_name=$(dirname $1)
    mkdir -p $dir_name
    echo "$2" > $1
    if [ -f $1 ]; then
        exit 0
    else
        echo "file is not created"
        exit 1
    fi
fi
