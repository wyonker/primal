#!/bin/bash
# Version 3.02.02
# Build 3
# 2018-12-26
# License GPLv3

RECEIVERS=`cat /etc/primal/primal.conf|grep "<scp"|cut -c5-|cut -d ">" -f1|tr "\n" " "`

logger -t primal "`date` Starting removal of old sent files..."
echo "`date` Starting removal of old sent files..."


for i in $RECEIVERS
do
	set -- $i
	source /home/dicom/bin/readconf.bash

	DIRFOUND=`find $PRISENT -name "$1_*" -type d -ctime +$PRIRET|wc -l`
	echo "Receiver: $1   Number of directories found that are to be removed:  $DIRFOUND   Days of retentnion:  $PRIRET"
	if [ "$PRIARCHTYPE0" == "S3" ] || [ "$PRIARCHTYPE0" == "disk" ]
	then
		find $PRISENT -name "$1_*" -type d -ctime +$PRIRET -exec /home/dicom/age2.bash {} \;
	else
		find $PRISENT -name "$1_*" -type d -ctime +$PRIRET -exec rm -fr {} \;
	fi
	DIRFOUND=`find $PRISENT -name "$1_*" -type d -ctime +$PRIRET|wc -l`
	echo "Receiver: $1   Number of directories found after processing (should be zero):  $DIRFOUND   Days of retentnion:  $PRIRET"

	FILLLEVEL=`df -h $PRISENT|tail -1|tr -s " "|cut -d " " -f5|cut -d "%" -f1`
	if [ $FILLLEVEL -gt 90 ]
	then
		echo "`date` Error:  Fill level is $FILLLEVEL which is above the safe threshold of 90%.  Pruning sent directories..."
		logger -t primal "`date` Error:  Fill level is $FILLLEVEL which is above the safe threshold of 90%.  Pruning sent directories..."
	fi
	DONE=0
	LC2=0
	while [ $FILLLEVEL -gt 90 ] && [ $DONE -ne 1 ] && [ $LC2 -le 100 ]
	do
		NUMDIRS=`ls $PRISENT -1str|wc -l`
		if [ $NUMDIRS -le 1 ]
		then
			echo "`date` Error:  No directories left to prune and the fill level is above 90%.  Exiting..."
			logger -t primal  "`date` Error:  No directories left to prune and the fill level is above 90%.  Exiting..."
			DONE=1
		fi
		DIRNAME=`ls -1str $PRISENT|head -3|tail -1|tr -s " "|cut -d " " -f2`
		if [ "$DIRNAME" != "" ] && [ "$DIRNAME" != " " ]
		then
			echo "update image set ilocation = NULL where puid = '$DIRNAME';"|$DBCONN
			rm -fr $PRISENT/$DIRNAME
		fi
		FILLLEVEL=`df -h $PRISENT|tail -1|tr -s " "|cut -d " " -f5|cut -d "%" -f1`
		sleep 3
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
