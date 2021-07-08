#!/bin/bash
# Version 3.20.04b04
# Build 3
# 2019-12-05
# License GPLv3

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

if [ $PRIPASSTU -gt 0 ]
then
	AET=`echo "select senderAET from receive where PUID = '$DIRNAME' limit 1;"|$DBCONN|tail -1`
	if [ "$AET" != "" ]
	then
		PRIAET=`echo "$AET"`
	fi
fi

NUMDEST=$3
if [ $PRICDCR -eq 1 ]
then
	#Need to send the compressed study
	FILEEXT=`echo "*.j2k"`
elif [ $PRICDCR -eq 2 ]
then
	#Need to send the uncompressed study
	FILEEXT=`echo "*.ucr"`
else
	#Just send the study as we received it
	FILEEXT=`echo "*.dcm"`
fi
#Are we supposed to just dump this?
if [ "${PRIDESTHIP[$NUMDEST]}" = "0.0.0.0" ]
then
	#Let's just update the DB and return
	NUMIMGS=`ls -1 $2|wc -l`
	THISPUID=`echo "$2"|rev|cut -d "/" -f1|rev`
    echo "`date`  Skipping destination $3 for PUID $THISPUID." >> $PRILOGDIR/$PRILFOUT
	echo "insert into send (puid, sservername, tdest, tstartsend, tendsend, timages, serror, complete) values ('$THISPUID', '`hostname -s`', '$3', '`date "+%Y-%m-%d %H:%M:%S"`', '`date "+%Y-%m-%d %H:%M:%S"`','$NUMIMGS', 0, 1);"|$DBCONNN
	#echo "update send set tendsend='`date "+%Y-%m-%d %H:%M:%S"`', serror='0', timages='$NUMIMGS', complete = '1' where send.puid='$THISPUID' and tdest='$3';"|$DBCONNN
	exit 0
fi

#Before we start sending, we need to see if there is a queue
QUEUELENGTH=`echo "select count(*) from ticket where destination = '${PRIDESTAEC[$NUMDEST]}' order by ticket_num;"|$DBCONNN`
NUMSENDS=`ps -ef|grep primalscu|grep -e "-aec ${PRIDESTAEC[$NUMDEST]}"|wc -l`
if [ $QUEUELENGTH -eq 0 ] && [ $NUMSENDS -lt 20 ]
then
	primalscu -ll $PRILL -aet $PRIAET -xf /home/dicom/bin/storescu.cfg Default -aec ${PRIDESTAEC[$NUMDEST]} ${PRIDESTHIP[$NUMDEST]} ${PRIDESTPORT[$NUMDEST]} $PRIOUT/$DIRNAME/$FILEEXT >> $PRILOGDIR/$PRILFOUT 2>&1 &
else
	THISPID=$$
	DONE=0
	while [ $DONE -ne 1 ]
	do
		HASTICKET=`echo "select count(*) from ticket where destination = '${PRIDESTAEC[$NUMDEST]}' and proc_id = '$THISPID';"|$DBCONNN`
		PROCID=`echo "select proc_id from ticket where destination = '${PRIDESTAEC[$NUMDEST]}' order by ticket_num limit 1;"|$DBCONNN`
		if [ $HASTICKET -eq 0 ] && [ $QUEUELENGTH -gt 0 ]
		then
			#We don't have a ticket and there is a queue.  Now we should check and see if the process on the top of the queue is still running.
			#In case there was a crash or it died without updating the database
			ISTOPTICKETRUNNING=`ps -ef|grep primalscu|tr "\t" " "|tr -s " "|cut -d " " -f2|grep $PROCID`
			if [ $ISTOPTICKETRUNNING -eq 0 ]
			then
				#The top ticket owner isn't in the process list.  Let's update the database
				echo "delete from ticket where destination = '${PRIDESTAEC[$NUMDEST]}' and proc_id = '$PROCID';"|$DBCONNN
				echo "Send.bash  PRIMAL ID $DIRNAME  Top ticket holder for destiantion ${PRIDESTAEC[$NUMDEST]} has expired.  Removing them from the queue."  >> $PRILOGDIR/$PRILFOUT
			fi
			echo "insert into ticket set destination = '${PRIDESTAEC[$NUMDEST]}', proc_id = '$THISPID';"|$DBCONNN
			echo "Send.bash  PRIMAL ID $DIRNAME  Adding process ID $THISPID to the queue for destination ${PRIDESTAEC[$NUMDEST]}.." >> $PRILOGDIR/$PRILFOUT
			sleep 1
		elif [ $HASTICKET -eq 0 ] && [ $QUEUELENGTH -eq 0 ]
		then
			#We don't have a ticket and there is no queue.  But there are still too many sends.  Best we wait.
			if [ $NUMSENDS -ge 20 ]
			then
				echo "insert into ticket set destination = '${PRIDESTAEC[$NUMDEST]}', proc_id = '$THISPID';"|$DBCONNN
				echo "Send.bash  PRIMAL ID $DIRNAME  Creating queue for destination ${PRIDESTAEC[$NUMDEST]} with $THISPID process ID." >> $PRILOGDIR/$PRILFOUT
				sleep 1
			else
				DONE=1
			fi
		elif [ $HASTICKET -gt 0 ] && [ $THISPID -eq $PROCID ]
		then
			#We have the top ticket in the queue.  Let's remove it and use it
			echo "delete from ticket where destination = '${PRIDESTAEC[$NUMDEST]}' and proc_id = '$THISPID';"|$DBCONNN
			echo "Send.bash  PRIMAL ID $DIRNAME  My ticket is on top.  Sending now..." >> $PRILOGDIR/$PRILFOUT
			DONE=1
		else
			#We have a ticket and there are guys in the queue above us.  Guess we wait
			#In case there was a crash or it died without updating the database
			ISTOPTICKETRUNNING=`ps -ef|grep primalscu|tr "\t" " "|tr -s " "|cut -d " " -f2|grep $PROCID`
			if [ $ISTOPTICKETRUNNING -eq 0 ]
			then
				#The top ticket owner isn't in the process list.  Let's update the database
				echo "delete from ticket where destination = '${PRIDESTAEC[$NUMDEST]}' and proc_id = '$PROCID';"|$DBCONNN
				echo "Send.bash  PRIMAL ID $DIRNAME  Top ticket holder for destiantion ${PRIDESTAEC[$NUMDEST]} has expired.  Removing them from the queue."  >> $PRILOGDIR/$PRILFOUT
			fi
			sleep 1
			QUEUELENGTH=`echo "select count(*) from ticket where destination = '${PRIDESTAEC[$NUMDEST]}' order by ticket_num;"|$DBCONNN`
			NUMSENDS=`ps -ef|grep primalscu|grep -e "-aec ${PRIDESTAEC[$NUMDEST]}"|wc -l`
		fi
		if [ $DONE -eq 1 ]
		then
			primalscu -ll $PRILL -aet $PRIAET -xf /home/dicom/bin/storescu.cfg Default -aec ${PRIDESTAEC[$NUMDEST]} ${PRIDESTHIP[$NUMDEST]} ${PRIDESTPORT[$NUMDEST]} $PRIOUT/$DIRNAME/$FILEEXT >> $PRILOGDIR/$PRILFOUT 2>&1 &
		fi
	done
fi

exit 0
