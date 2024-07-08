#!/bin/bash

if [ $# -eq 0 ]; then
    echo "path and text string not defined"
    exit 1
elif [ $# -eq 1 ]; then
    echo "text string not defined"
    exit 1
else
    echo "$2" > $1
    if [ -f $1 ]; then
        echo "file exist"
    else
        echo "file is not created"
        exit 1
    fi
fi
