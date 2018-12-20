#!/bin/bash
# Version 3.00.00b1
# Build 1
# 2015-09-11
# License GPLv3

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash


convert -size 560x85 xc:transparent -font Palatino-Bold -pointsize 72 -fill black -stroke red -draw "text 20,55 'Linux and Life'" linuxandlife.png

