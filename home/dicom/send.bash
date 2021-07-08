#!/bin/bash
# Version 3.20.05b1
# Build 4
# 2019-12-10
# License GPLv3

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

FILENAME=`ls -1 $PRIOUT/$DIRNAME/*|head -1`
SERIESINFO=`dcmdump $FILENAME|egrep '(0010,0010)|(0010,0020)|(0020,000d)'`
PNAME=`echo "$SERIESINFO"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PAPID=`echo "$SERIESINFO"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SIUID=`echo "$SERIESINFO"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
NUMFILES=`ls -1 $PRIOUT/$DIRNAME/*|wc -l`

if [ $PRIPASSTU -gt 0 ]
then
	AET=`echo "select senderAET from receive where PUID = '$DIRNAME' limit 1;"|$DBCONN|tail -1`
	if [ "$AET" != "" ]
	then
		PRIAET=`echo "$AET"`
	fi
fi

NUMDEST=0
while [ $NUMDEST -lt $INTNUMREC ]
do
	if [ ${PRIDESTAD[$INTNUMREC]} -eq -1 ]
	then
		/home/dicom/send2.bash $1 $PRIOUT/$DIRNAME $NUMDEST & >> $PRILOGDIR/$PRILFOUT
	else
		INTRETURN=`/home/dicom/send_cond $NUMDEST $DIRNAME`
		if [ $INTRETURN -ne 0 ] && [ $INTRETURN -ne 1 ]
		then
			if [ ${PRIDESTAD[$INTNUMREC]} -eq 0 ]
			then
				echo "`date` ERROR:  Rule check failed to returned $INTRETURN.  Must return 0 or 1.  Will follow the default and route." >> $PRILOGDIR/$PRILFOUT
				/home/dicom/send2.bash $1 $PRIOUT/$DIRNAME $NUMDEST & >> $PRILOGDIR/$PRILFOUT
			elif [ ${PRIDESTAD[$INTNUMREC]} -eq 1 ]
			then
				echo "`date` ERROR:  Rule check failed to returned $INTRETURN.  Must return 0 or 1.  Will follow the default and not route." >> $PRILOGDIR/$PRILFOUT
			else
				echo "`date` ERROR:  Rule check failed to returned $INTRETURN.  Must return 0 or 1.  Unable to determine what the default is (UNDEFINED BEHAVIOR).  Exiting..." >> $PRILOGDIR/$PRILFOUT
				exit 1
			fi
		elif [ ${PRIDESTAD[$INTNUMREC]} -eq 0 ] && [ $INTRETURN -eq 0 ]
		then
			/home/dicom/send2.bash $1 $PRIOUT/$DIRNAME $NUMDEST & >> $PRILOGDIR/$PRILFOUT
		elif [ ${PRIDESTAD[$INTNUMREC]} -eq 0 ] && [ $INTRETURN -eq 1 ]
		then
				echo "`date` INFO:  Not routing because of a rule match for destination $INTRETURN." >> $PRILOGDIR/$PRILFOUT
		elif [ ${PRIDESTAD[$INTNUMREC]} -eq 1 ] && [ $INTRETURN -eq 1 ]
		then
			/home/dicom/send2.bash $1 $PRIOUT/$DIRNAME $NUMDEST & >> $PRILOGDIR/$PRILFOUT
		elif [ ${PRIDESTAD[$INTNUMREC]} -eq 1 ] && [ $INTRETURN -eq 0 ]
		then
				echo "`date` INFO:  Not routing because no rules match for destination $INTRETURN." >> $PRILOGDIR/$PRILFOUT
		fi
	fi
	let NUMDEST=$NUMDEST+1
done

NUMCOMPLETE=`echo "select count(*) from send where puid = '$DIRNAME' and complete = '1'"|$DBCONN|tail -1`
while [ $NUMCOMPLETE -lt $INTNUMREC ] && [ $LC -lt 3600 ]
do
	sleep 1
	NUMCOMPLETE=`echo "select count(*) from send where puid = '$DIRNAME' and complete = '1'"|$DBCONN|tail -1`
	let LC=$LC+1
done

if [ $LC -lt 3600 ]
then
	ISERROR=`echo "select serror from send where puid = '$DIRNAME' group by serror;"|$DBCONNN`
	ISERROR=`echo "$ISERROR"|sort|tail -1|tr "\t" " "|cut -d " " -f1`
	if [ $ISERROR -eq 1 ]
	then
		CURRDIR=$PRIERROR
	else
		#If this is a QR receiver, we need to update the QR table
		INTQR=`echo "select count(*) from QR where SIUID='$SIUID' and puid='';"|$DBCONNN`
		if [ $INTQR -gt 0 ]
		then
			TEMPPUID=`echo "select PatientPUID from QR where SIUID='$SIUID' and puid='' limit 1;"|$DBCONNN`
			TEMPDATE=`date +"%Y-%m-%d %H:%M:%S"`
			echo "update QR set puid='$DIRNAME', RetrieveDate='$TEMPDATE' where SIUID='$SIUID' and PatientPUID='$TEMPPUID';"|$DBCONNN
		fi
		CURRDIR=$PRISENT
	fi
	echo "update image set ilocation = '$CURRDIR/$DIRNAME' where PUID = '$DIRNAME';"|$DBCONN

	if [ $ISERROR -eq 1 ]
    then
		mv $PRIOUT/$DIRNAME $PRIERROR/
	else
		mv $PRIOUT/$DIRNAME $PRISENT/
	fi
fi
rm -f /tmp/$DIRNAME
