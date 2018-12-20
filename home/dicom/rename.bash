#!/bin/bash
# Version 3.00.00b14
# Build 7
# 2015-11-125
# License GPLv3

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

if [ -e /tmp/$DIRNAME.bash ]
then
    rm -f /tmp/$DIRNAME.bash
fi

FILENAME=`ls -1 $PRIPROC/$DIRNAME/*|head -1`
SERIESINFO=`dcmdump $FILENAME|egrep '(0010,0010)|(0010,0020)|(0020,000d)'`
PNAME=`echo "$SERIESINFO"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PAPID=`echo "$SERIESINFO"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SIUID=`echo "$SERIESINFO"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
NUMFILES=`ls -1 $PRIPROC/$DIRNAME/*|wc -l`
echo "`date` Started modifications for Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images."  >> $PRILOGDIR/$PRILFPROC
#Need to process any custom scripting for tags here.
#This will call the custom file that has a name in the format of
#<receiver #>_<tag id>.bash
#Where the , in the tag id is replaced with a .
if [ ! -e $PRIPROC/$DIRNAME/error.txt ]
then
	INTLC=1
	PDEPTH=`echo "$PRIPROC"|awk -F "/" '{print NF+2}'`
	LIST=`ls -1 $PRIPROC/$DIRNAME/* 2>/dev/null|cut -d "/" -f$PDEPTH`
	for i in $LIST
	do
		echo "PRIMAL: $DIRNAME Patient: $PNAME MRN: $PAPID Queueing file $INTLC of $NUMFILES" >> $PRILOGDIR/$PRILFPROC
		/home/dicom/rename2.bash $1 $2 $i &
		let INTLC=$INTLC+1
		STILLRUN=`ps -ef|grep rename2.bash|grep $DIRNAME|grep -v grep|wc -l`
		while [ $STILLRUN -gt 250 ]
		do
			sleep 3
			STILLRUN=`ps -ef|grep rename2.bash|grep $DIRNAME|grep -v grep|wc -l`
		done
	done
	STILLRUN=`ps -ef|grep rename2.bash|grep $DIRNAME|grep -v grep|wc -l`
	while [ $STILLRUN -gt 0 ]
	do
		echo "`date` $STILLRUN modification(s) still running for Patient $PNAME.  Sleeping..." >> $PRILOGDIR/$PRILFPROC
		STILLRUN=`ps -ef|grep rename2.bash|grep $DIRNAME|grep -v grep|wc -l`
		sleep 3
	done
else
	echo "`date` Finished modifications for Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images."  >> $PRILOGDIR/$PRILFPROC
	exit 95
fi

#If there were no errors, remove the .bak files that dcmdump creates
if [ ! -e "$PRIPROC/$DIRNAME/error.txt" ]
then
    rm $PRIPROC/$DIRNAME/*.bak
fi

echo "`date` Finished modifications for Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images."  >> $PRILOGDIR/$PRILFPROC

exit 0
