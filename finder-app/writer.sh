#!/bin/sh

if [ -z $1 ]; then
    echo "No path supplied"
    exit 1
fi

if [ -z $2 ]; then
    echo "No string supplied"
    exit 1
fi

mkdir -p `dirname $1`

if [ -z $? ]; then
    echo "Unable to create directory"
    exit 1
fi

touch $1

if [ -z $? ]; then
    echo "Unable to create file"
    exit 1
fi 

echo $2 > $1
