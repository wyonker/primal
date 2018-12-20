#!/bin/bash
# Version 3.00.00b14
# Build 2
# 2015-11-23
# License GPLv3

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

FILENAME=`ls -1 $PRIPROC/$DIRNAME/*|head -1`
SERIESINFO=`dcmdump $FILENAME|egrep '(0010,0010)|(0010,0020)|(0020,000d)'`
PNAME=`echo "$SERIESINFO"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PAPID=`echo "$SERIESINFO"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SIUID=`echo "$SERIESINFO"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
NUMFILES=`ls -1 $PRIPROC/$DIRNAME/*|wc -l`
if [ "$PRIQRAGE" != "ALL" ]
then
	STARTDATE=`date -d "$PRIQRAGE days ago" +%Y%m%d`
fi

STARTQRTIME=`date +%s`
echo "`date` Started Query/Retrieve for Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images." >> $PRILOGDIR/$PRILFQR

LC=0
BOLEXIST=1
while [ $LC -le 10 ] && [ $BOLEXIST -gt 0 ]
do
    BOLEXIST=`echo "$STRTEMP"|grep -c PRIQRHIP$LC`
    if [ $BOLEXIST -gt 0 ]
    then
        PRIQRHIP[$LC]=`echo "$STRTEMP"|grep PRIQRHIP$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        PRIQRPORT[$LC]=`echo "$STRTEMP"|grep PRIQRPORT$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        PRIQRAEC[$LC]=`echo "$STRTEMP"|grep PRIQRAEC$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        PRIQRDESTHIP[$LC]=`echo "$STRTEMP"|grep PRIQRDESTHIP$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        PRIQRDESTPORT[$LC]=`echo "$STRTEMP"|grep PRIQRDESTPORT$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        PRIQRDESTAEC[$LC]=`echo "$STRTEMP"|grep PRIQRDESTAEC$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        let LC=$LC+1
    fi
done
INTNUMQRDEST=$LC

INTLC1=1
NUMTAGS=`echo "$STRTEMP"|grep PRIQRTAG|wc -l`

ISSERIES=0
while [ $INTLC1 -le $NUMTAGS ]
do
	TAG[$INTLC1]=`echo "$STRTEMP"|grep "PRIQRTAG"|cut -d "=" -f2|cut -d ":" -f1|head -$INTLC1|tail -1`
	VAL[$INTLC1]=`echo "$STRTEMP"|grep "PRIQRTAG"|cut -d ":" -f2|cut -d ":" -f1|head -$INTLC1|tail -1`
	GROUP[$INTLC1]=`echo "$STRTEMP"|grep "PRIQRTAG"|cut -d ":" -f3|cut -d "#" -f1|head -$INTLC1|tail -1`
	GERROR[$INTLC1]=0
	NEEDCUSTOM=`echo "${VAL[$INTLC1]}"|grep "@o@"|wc -l`
    if [ $NEEDCUSTOM -gt 0 ]
    then
		FILENAME=`echo "${TAG[$INTLC1]}"|cut -d "(" -f2|cut -d ")" -f1|tr "," "."`
        FILENAME=`echo "qr_$1_$FILENAME.bash"`
        NEWPRESTEMP=`/home/dicom/scripts/$FILENAME $1 $2 $TAGPASS`
        if [ $? -ne 0 ]
        then
			SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
            echo "`date` WARN  Custom processing failed for group ${GROUP[$INTLC1]} for patient $PNAME MRN $PAPID PRIMAL $DIRNAME."  >> $PRILOGDIR/$PRILFQR
			GERROR[$INTLC1]=1
        fi
		VAL[$INTLC1]=`echo "$NEWPRESTEMP"`
	fi
	TAGSTR=`echo "$TAGSTR"\|${TAG[$INTLC1]}`
	let INTLC1=$INTLC1+1
done

FILENAME=`ls -1 $PRIPROC/$DIRNAME/*|head -1`
TAGSTR=`echo "(0008,0060)"$TAGSTR`
SERIESINFO=`dcmdump $FILENAME|egrep "$TAGSTR"`
MODALITY=`echo "$SERIESINFO"|grep "(0008,0060)"|cut -d "[" -f2|cut -d "]" -f1`

#Need to find out the number of groups
INTLC1=1
INTNUMGROUPS=0
while [ $INTLC1 -le $NUMTAGS ]
do
	if [ $INTNUMGROUPS -lt ${GROUP[$INTLC1]} ]
	then
		INTNUMGROUPS=${GROUP[$INTLC1]}
	fi
	let INTLC1=$INTLC1+1
done

INTLC2=1
while [ $INTLC2 -le $INTNUMGROUPS ]
do
	QUERYLEVEL[$INTLC2]="STUDY"
	let INTLC2=$INTLC2+1
done

INTLC2=1
while [ $INTLC2 -le $INTNUMGROUPS ]
do
	INTLC1=1
	while [ $INTLC1 -le $NUMTAGS ]
	do
		if [ "${VAL[$INTLC1]}" != "" ] && [ "${VAL[$INTLC1]}" != " " ]
		then
			if [ ${GROUP[$INTLC1]} -eq $INTLC2 ]
			then
				if [ "${TAG[$INTLC1]}" == "(0008,0060)" ]
				then
					QUERYLEVEL[$INTLC2]="SERIES"
				fi
				PRE[$INTLC1]=`echo "$SERIESINFO"|grep "${TAG[$INTLC1]}"|head -1|cut -d "[" -f2|cut -d "]" -f1`
				NEWTAG[$INTLC1]=`echo "${VAL[$INTLC1]}"|sed -e "s~@t@~${PRE[$INTLC1]}~g"`
				NEWTAG[$INTLC1]=`echo "${NEWTAG[$INTLC1]}"|sed -e "s~@m@~$MODALITY~g"`
				TEMPFINDCMD[$INTLC2]=`echo "${TEMPFINDCMD[$INTLC2]} -k \"${TAG[$INTLC1]}=${NEWTAG[$INTLC1]}\""`
			fi
		else
			echo "`date` Warning:  Will not query on tag ${TAG[$INTLC1]} for patient $PNAME MRN $PAPID PRIMAL $DIRNAME" >> $PRILOGDIR/$PRILFQR
		fi
		let INTLC1=$INTLC1+1
	done
	FINDCMD[$INTLC2]=`echo "/home/dicom/bin/findscu -ll debug -aet $PRIAET -S -k 0008,0052=${QUERYLEVEL[$INTLC2]} ${TEMPFINDCMD[$INTLC2]}"`
	let INTLC2=$INTLC2+1
done

INTLC1=0
INTLC2=0
ISDONE=0
while [ $INTLC1 -lt $INTNUMQRDEST ] && [ $ISDONE -eq 0 ]
do
	INTLC3=1
	while [ $INTLC3 -le $INTNUMGROUPS ]
	do
		if [ ${GERROR[$INTLC3]} -ne 1 ]
		then
			echo "${FINDCMD[$INTLC3]} -aec ${PRIQRAEC[$INTLC1]} -k 0020,000D -k 0008,0020 ${PRIQRHIP[$INTLC1]} ${PRIQRPORT[$INTLC1]} 2>&1" > /tmp/$DIRNAME.cfind
			if [ "$PRILL" = "DEBUG" ] || [ "$PRILL" = "debug" ]
			then
				echo "`date` PRIMAL: $DIRNAME : ${FINDCMD[$INTLC3]} -aec ${PRIQRAEC[$INTLC1]} -k 0020,000D -k 0008,0020 ${PRIQRHIP[$INTLC1]} ${PRIQRPORT[$INTLC1]} 2>&1" >> $PRILOGDIR/$PRILFQR
			fi
			chmod 755 /tmp/$DIRNAME.cfind
			FINDOUTPUT[$INTLC3]=`/tmp/$DIRNAME.cfind|egrep '0008,0020|0020,000d'|cut -d "[" -f2|cut -d "]" -f1`
			rm -f /tmp/$DIRNAME.cfind
			UNSORTEDLIST[$INTLC3]=$(echo "${FINDOUTPUT[$INTLC3]}" | awk '{
				if(NR > 2) {
					if(EVENODD != 1) {
						LINEBUF=$0;
						EVENODD=1;
					} else {
						LINEBUF2=LINEBUF" "$0;
						print (LINEBUF2);
						EVENODD=0;
					}
				}
			}')
		fi
		let INTLC3=$INTLC3+1
	done

	INTLC3=0
    while [ $INTLC3 -le $INTNUMGROUPS ]
    do
		if [ $INTLC3 -eq 0 ]
		then
			UNSORTEDMASTER=`echo "$UNSORTEDMASTER${UNSORTEDLIST[$INTLC3]}"`
		else
			UNSORTEDMASTER=`echo -e "$UNSORTEDMASTER\n${UNSORTEDLIST[$INTLC3]}"`
		fi	
		let INTLC3=$INTLC3+1
	done
	NUMRESULTS=`echo "$UNSORTEDMASTER"|wc -l`
	SORTEDLIST=`echo "$UNSORTEDMASTER"|sort -u -k2|sort -nr -k1|cut -d " " -f2|tr "\n" " "`
	if [ "$PRIQRAGE" != "ALL" ] && [ "$PRIQRAGE" != "all" ]
	then
		STARTRANGE=`date -d "$PRIQRAGE days ago" +%Y%m%d`
		RESTRICTEDLIST=$(echo "${SORTEDLIST[$INTLC3]}" | awk -v STARTRANGE="$STARTRANGE" '{
				if ($1 > STARTRANGE) {
					print($0);
				}
			}')
	else
		RESTRICTEDLIST=`echo "$SORTEDLIST"`
	fi
	echo "`date` PRIMAL: $DIRNAME : Found $NUMRESULTS results for patient $PNAME in archive ${PRIQRAEC[$INTLC1]}" >> $PRILOGDIR/$PRILFQR
	for i in $RESTRICTEDLIST
	do
		CURSTUDY=`echo "select count(*) from study where SIUID='$i' and puid='$DIRNAME';"|mysql primal|tail -1`
		if [ $CURSTUDY -eq 0 ]
		then
			if [ $INTLC2 -lt $PRIQRMAX ]
			then
				echo "`date` PRIMAL: $DIRNAME : Retrieving StudyInstanceUID $i from archive ${PRIQRAEC[$INTLC1]}" >> $PRILOGDIR/$PRILFQR
				if [ "$PRILL" = "DEBUG" ] || [ "$PRILL" = "debug" ]
				then
					echo "/home/dcm4che/bin/movescu -b ${PRIQRDESTAEC[$INTLC1]}@${PRIQRDESTHIP[$INTLC1]}:${PRIQRDESTPORT[$INTLC1]} -c ${PRIQRAEC[$INTLC1]}@${PRIQRHIP[$INTLC1]}:${PRIQRPORT[$INTLC1]} -m StudyInstanceUID="$i" --dest ${PRIQRDESTAEC[$INTLC1]}" >> $PRILOGDIR/$PRILFQR
				fi
				/home/dcm4che/bin/movescu -b ${PRIQRDESTAEC[$INTLC1]}@${PRIQRDESTHIP[$INTLC1]}:${PRIQRDESTPORT[$INTLC1]} -c ${PRIQRAEC[$INTLC1]}@${PRIQRHIP[$INTLC1]}:${PRIQRPORT[$INTLC1]} -m StudyInstanceUID="$i" --dest ${PRIQRDESTAEC[$INTLC1]} &
				sleep 1
				DBDATE=`date +"%Y-%m-%d %H:%M:%S"`
				echo "insert into QR set SIUID='$i', PatientPUID='$DIRNAME', QueryDate='$DBDATE', QueryHost='${PRIQRHIP[$INTLC1]}', QueryHostPort='${PRIQRPORT[$INTLC1]}';"|mysql -u root primal
			fi
			let INTLC2=$INTLC2+1
			if [ $INTLC2 -gt $PRIQRMAX ] && [ $PRIQRMAX -ne 0 ]
			then
				ISDONE=1
			fi
		else
			echo "Ignoring current study..."
		fi
	done
	let INTLC1=$INTLC1+1
done

INTLC1=0
ISDONE=0
if [ $PRIQRWAIT -eq 1 ]
then
	while [ $ISDONE -eq 0 ]
	do
		let NUMQR=$INTLC2-1
		
		NUMRET=`echo "select count(*) from QR where PatientPUID='$DIRNAME' and RetrieveDate IS NULL;"|mysql -N -u root primal`
		RESTRICTEDLIST2=`echo "select SIUID from QR where PatientPUID='$DIRNAME' and RetrieveDate IS NULL;"|mysql -N -u root primal|tr "\n" " "`
		if [ $NUMRET -gt 0 ]
		then
			let INTDIFF=40-$INTLC1
			INTTIMELEFT=`echo "$INTDIFF*15"|bc -l`
			echo "`date` PRIMAL: $DIRNAME for patient $PNAME MRN $PAPID  Still waiting for studies $RESTRICTEDLIST2.  Waiting for 15 seconds (Time remaining: $INTTIMELEFT sec)..." >> $PRILOGDIR/$PRILFQR
			sleep 15
			NUMRET=`echo "select count(*) from QR where PatientPUID='$DIRNAME' and RetrieveDate IS NULL;"|mysql -N -u root primal`
			RESTRICTEDLIST2=`echo "select SIUID from QR where PatientPUID='$DIRNAME' and RetrieveDate IS NULL;"|mysql -N -u root primal|tr "\n" " "`
		else
			ISDONE=1
		fi
		if [ $INTLC1 -gt 40 ]
		then
			echo "`date` WARNING  PRIMAL: $DIRNAME for patient $PNAME MRN $PAPID  Waited for 600 seconds but all priors were not retrieved.  Forwarding stduy anyway..." >> $PRILOGDIR/$PRILFQR
			ISDONE=1
		fi
		let INTLC1=$INTLC1+1
	done
fi

echo "`date` Finished Query/Retrieve for Patient: $PNAME MRN: $PAPID PRIMAL: $DIRNAME with $NUMFILES images." >> $PRILOGDIR/$PRILFQR
