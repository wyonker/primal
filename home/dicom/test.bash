#!/bin/bash -x

RECNUM="1"
FNAME=`date +%s`
FNAME=`echo $RECNUM"_"$FNAME"_"$RANDOM.txt`

echo "$FNAME"

