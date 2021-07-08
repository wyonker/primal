#!/bin/bash
# Version 3.30.02
# Build 6
# 2021-06-14
# License GPLv3

RECEIVERS=`cat /etc/primal/primal.conf|grep "<scp"|cut -c5-|cut -d ">" -f1|tr "\n" " "`

logger -t primal "`date` Starting removal of old sent files..."
echo "`date` Starting removal of old sent files..."

for i in $RECEIVERS
do
	set -- $i
	source /home/dicom/bin/readconf.bash
	if [ $PRIRET -eq -1 ]
	then
		echo "Receiver: $1 is set to not purge.  Skipping..."
	else
		if [ $PRIRET -eq 0 ]
		then
			DIRFOUND=`find $PRISENT -name "$1_*" -type d|wc -l`
		else
			DIRFOUND=`find $PRISENT -name "$1_*" -type d -ctime +$PRIRET|wc -l`
		fi
		echo "Receiver: $1   Number of directories found that are to be removed:  $DIRFOUND   Days of retentnion:  $PRIRET"
		if [ "$PRIARCHTYPE0" == "S3" ] || [ "$PRIARCHTYPE0" == "disk" ]
		then
			if [ $PRIRET -eq 0 ]
			then
				find $PRISENT -name "$1_*" -type d -exec /home/dicom/age2.bash a {} \;
			else
				find $PRISENT -name "$1_*" -type d -ctime +$PRIRET -exec /home/dicom/age2.bash a {} \;
			fi
		else
			if [ $PRIRET -eq 0 ]
			then
				find $PRISENT -name "$1_*" -type d -exec /home/dicom/age2.bash d {} \;
			else
				find $PRISENT -name "$1_*" -type d -ctime +$PRIRET -exec /home/dicom/age2.bash d {} \;
			fi
		fi
		if [ $PRIRET -eq 0 ]
		then
			DIRFOUND=`find $PRISENT -name "$1_*" -type d|wc -l`
		else
			DIRFOUND=`find $PRISENT -name "$1_*" -type d -ctime +$PRIRET|wc -l`
		fi
		echo "Receiver: $1   Number of directories found after processing (should be zero):  $DIRFOUND   Days of retentnion:  $PRIRET"
	fi

	FILLLEVEL=`df -h $PRISENT|tail -1|tr -s " "|cut -d " " -f5|cut -d "%" -f1`
	if [ $FILLLEVEL -gt 80 ]
	then
		echo "`date` Error:  Fill level is $FILLLEVEL which is above the safe threshold of 80%.  Pruning sent directories..."
		logger -t primal "`date` Error:  Fill level is $FILLLEVEL which is above the safe threshold of 80%.  Pruning sent directories..."
	fi
	DONE=0
	LC2=0
	while [ $FILLLEVEL -gt 80 ] && [ $DONE -ne 1 ] && [ $LC2 -le 100 ]
	do
		NUMDIRS=`ls $PRISENT -1tr|wc -l`
		if [ $NUMDIRS -le 1 ]
		then
			echo "`date` Error:  No directories left to prune and the fill level is above 80%.  Trying error directory..."
			logger -t primal  "`date` Error:  No directories left to prune and the fill level is above 80%.  Trying error directory..."
			DONE=1
		fi
		DIRNAME=`ls -1tr $PRISENT|head -1`
		if [ "$DIRNAME" != "" ] && [ "$DIRNAME" != " " ]
		then
			echo "update image set ilocation = NULL where puid = '$DIRNAME';"|$DBCONN
			rm -fr $PRISENT/$DIRNAME
		fi
		FILLLEVEL=`df -h $PRISENT|tail -1|tr -s " "|cut -d " " -f5|cut -d "%" -f1`
		sleep 1
		let LC2=$LC2+1
	done	
	DONE=0
	LC2=0
	while [ $FILLLEVEL -gt 80 ] && [ $DONE -ne 1 ] && [ $LC2 -le 100 ]
	do
		NUMDIRS=`ls $PRIERROR -1tr|wc -l`
		if [ $NUMDIRS -le 1 ]
		then
			echo "`date` Error:  No directories left to prune and the fill level is above 80%.  Trying exiting..."
			logger -t primal  "`date` Error:  No directories left to prune and the fill level is above 80%.  Trying exiting..."
			DONE=1
		fi
		DIRNAME=`ls -1tr $PRIERROR|head -1`
		if [ "$DIRNAME" != "" ] && [ "$DIRNAME" != " " ]
		then
			echo "update image set ilocation = NULL where puid = '$DIRNAME';"|$DBCONN
			rm -fr $PRIERROR/$DIRNAME
		fi
		FILLLEVEL=`df -h $PRIERROR|tail -1|tr -s " "|cut -d " " -f5|cut -d "%" -f1`
		sleep 1
		let LC2=$LC2+1
	done	

	LC2=0
	if [ $FILLLEVEL -gt 95 ] && [ $LC2 -le 100 ]
	then
		echo "`date` Error:  Fill level is $FILLLEVEL which is above the safe operating threshold of 95%.  Killing receiver $1..."
		logger -t primal "`date` Error:  Fill level is $FILLLEVEL which is above the safe operating threshold of 95%.  Killing receiver $1..."
		NUMTODIE=`ps -ef|grep storescp|grep "\-ss $1"|grep -v "grep"|wc -l`
		DONE=0
		LC=0
		while [ $NUMTODIE -gt 0 ] && [ $DONE -ne 1 ]
		do
			DEATHMARK=`ps -ef|grep storescp|grep "\-ss $1"|grep -v "grep"|head -1|tr -s " "|cut -d " " -f2`
			kill $DEATHMARK
			NUMTODIE=`ps -ef|grep storescp|grep "\-ss $1"|grep -v "grep"|wc -l`
			let LC=$LC+1
			if [ $LC -gt 40 ]
			then
				DONE=1
			fi
			sleep 3
		done
		let LC2=$LC2+1
	fi
done

#Clean /var/www/html/tmp
find /var/www/html/tmp -name "*.jpg" -type f -ctime +2 -exec rm -f {} \;

logger -t primal "`date` Finished removal of old sent files..."
echo "`date` Finished removal of old sent files..."
