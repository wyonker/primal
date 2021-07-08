#!/bin/bash
# Version 3.30.00
# Build 9
# 2020-04-14
# License GPLv3

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

if [ -e /tmp/$DIRNAME.bash ]
then
	rm -f /tmp/$DIRNAME.bash
fi

STARTPROCTIME=`date +%s`
STRTEMP1="`cat /tmp/$DIRNAME` $STARTPROCTIME"
echo "$STRTEMP1" > /tmp/$DIRNAME
FILENAME=`ls -1 $PRIPROC/$DIRNAME/*.dcm|head -1`
SERIESINFO=`dcmdump $FILENAME|egrep '(0010,0010)|(0010,0020)|(0020,000d)'`
PNAME=`echo "$SERIESINFO"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PAPID=`echo "$SERIESINFO"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SIUID=`echo "$SERIESINFO"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
NUMFILES=`ls -1 $PRIPROC/$DIRNAME/*|wc -l`
SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
echo "`date` Started processing study for Patient: $PNAME MRN: $PAPID with $NUMFILES images." >> $PRILOGDIR/$PRILFPROC
echo "insert into process (puid, pservername, tstartproc) values ('$DIRNAME', '`hostname`', '`date +"%Y-%m-%d %H:%M:%S"`');"|$DBCONN

if [ "$PRIQRREC" != "" ]
then
	/home/dicom/qr.bash $1 $2
	if [ $? -eq 95 ]
    then
        echo "`date` ERROR:  Q/R code returned an error for patient $PNAME MRN $PAPID PRIMAL $DIRNAME.  Moving to error..." >> $PRILOGDIR/$PRILFPROC 2>&1
        echo "`date` ERROR:  Q/R code returned an error for patient $PNAME MRN $PAPID PRIMAL $DIRNAME.  Moving to error..." > $PRIPROC/$DIRNAME/error.txt 2>&1
        ISERROR=1
        exit 0
    fi
else
	echo "`date` Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images Q/R skipped PRIQRREC=$PRIQRREC"  >> $PRILOGDIR/$PRILFPROC
fi

if [ $PRIDUPE -eq 1 ]
then
	/home/dicom/dupe_check.bash $1 $2
	if [ $? -eq 95 ]
	then
		echo "`date` Warning:  All images for SIUID $SIUID ($PNAME) have been routed before.  Will not route study and will move it to a holding area..." >> $PRILOGDIR/$PRILFPROC 2>&1
		ISERROR=1
		mv $2 $PRIHOLD/ >> $PRILOGDIR/$PRILFPROC 2>&1
		echo "update image set ilocation = '$PRIHOLD/$DIRNAME' where PUID = '$DIRNAME';"|$DBCONN
		exit 0
	fi
else
	echo "`date` Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images, duplication check  skipped PRIDUPE=$PRIDUPE"  >> $PRILOGDIR/$PRILFPROC
fi

if [ $PRIPREFIX -gt 0 ]
then
	/home/dicom/rename.bash $1 $2
	if [ $? -eq 95 ]
	then
		echo "`date` Error:  There was an error in DICOM tag modification.  Study moved to error!"  >> $PRILOGDIR/$PRILFPROC 2>&1
	fi
else
	echo "`date` Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images, image tag modification skipped PRIPREFIX=$PRIPREFIX" >> $PRILOGDIR/$PRILFPROC
fi

if [ $PRICDCR -gt 0 ]
then
	/home/dicom/cdcr.bash $1 $2
	if [ $? -eq 95 ]
	then
		echo "`date` Error:  There was an error in compression.  Study may not be sent!" >> $PRILOGDIR/$PRILFPROC 2>&1
	fi
else
	echo "`date` Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images, Compression/decompression skipped PRICDCR=$PRICDCR" >> $PRILOGDIR/$PRILFPROC
fi

PROCERROR=`echo "select count(*) from process where puid='$DIRNAME' and perror='1';"|$DBCONNN|tail -1`

if [ -e $PRIPROC/$DIRNAME/error.txt ]
then
	PROCERROR=1
fi

if [ $PROCERROR -eq 1 ]
then
	mv $PRIPROC/$DIRNAME $PRIERROR >> $PRILOGDIR/$PRILFPROC
	echo "update image set ilocation = '$PRIERROR/$DIRNAME' where PUID = '$DIRNAME';"|$DBCONN

	SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
	echo "`date` Processing failed for $DIRNAME.  Study moved to $PRIERROR/$DIRNAME."  >> $PRILOGDIR/$PRILFPROC
	echo "update process set tendproc='`date +"%Y-%m-%d %H:%M:%S"`', perror='1' where process.puid='$DIRNAME';"|$DBCONN
else
	ENDPROCTIME=`date +%s`
	STRTEMP1="`cat /tmp/$DIRNAME` $ENDPROCTIME"
	echo "$STRTEMP1" > /tmp/$DIRNAME
	PROCTIME=`echo "$ENDPROCTIME-$STARTPROCTIME"|bc 2>/dev/null`
	IMGSEC=`echo "scale=2; $NUMFILES/$PROCTIME"|bc 2>/dev/null`
	echo "`date` Finished processing study for PUID: $DIRNAME  Patient: $PNAME MRN: $PAPID with $NUMFILES images in $PROCTIME seconds @ $IMGSEC image(s)/sec." >> $PRILOGDIR/$PRILFPROC
	SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
	echo "update process set tendproc='`date +"%Y-%m-%d %H:%M:%S"`', perror='0' where puid='$DIRNAME';"|$DBCONN
	mv $PRIPROC/$DIRNAME $PRIOUT/ >> $PRILOGDIR/$PRILFPROC
	echo "update image set ilocation = '$PRIOUT/$DIRNAME' where PUID = '$DIRNAME';"|$DBCONN

	#If this is a QR receiver, we need to update the QR table
	INTQR=`echo "select count(*) from QR where SIUID='$SIUID' and puid='';"|$DBCONNN`
	if [ $INTQR -gt 0 ]
	then
		TEMPPUID=`echo "select PatientPUID from QR where SIUID='$SIUID' and puid='' limit 1;"|$DBCONNN`
		TEMPDATE=`date +"%Y-%m-%d %H:%M:%S"`
		echo "update QR set puid='$DIRNAME', RetrieveDate='$TEMPDATE' where SIUID='$SIUID' and PatientPUID='$TEMPPUID';"|$DBCONNN
	fi

	#nohup /home/dicom/send.bash $1 $PRIOUT/$DIRNAME & >> $PRILOGDIR/$PRILFOUT 2>&1
	mq send /prim_send $DIRNAME
fi

rm -f /tmp/$DIRNAME.bash
rm -f /tmp/$DIRNAME.txt
