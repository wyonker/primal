#!/bin/bash -x

if [ "$1" == "" ]
then
	echo "Usage:  primalrm {directory name}"
	exit 1
fi

if [ "$1" == "-h" ]
then
	echo "primalrm - used to remove a directory containing images there were received by PRIMAL and their database pointers."
	echo "Usage:  primalrm {directory name}"
	exit 0
fi

STARTSRIGHT=`echo "$1"|grep "^-d"|wc -l`
if [ $STARTSRIGHT -ne 1 ]
then
    echo "Usage:  primalrm -d{directory name}"
    exit 1
fi

HASWILDCARD=`echo "$1"|grep -e "\*" -e "\?"|wc -l`
if [ $HASWILDCARD -gt 0 ]
then
	echo "Wildcard characters are not allowed."
    echo "Usage:  primalrm -d{directory name}"
    exit 1
fi

INDIR=`echo "$1"|cut -c3-`

if [ ! -e "$INDIR" ]
then
	echo "Usage:  primalrm {directory name}"
    exit 1
fi

if [ `whoami` != "root" ]
then
	echo "$0 must be run as root."
    echo "Usage:  primalrm -d{directory name}"
    exit 1
fi

ENDSLASH=`echo "$INDIR"|grep "/$"|wc -l`
if [ $ENDSLASH -eq 1 ]
then
	INCHR=`echo "$INDIR"|awk '{ print length($0); }'`
	let INCHR=$INCHR-1
	INDIR=`echo "$INDIR"|cut -c-$INCHR`
fi

TEMPVAR=`echo $INDIR|tr "/" "\n"|wc -l`
DIRNAME=`echo $INDIR|cut -d "/" -f $TEMPVAR`
let TEMPVAR=$TEMPVAR+1
ISERVERNAME=`hostname`

LIST=`ls -1 $INDIR/* 2>/dev/null|cut -d "/" -f$TEMPVAR`
for i in $LIST
do
	SOPIUID=`dcmdump $INDIR/$i|grep "0008,0018"|cut -d "[" -f2|cut -d "]" -f1|tr -s " "`
	ILOCATION=`echo "select ilocation from image where SOPIUID='$SOPIUID' and puid='$DIRNAME' and iservername='$ISERVERNAME';"|$DBCONNN`
	if [ "$ILOCATION" != "NULL" ]
	then
		echo "update image set ilocation = NULL where SOPIUID='$SOPIUID' and puid='$DIRNAME' and iservername='$ISERVERNAME';"|$DBCONNN
	fi
	ILOCATION=`echo "select ilocation from image where SOPIUID='$SOPIUID' and puid='$DIRNAME' and iservername='$ISERVERNAME';"|$DBCONNN`
	if [ "$ILOCATION" == "NULL" ]
	then
		rm -f $INDIR/$i
	fi
done

LIST=`ls -1 $INDIR/*|wc -l`
if [ $LIST -eq 0 ]
then
	rm -fr $INDIR
else
	echo "Sometingwong!!!"
	exit 1
fi
