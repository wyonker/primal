#!/bin/bash
# Version 1.00.02
# Build 1
# 2026-07-30
# License GPLv3

PRIRECTYPE=`echo "SELECT rec_type FROM conf_rec WHERE conf_rec_id=$1;"|mysql -N -u root primal`
PRICL=`echo "SELECT rec_comp_level FROM conf_rec WHERE conf_rec_id=$1;"|mysql -N -u root primal`
PRIAET=`echo "SELECT rec_aet FROM conf_rec WHERE conf_rec_id=$1;"|mysql -N -u root primal`
PRIRECTO=`echo "SELECT rec_time_out FROM conf_rec WHERE conf_rec_id=$1;"|mysql -N -u root primal`
PRILL=`echo "SELECT rec_log_level FROM conf_rec WHERE conf_rec_id=$1;"|mysql -N -u root primal`
PRIPORT=`echo "SELECT rec_port FROM conf_rec WHERE conf_rec_id=$1;"|mysql -N -u root primal`
PRIIF=`echo "SELECT rec_dir FROM conf_rec WHERE conf_rec_id=$1;"|mysql -N -u root primal`
PRILFIN=`echo "SELECT rec_log_full_path FROM conf_rec WHERE conf_rec_id=$1;"|mysql -N -u root primal`

if [ "$PRIRECTYPE" != "1" ]
then
	exit
else
	echo "`date` Starting receiver $1 on port $PRIPORT with AET of $PRIAET and writing files to $PRIIF..." >> $PRILOGDIR/$PRILFIN
fi

export DCMDICTPATH="/home/dicom/share/dcmtk/dicom.dic"

DONE=0
while [ $DONE -ne 1 ]
do
	ISSCPRUNNING=`ps -ef|grep storescp|grep -e "-ss $1"|grep -v grep|wc -l`
	if [ $ISSCPRUNNING -lt 1 ]
	then
       	/home/dicom/bin/storescp --fork +cl $PRICL -aet $PRIAET -tos $PRIRECTO -ll $PRILL -od $PRIIF -ss $1 -xf /home/dicom/bin/storescp.cfg Default -fe ".dcm" -xcr "/usr/local/bin/mq -n send /prim_receive \"#p 1 #a #c\"" $PRIPORT >> $PRILFIN 2>&1 &
	fi
	sleep 3
done
