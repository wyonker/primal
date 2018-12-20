#!/bin/bash
# Version 3.00.00b12
# Build 1
# 2015-10-29
# License GPLv3

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

ISDUPE=1
PDEPTH=`echo "$PRIPROC"|awk -F "/" '{print NF+2}'`
LIST=`ls -1 $PRIPROC/$DIRNAME/* 2>/dev/null|cut -d "/" -f$PDEPTH`
for i in $LIST
do
	IMAGEDUMP=`/home/dicom/bin/dcmdump $PRIPROC/$DIRNAME/$i|egrep '(0002,0010)|(0008,0018)|(0020,000d)|(0020,000e)'`
	TRANSSYN=`echo "$IMAGEDUMP"|grep "(0002,0010)"|head -1|cut -d "=" -f2|cut -d "#" -f1|tr -s " "`
	SOPIUID=`echo "$IMAGEDUMP"|grep "(0008,0018)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
	SIUID=`echo "$IMAGEDUMP"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
	SERIUID=`echo "$IMAGEDUMP"|grep "(0020,000e)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
	ISFOUND=`echo "select count(*) from image as i left join series as s on i.SERIUID = s.SERIUID where i.puid != '$DIRNAME' and s.SIUID = '$SIUID' and i.SOPIUID = '$SOPIUID';"|mysql primal|tail -1`
	if [ $ISFOUND -eq 0 ]
	then
		ISDUPE=0
	fi
done

if [ $ISDUPE -eq 1 ]
then
	#If we still think this is a duplicate, Let's check tehs status of the study to make sure there was no error.
	STATUS=`echo "select status from patient where puid = '$DIRNAME';"|mysql -u root primal|tail -1`
	if [ "$STATUS" == "ERR" ]
	then
		#There was an error so this never got sent.  Can't be a duplicate then
		ISDUPE=0
	fi
fi

if [ $ISDUPE -eq 1 ]
then
	exit 95
else
	exit 0
fi
