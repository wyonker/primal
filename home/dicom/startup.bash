#!/bin/bash
# Version 3.00.00b1
# Build 2
# 2016-01-14
# License GPLv3

source /root/.bashrc

STARTVAR=$2

# Make sure only root can run our script
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

if [ "$1" != "start" ] && [ "$1" != "stop" ] && [ "$1" != "restart" ]
then
	echo "Usage: $0 {start|stop|restart} {recever number or ALL}"
	exit 1
fi

SCPFOUND=0
if [ "$STARTVAR" == "all" ] || [ "$STARTVAR" == "ALL" ]
then
	SCPFOUND=1
else
	LIST=`cat /etc/primal/primal.conf|grep "<scp[a-zA-Z0-9]>"|cut -d "<" -f2|cut -d ">" -f1|cut -d "p" -f2|tr "\n" " "`
	for i in $LIST
	do
		if [ "$STARTVAR" == "$i" ]
		then
			SCPFOUND=1
		fi
	done
fi
if [ $SCPFOUND -eq 0 ]
then
	echo "Usage: $0 {start|stop|restart} {recever number or ALL}"
	exit 1
fi

if [ -e "/etc/primal/primalhl7.conf" ]
then
	NEEDHL7=`cat /etc/primal/primalhl7.conf|grep "PRIHL7"|cut -d "=" -f2|tr '[:upper:]' '[:lower:]'`
else
	NEEDHL7="no"
fi

RECEIVERS=`cat /etc/primal/primal.conf|grep "<scp"|cut -c5-|cut -d ">" -f1|tr "\n" " "`

if [ "$1" == "stop" ] || [ "$1" == "restart" ]
then
	logger -t primal "`date` Stopping PRIMAL receivers..."
	echo "`date` Stopping PRIMAL receivers..."
	if [ "$STARTVAR" == "all" ] || [ "$STARTVAR" == "ALL" ]
	then
		ISRUNNING1=`ps -ef|grep "scp.bash"|grep -v grep|wc -l`
		ISRUNNING2=`ps -ef|grep "storescp"|grep -v grep|wc -l`
	else
		ISRUNNING1=`ps -ef|grep "scp.bash $STARTVAR"|grep -v grep|wc -l`
		ISRUNNING2=`ps -ef|grep "storescp"|grep "--ss $STARTVAR"|grep -v grep|wc -l`
	fi
	let ISRUNNING=$ISRUNNING1+$ISRUNNING2
		
	if [ $ISRUNNING -gt 0 ]
	then
		if [ $ISRUNNING1 -gt 0 ]
		then
			if [ "$STARTVAR" == "all" ] || [ "$STARTVAR" == "ALL" ]
			then
				SSSTRING="scp.bash"
			else
				SSSTRING="scp.bash $STARTVAR"
			fi
			ISDONE=0
			LC1=0
			while [ $ISDONE -ne 1 ]
			do
				KILLLIST=`ps -ef|grep -e "$SSSTRING"|grep -v grep|tr -s " "|cut -d " " -f2|tr "\n" " "|sed 's/ $/\n/g'`
				if [ $LC1 -eq 0 ]
				then
					kill $KILLLIST
				else
					kill -9 $KILLLIST
				fi
				sleep 1
				ISDEAD=`ps -ef|grep storescp|grep -v grep|wc -l`
				if [ $ISDEAD -lt 1 ]
				then
					ISDONE=1
				fi
				if [ $LC1 -gt 2 ]
				then
					echo "`date` Error:  Could not stop all scp.bash processes.  Exiting..."
					exit 1
				fi
				let LC1=$LC1+1
			done
		fi

		#The storescp could have just went away.  Better get a new count
		if [ "$STARTVAR" == "all" ] || [ "$STARTVAR" == "ALL" ]
		then
			ISRUNNING2=`ps -ef|grep "storescp"|grep -v grep|wc -l`
		else
			ISRUNNING2=`ps -ef|grep "storescp"|grep "--ss $STARTVAR"|grep -v grep|wc -l`
		fi
		if [ $ISRUNNING2 -gt 0 ]
		then
			if [ "$STARTVAR" == "all" ] || [ "$STARTVAR" == "ALL" ]
			then
				SSSTRING=" -ss "
			else
				SSSTRING=" -ss $STARTVAR"
			fi
			ISDONE=0
			LC1=0
			while [ $ISDONE -ne 1 ]
			do
				KILLLIST=`ps -ef|grep storescp|grep -e "$SSSTRING"|grep -v grep|tr -s " "|cut -d " " -f2|tr "\n" " "|sed 's/ $/\n/g'`
				if [ $LC1 -eq 0 ]
				then
					kill $KILLLIST
				else
					kill -9 $KILLLIST
				fi
				sleep 1
				ISDEAD=`ps -ef|grep storescp|grep -v grep|wc -l`
				if [ $ISDEAD -lt 1 ]
				then
					ISDONE=1
				fi
				if [ $LC1 -gt 2 ]
				then
					echo "`date` Error:  Could not stop all storescp processes.  Exiting..."
					exit 1
				fi
				let LC1=$LC1+1
			done
		fi
	fi
	logger -t primal "`date` Finished stopping PRIMAL receivers..."
	echo "`date` Finished stopping PRIMAL receivers..."
	if [ "$NEEDHL7" == "yes" ]
	then
		logger -t primal "`date` Stopping HL7 receivers..."
		echo "`date` Stopping HL7 receivers..."
		ISHL7RUNNING=`ps -ef|grep mirthconnect|grep -v grep|wc -l`
		if [ $ISHL7RUNNING -gt 0 ]
		then
			KILLLIST=`ps -ef|grep mirthconnect|grep -v grep|tr -s " "|cut -d " " -f2`
			kill $KILLLIST
			sleep 5
		fi
		logger -t primal "`date` Finished stopping HL7 receivers..."
		echo "`date` Finished stopping HL7 receivers..."
	fi
