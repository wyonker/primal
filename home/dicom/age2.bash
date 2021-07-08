#!/bin/bash
# Version 3.03.01
# Build 2
# 2020-03-23
# License GPLv3

# This script should be passed a directory containing DICOM files that are to be archived.

RECNUM=`echo "$2"|rev|cut -d "/" -f1|rev|cut -d "_" -f1`
PRIMALID=`echo "$2"|rev|cut -d "/" -f1|rev`
THISDATE=`date "+%Y/%m/%d/%H"`
THISMDATE=`date "+%Y-%m-%d %H:%M:%S"`
THISTYPE=$1
THISDIR=$2

set -- $RECNUM
source readconf.bash
if [ "$THISTYPE" == "a" ] || [ "$THISTYPE" == "A" ]
then
	if [ "$PRIARCHTYPE0" == "disk" ]
	then
		#Move the directory and update the database
		mv $THISDIR $PRIARCHHIP0/$THISDATE/$PRIMALID/
		logger -t PRIMAL "`date`  INFO:  Archiving $THISDIR to $PRIARCHHIP0/$THISDATE/"
		echo 'update image set ilocaiton = "'$PRIARCHHIP0/$THISDATE/$PRIMALID/'", idate = "'$THISMDATE'" where puid = "'$PRIMALID'";'|$DBCONNN
	elif [ "$PRIARCHTYPE0" == "S3" ]
	then
		aws s3 mv $THISDIR/* "s3://$PRIARCHHIP0/$THISDATE/$PRIMALID/" --no-progress > /dev/null
		logger -t PRIMAL "`date`  INFO:  Archiving $THISDIR to S3"
		echo 'update image set ilocaiton = "'$PRIARCHHIP0/$THISDATE/$PRIMALID/'", idate = "'$THISMDATE'" where puid = "'$PRIMALID'";'|$DBCONNN
	fi
fi
if [ "$THISTYPE" == "d" ] || [ "$THISTYPE" == "D" ]
then
	logger -t PRIMAL "`date`  INFO:  Pruning $THISDIR"
	echo 'update image set ilocation = NULL, idate = "'$THISMDATE'" where puid = "'$PRIMALID'";'|$DBCONNN
	rm -fr $THISDIR
fi
