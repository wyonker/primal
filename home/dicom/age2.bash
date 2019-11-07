#!/bin/bash
# Version 3.02.02
# Build 1
# 2018-12-26
# License GPLv3

# This script should be passed a directory containing DICOM files that are to be archived.

RECNUM=`echo "$2"|rev|cut -d "/" -f1|rev|cut -d "_" -f1`
PRIMALID=`echo "$2"|rev|cut -d "/" -f2|revi`
THISDATE=`date "+%Y/%m/%d/%H"`
THISMDATE=`date "+%Y-%m-%d %H:%M:%S"`

source readconf.bash

if [ "$PRIARCHTYPE0" == "disk" ]
then
	#Move the directory and update the database
	mv $2 $PRIARCHHIP0/$THISDATE/$PRIMALID/
	echo 'update image set ilocaiton = "'$PRIARCHHIP0/$THISDATE/$PRIMALID/'", idate = "'$THISMDATE'" where puid = "'$PRIMALID'";'|$DBCONNN
elif [ "$PRIARCHTYPE0" == "S3" ]
then
	aws s3 mv $2/* "s3://$PRIARCHHIP0/$THISDATE/$PRIMALID/" --no-progress > /dev/null
	echo 'update image set ilocaiton = "'$PRIARCHHIP0/$THISDATE/$PRIMALID/'", idate = "'$THISMDATE'" where puid = "'$PRIMALID'";'|$DBCONNN
fi
