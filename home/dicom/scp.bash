#!/bin/bash
# Version 3.00.00b1
# Build 1
# 2015-09-11
# License GPLv3

re='^[0-9]+$'
if ! [[ $1 =~ $re ]]
then
    echo "USAGE:  $0 <receiver number (between 1 and 10)>"
    logger -t primal  "Error:  $0 must be called with a receiver number (between 1 and 10).  Exiting..."
    exit 1
fi

source readconf.bash

ISRUNNING=`ps -ef|grep "check_inbound.bash $1"|grep -v grep|wc -l`
if [ $ISRUNNING -gt 0 ]
then
	PID=`ps -ef|grep "check_inbound.bash $1"|grep -v grep|tr -s " "|cut -d " " -f2`
	echo "`date` Error:  check_inbound is running for $1 with a PID of $PID.  Exiting..." >> $PRILOGDIR/$PRILFIN 2>&1
	echo "`date` Error:  check_inbound is running for $1 with a PID of $PID.  Exiting..."
	exit 1
fi

if [ $PRIPREFIX -eq 0 ]
then
	echo "`date` Starting receiver $1 on port $PRIPORT with AET of $PRIAET and writing files to $PRIIF (no DICOM modifications)..." >> $PRILOGDIR/$PRILFIN
else
	echo "`date` Starting receiver $1 on port $PRIPORT with AET of $PRIAET and writing files to $PRIIF (DICOM tags will be modified)..." >> $PRILOGDIR/$PRILFIN
fi
	

export DCMDICTPATH="/home/dicom/share/dcmtk/dicom.dic"

DONE=0
while [ $DONE -ne 1 ]
do
	ISCBRUNNING=`ps -ef|grep "check_inbound.bash $1"|grep -v grep|wc -l`
	if [ $ISCBRUNNING -lt 1 ]
	then
		/home/dicom/check_inbound.bash $1 $$ & >> $PRILOGDIR/$PRILFIN 2>&1
	fi
	ISSCPRUNNING=`ps -ef|grep storescp|grep -e "-ss $1"|grep -v grep|wc -l`
	if [ $ISSCPRUNNING -lt 1 ]
	then
		/home/dicom/bin/storescp --fork -up -ac +cl $PRICL -aet $PRIAET -tos $PRIRECTO -ll $PRILL -od $PRIIF -ss $1 -xf /home/dicom/bin/storescp.cfg Default $PRIPORT >> $PRILOGDIR/$PRILFIN 2>&1 &
	fi
	sleep 3
done
