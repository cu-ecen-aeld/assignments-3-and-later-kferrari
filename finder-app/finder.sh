#!/bin/sh

FILESDIR=$1
SEARCHSTR=$2

if [ -z $1 ]; then
    echo "No path supplied"
    exit 1
fi

if [ -z $2 ]; then
    echo "No search string supplied"
    exit 1
fi

if [ ! -e $1 ]; then
    echo "path $1 does not exist"
fi

TOTAL=`find $1 -type f | wc -l`
LINES=`grep -r $2 $1 | wc -l`

echo "The number of files are $TOTAL and the number of matching lines are $LINES"
