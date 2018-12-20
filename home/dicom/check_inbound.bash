#!/bin/bash
# Version 3.00.00b24
# Build 1
# 2016-02-08
# License GPLv3

RECNUM=$1
PROCESSID=$2
source readconf.bash

SCPISRUNNING=`ps -ef|tr "\t" " "|tr -s " "|cut -d " " -f2|grep $PROCESSID|wc -l`

while [ $SCPISRUNNING -ge 1 ]
do
	NUMOFDIR=`ls -1 $PRIIF|grep "^$RECNUM"|wc -l`
	PIDOFSCP=`ps -ef|grep storescp|grep -e "-ss $RECNUM"|grep -v grep|tr "\t" " "|tr -s " "|cut -d " " -f2|tr "\n" " "`

	if [ $NUMOFDIR -gt 0 ]
	then
		LISTOFDIR=`ls -1 $PRIIF|grep "^$RECNUM"|tr "\n" " "`
		for i in $LISTOFDIR
		do
			#First check to see if the study is in the database
			ISINDB=`echo "select count(*) from receive where PUID='$i';"|mysql -N -u root primal`
			TEMPDATE=`ls --full-time $PRIIF|grep $i|tr "\t" " "|tr -s " "|cut -d " " -f6-7|cut -d "." -f1`
			TIMELASTCHANGED=`date -d "$TEMPDATE" +%s`
			let MAXAGE=$PRIRECTO*7
			MAXLASTCHANGE=`date -d "$MAXAGE seconds ago" +%s`
			if [ $TIMELASTCHANGED -lt $MAXLASTCHANGE ]
			then
				#This study is too old to be here.
				ISOWNED=0
				if [ $ISINDB -gt 0 ]
				then
					#The study is old but someone claimed ownership at one time
					ISOWNED=1
					echo "Check_Inbound  PUID $i which is older than the max age of $MAXAGE seconds." >> $PRILOGDIR/$PRILFIN
				fi
				NUMOFSCP=`ps -ef|grep storescp|grep e "-ss $RECNUM"|grep -v grep|wc -l`
				if [ $NUMOFSCP -lt 2 ] && [ $ISOWNED -eq 1 ]
				then
					#No one is alive so if anyone had claimed to own this study, they have expired
					echo "Check_Inbound  PUID $i was in the DB but the owner has expired." >> $PRILOGDIR/$PRILFIN
					ISOWNED=0
				fi
				#I don't believe the following check works but it doesn't hurt
				for	j in $PIDOFSCP
				do
					OWNSFILE=`ls -l /proc/$j/fd|grep $i|wc -l`
					if [ $OWNSFILE -gt 0 ]
					then
						echo "Check_Inbound  PUID $i has someone writing to the study."  >> $PRILOGDIR/$PRILFIN
						ISOWNED=1
					fi
				done
				if [ $ISOWNED -eq 0 ]
				then
					FILENAMECI=`ls -1 $PRIIF/$i/*|head -1`
					NUMFILES=`ls -1 $PRIIF/$i/*|wc -l`
					STEMP1=`dcmdump +P 0008,0020 $FILENAMECI|cut -d "[" -f2|cut -d "]" -f1`
					STEMPYR=`echo "$STEMP1"|cut -c1-4`
					FOUNDP=`echo "$STEMP1"|tr "." "\n"|wc -l`
					if [ $FOUNDP -gt 1 ]
					then
						STEMPMO=`echo "$STEMP1"|cut -c6-7`
						STEMPDA=`echo "$STEMP1"|cut -c9-10`
					else
						STEMPMO=`echo "$STEMP1"|cut -c5-6`
						STEMPDA=`echo "$STEMP1"|cut -c7-8`
					fi
					STEMP2=`dcmdump +P 0008,0030 $FILENAMECI|cut -d "[" -f2|cut -d "]" -f1`
					STEMPHR=`echo "$STEMP2"|cut -c1-2`
					FOUNDC=`echo "$STEMP2"|grep ":"|wc -l`
					if [ $FOUNDC -gt 0 ]
					then
						STEMPMN=`echo "$STEMP2"|cut -c4-5`
						STEMPSC=`echo "$STEMP2"|cut -c7-8`
					else
						STEMPMN=`echo "$STEMP2"|cut -c3-4`
						STEMPSC=`echo "$STEMP2"|cut -c5-6`
					fi
					SDATE=`echo "$STEMPYR-$STEMPMO-$STEMPDA $STEMPHR:$STEMPMN:$STEMPSC"`
					PNAME=`dcmdump +P 0010,0010 $FILENAMECI|cut -d "[" -f2|cut -d "]" -f1`
					PAPID=`dcmdump +P 0010,0020 $FILENAMECI|cut -d "[" -f2|cut -d "]" -f1`
					PDOB=`dcmdump +P 0010,0030 $FILENAMECI|cut -d "[" -f2|cut -d "]" -f1`
					HASBRACKET=`dcmdump +P 0010,4000 $FILENAMECI|awk -F "[" '{print NF-1}'`
					if [ $HASBRACKET -lt 1 ]
					then
						PCOMM=`dcmdump +P 0010,4000 $FILENAMECI|cut -d "(" -f2|cut -d ")" -f1`
					else
						PCOMM=`dcmdump +P 0010,4000 $FILENAMECI|cut -d "[" -f2|cut -d "]" -f1`
					fi
					STEMP1=`ls -ltr $PRIIF/$i|head -2|tail -1|tr "\t" " "|tr -s " "|cut -d " " -f6-8`
					STARTRECDATE=`date -d "$STEMP1" "+%Y-%m-%d %H:%M:%S"`
					ENDRECDATE=`date "+%Y-%m-%d %H:%M:%S"`
					if [ $ISINDB -lt 1 ]
					then
						echo "insert into patient set puid='$i', pname='$PNAME', pid='$PAPID', paccn='0', pdob='$PDOB', PatientComments='$PCOMM';"|mysql -N -u root primal
						echo "insert into receive set puid='$i', rservername='`hostname`', tstartrec='$STARTRECDATE', tendrec='$ENDRECDATE', rec_images='$NUMFILES'"|mysql -N -u root primal
					else
						echo "update receive set tendrec='$ENDRECDATE' where puid='$i' and rservername='`hostname`';"|mysql -N -u root primal
					fi
					echo "Check_Inbound  PUID $i was orpahned.  Moving to processing..." >> $PRILOGDIR/$PRILFIN
					mv $PRIIF/$i $PRIPROC/
					/home/dicom/process.bash $RECNUM $PRIPROC/$i >> $PRILOGDIR/$PRILFPROC
				fi
			fi
		done
	fi
	sleep 60
done

echo "Check_Inbound:  Parent process ID $PROCESSID not found.  Exiting..." >> $PRILOGDIR/$PRILFIN
exit 0