fi

if [ "$1" == "start" ] || [ "$1" == "restart" ]
then
	logger -t primal "`date` Starting up PRIMAL receivers..."
	echo "`date` Starting up PRIMAL receivers..."
	if [ "$STARTVAR" == "all" ] || [ "$STARTVAR" == "ALL" ]
    then
		RUNNING=`ps -ef|grep scp.bash|grep -v grep|wc -l`
	else
		RUNNING=`ps -ef|grep "scp.bash $STARTVAR"|grep -v grep|wc -l`
	fi
	if [ $RUNNING -gt 0 ]
	then
		logger -t primal "`date` Error:  Cannot start PRIMAL receivers.  Some are already running.  Try stopping first..."
		echo "`date` Error:  Cannot start PRIMAL receivers.  Some are already running.  Try stopping first..."
		exit 1
	fi

	for i in $RECEIVERS
	do
		set -- $i
		source /home/dicom/bin/readconf.bash
		if [ "$STARTVAR" == "all" ] || [ "$STARTVAR" == "ALL" ] || [ "$STARTVAR" == "$i" ]
		then
			logger -t primal "`date` Starting PRIMAL $i receiver..."
			/usr/bin/screen -aA -h 20000 -d -m -S "PRIMAL SCP receiver $i" /home/dicom/scp.bash $i
		fi
	done
	logger -t primal "`date` Finished PRIMAL startup..."
	echo "`date` Finished PRIMAL startup..."

	if [ "$NEEDHL7" == "yes" ]
	then
	    logger -t primal "`date` Starting up PRIMAL HL7 receivers..."
	    echo "`date` Starting up PRIMAL HL7 receivers..."
		RUNNINGHL7=`ps -ef|grep "mirthconnect"|grep -v grep|wc -l`
		if [ $RUNNINGHL7 -gt 0 ]
		then
			logger -t primal "`date` Error:  Cannot start PRIMAL HL7 receivers.  Some are already running.  Try stopping first..."
			echo "`date` Error:  Cannot start PRIMAL HL7 receivers.  Some are already running.  Try stopping first..."
			exit 1
		fi
		/usr/bin/screen -aA -h 20000 -d -m -S "PRIMAL HL7 receiver" /opt/mirthconnect/mcserver
		logger -t primal "`date` Finished PRIMAL HL7 startup..."
		echo "`date` Finished PRIMAL HL7 startup..."
	fi
fi
