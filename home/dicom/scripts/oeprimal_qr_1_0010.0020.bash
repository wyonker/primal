#!/bin/bash -x

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

PDEPTH=`echo "$PRIPROC"|awk -F "/" '{print NF+2}'`
FILENAME=`ls -1 $2|head -1|tail -1`
IMAGEDUMP=`/home/dicom/bin/dcmdump $PRIPROC/$DIRNAME/$FILENAME|egrep '(0002,0010)|(0008,0018)|(0008,0050)|(0010,0010)|(0010,0020)|(0010,0030)|(0010,4000)|(0020,000d)|(0020,000e)'`
TRANSSYN=`echo "$IMAGEDUMP"|grep "(0002,0010)"|head -1|cut -d "=" -f2|cut -d "#" -f1|tr -s " "`
SOPIUID=`echo "$IMAGEDUMP"|grep "(0008,0018)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
ACCN=`echo "$IMAGEDUMP"|grep "(0008,0050)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PNAME=`echo "$IMAGEDUMP"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
MRN=`echo "$IMAGEDUMP"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PDOB=`echo "$IMAGEDUMP"|grep "(0010,0030)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PATIENTCOMMENTS=`echo "$IMAGEDUMP"|grep "(0010,4000)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SIUID=`echo "$IMAGEDUMP"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SERIUID=`echo "$IMAGEDUMP"|grep "(0020,000e)"|head -1|cut -d "[" -f2|cut -d "]" -f1`

STARTDATE="`date -d "yesterday" +%Y-%m-%d` 00:00:00"
ENDATE="`date +%Y-%m-%d` 23:59:59"

#Check to see if the patientcomments made it into the database
RESULTS=`echo "select PatientComments from patient where puid = '$DIRNAME';"|$DBCONNN`

if [ "$RESULTS" == "" ] || [ "$RESULTS" == " " ]
then
	if [ "$PATIENTCOMMENTS" != "" ] && [ "$PATIENTCOMMENTS" != " " ]
	then
		RESULTS=`echo "update patient set PatientComments='$PATIENTCOMMENTS' where puid = '$DIRNAME';"|$DBCONNN`
	else
		#Lets check all the files and see if any of them have a patientcomment
		DONE=0
		LIST=`ls -1 $PRIPROC/$DIRNAME/* 2>/dev/null|cut -d "/" -f$PDEPTH`
		for i in $LIST
		do
			if [ $DONE -eq 0 ]
			then
				TEMPVAR=`dcmdump $i|grep "(0010,4000)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
				if [ "$TEMPVAR" != "" ] && [ "$TEMPVAR" != "" ]
				then
					RESULTS=`echo "update patient set PatientComments='$TEMPVAR' where puid = '$DIRNAME';"|$DBCONNN`
					DONE=1
				fi
			fi
		done
	fi
fi

PATIENTCOMMENTS=`echo "$PATIENTCOMMENTS"|tr -s " "`

if [ "$PATIENTCOMMENTS" == "" ] || [ "$PATIENTCOMMENTS" == " " ]
then
	echo "ERROR"
	exit 95
else
	echo "$PATIENTCOMMENTS"
	exit 0
fi
