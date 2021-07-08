#!/bin/bash
# Version 1.30.1
# Build 9
# 2020-07-10

RECNUM=`echo "$1"`
re='^[0-9]+$'
INTTEMP=`cat /etc/primal/primal.conf|grep -n "</scp$1>"|cut -d ":" -f1`
if ! [[ $INTTEMP =~ $re ]]
then
    echo "Error:  Instance number not found in /etc/primal/primal.conf.  Exiting..."
    logger -t primal "Error:  Instance number not found in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    exit 1
fi

INTTEMP2=`cat /etc/primal/primal.conf|grep -n "<scp$1>"|cut -d ":" -f1`
let INTTEMP3=$INTTEMP-$INTTEMP2
STRTEMP=`cat /etc/primal/primal.conf|head -$INTTEMP|tail -$INTTEMP3`

PRILOGDIR=`echo "$STRTEMP"|grep PRILOGDIR|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
touch $PRILOGDIR/$PRILFIN
if [ $? -gt 0 ]
then
    echo "Error:  Could not write to log file $PRILOGDIR/$PRILFIN for receiver #$RECNUM.  Exiting..."
    logger -t primal "Error:  Log directory does not exist for receiver #$RECNUM.  Exiting..."
    exit 1
fi

PRILFIN=`echo "$STRTEMP"|grep PRILFIN|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
touch $PRILOGDIR/$PRILFIN
if [ $? -gt 0 ]
then
    echo "`date` Error:  Log directory does not exist for receiver #$1.  Exiting..."
    logger -t primal "Error:  Log directory does not exist for receiver #$1.  Exiting..."
    exit 1
fi

PRILFPROC=`echo "$STRTEMP"|grep PRILFPROC|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
touch $PRILOGDIR/$PRILFPROC
if [ $? -gt 0 ]
then
    echo "Error:  Could not write to log file $PRILOGDIR/$PRILFPROC for receiver #$RECNUM.  Exiting..."
    logger -t primal "Error:  Could not write to log file $PRILOGDIR/$PRILFPROC for receiver #$RECNUM.  Exiting..."
    exit 1
fi

PRILFQR=`echo "$STRTEMP"|grep PRILFQR|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
touch $PRILOGDIR/$PRILFQR
if [ $? -gt 0 ]
then
    echo "Error:  Could not write to log file $PRILOGDIR/$PRILFQR for receiver #$RECNUM.  Exiting..."
    logger -t primal "Error:  Could not write to log file $PRILOGDIR/$PRILFQR for receiver #$RECNUM.  Exiting..."
    exit 1
fi

PRILFOUT=`echo "$STRTEMP"|grep PRILFOUT|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
touch $PRILOGDIR/$PRILFOUT
if [ $? -gt 0 ]
then
    echo "Error:  Could not write to log file $PRILOGDIR/$PRILFOUT for receiver #$RECNUM.  Exiting..."
    logger -t primal "Error:  Could not write to log file $PRILOGDIR/$PRILFOUT for receiver #$RECNUM.  Exiting..." >> $PRILOGDIR/$PRILFPROC
    exit 1
fi

PRIIF=`echo "$STRTEMP"|grep PRIIF|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ ! -d "$PRIIF" ]
then
    echo "Error:  Inbound directory does not exist for receiver #$1.  Exiting..."
    logger -t primal "Error:  Inbound directory does not exist for receiver #$1.  Exiting..."
    exit 1
fi

PRIRECTO=`echo "$STRTEMP"|grep PRIRECTO|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ $PRIRECTO -lt 3 ] || [ $PRIRECTO -gt 300 ]
then
    echo "Error:  The receive time out must be between 3 and 300 seconds.  Error for receiver #$1.  Exiting..."
    logger -t primal "Error:  The receive time out must be between 3 and 300 seconds.  Error for receiver #$1.  Exiting..."
    exit 1
fi

PRIRET=`echo "$STRTEMP"|grep PRIRET|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ $PRIRET -lt -1 ] || [ $PRIRET -gt 300 ]
then
    echo "Error:  The retention time must be defined.  If you wish to keep sent files forever, please set it to -1 in the configuration file.  Error for receiver #$1.  Exiting..."
    logger -t primal "Error:  The retention time must be defined.  If you wish to keep sent files forever, please set it to 0 in the configuration file.  Error for receiver #$1.  Exiting..."
    exit 1
fi

PRIRECTYPE=`echo "$STRTEMP"|grep PRIRECTYPE|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ "$PRIRECTYPE" != "DICOM"  ] && [ "$PRIRECTYPE" != "TAR" ]
then
    PRIRECTYPE="DICOM"
fi

PRIPORT=`echo "$STRTEMP"|grep PRIPORT|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if ! [[ $PRIPORT =~ $re ]]
then
    echo "Error:  Port to listen on is not in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    logger -t primal "Error:  Port to listen on is not in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    exit 1
fi
if [ $PRIPORT -lt 100 ] || [ $PRIPORT -gt 65535 ]
then
    echo "Error:  Port to listen on is $PRIPORT which must be between 100 and 65535.  Exiting..."
    logger -t primal "Error:  Port to listen on is $PRIPORT which must be between 100 and 65535.  Exiting...."
    exit 1
fi

PRILL=`echo "$STRTEMP"|grep PRILL|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ "$PRILL" != "fatal" ] && [ "$PRILL" != "error" ] && [ "$PRILL" != "warn" ] && [ "$PRILL" != "info" ] && [ "$PRILL" != "debug" ] && [ "$PRILL" != "trace" ]
then
    echo "Error:  Log Level must be of value (fatal, error, warn, info, debug or trace) for receiver #$1.  Exiting..."
    logger -t primal "Error:  Log Level must be of value (fatal, error, warn, info, debug or trace) for receiver #$1.  Exiting..."
    exit 1
fi

PRIAET=`echo "$STRTEMP"|grep PRIAET|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if ! [[ "$PRIAET" =~ ^[a-zA-Z0-9_-]+$ ]]
then
    echo "Error:  AET must exist and can only contain alpha and numeric characters.  Receiver #$1 failed check.  Exiting..."
    logger -t primal "Error:  AET must exist and can only contain alpha and numeric characters.  Receiver #$1 failed check.  Exiting..."
    exit 1
fi

PRICL=`echo "$STRTEMP"|grep PRICL|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if ! [[ $PRICL =~ $re ]]
then
    echo "Error:  Compression level is not in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    logger -t primal "Error:  Compression level is not in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    exit 1
fi
if [ $PRICL -lt 0 ] || [ $PRICL -gt 9 ]
then
    echo "Error:  Compression level must be between 0 and 9.  Exiting..."
    logger -t primal "Error:  Compression level must be between 0 and 9.  Exiting...."
    exit 1
fi

PRIDUPE=`echo "$STRTEMP"|grep PRIDUPE|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if ! [[ $PRIDUPE =~ $re ]]
then
    echo "Error:  Decompression is not specified in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    logger -t primal "Error:  Decompression is not specified in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    exit 1
fi
if [ $PRIDUPE -lt 0 ] || [ $PRIDUPE -gt 1 ]
then
    echo "Error:  PRIDUPE (Decompress) must be 0 or 1 (to decompress).  Exiting..."
    logger -t primal "Error:  PRIDUPE (Decompress) must be 0 or 1 (to decompress).  Exiting...."
    exit 1
fi

PRIQRREC=`echo "$STRTEMP"|grep PRIQRREC|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
PRIQRAGE=`echo "$STRTEMP"|grep PRIQRAGE|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
PRIQRWAIT=`echo "$STRTEMP"|grep PRIQRWAIT|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
PRIQRMAX=`echo "$STRTEMP"|grep PRIQRMAX|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
PRIQRLIST=`echo "$STRTEMP"|grep PRIQRLIST|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
QRRECEXISTS=`cat /etc/primal/primal.conf|grep "<scp$PRIQRREC>"|grep -v grep|wc -l`
if [ "$PRIQRREC" != "" ]
then
	if [ $QRRECEXISTS -lt 1 ]
	then
		echo "Error:  Query/Retrieve receiver defined but not found for receiver #$1.  Exiting..."
		logger -t primal "Error:  Query/Retrieve receiver defined but not found for receiver #$1.  Exiting..."
		exit 1
	else
		if [ "$PRIQRAGE" != "ALL" ]
		then
			if ! [[ $PRIQRAGE =~ $re ]]
			then
				echo "Error:  Query/Retrieve receiver defined but PRIQRAGE is either not defined or is not a number for receiver #$1.  Exiting..."
				logger -t primal "Error:  Query/Retrieve receiver defined but PRIQRAGE is either not defined or is not a number for receiver #$1.  Exiting..."
				exit 1
			elif [ $PRIQRAGE -lt 1 ] || [ $PRIQRAGE -gt 7300 ]
			then
				echo "Error:  Query/Retrieve receiver defined but PRIQRAGE must be between 1 and 7300 days for receiver #$1.  Exiting..."
				logger -t primal "Error:  Query/Retrieve receiver defined but PRIQRAGE must be between 1 and 7300 days for receiver #$1.  Exiting..."
				exit 1
			fi
		fi
		if ! [[ $PRIQRMAX =~ $re ]]
		then
			echo "Error:  Query/Retrieve receiver defined but PRIQRMAX is either not defined or is not a number for receiver #$1.  Exiting..."
			logger -t primal "Error:  Query/Retrieve receiver defined but PRIQRMAX is either not defined or is not a number for receiver #$1.  Exiting..."
			exit 1
		elif [ $PRIQRMAX -lt 0 ] || [ $PRIQRMAX -gt 100 ]
		then
			echo "Error:  Query/Retrieve receiver defined but PRIQRMAX must be between 0(all) and 100 for receiver #$1.  Exiting..."
			logger -t primal "Error:  Query/Retrieve receiver defined but PRIQRMAX must be between 0(all) and 100 for receiver #$1.  Exiting..."
			exit 1
		fi
		if ! [[ $PRIQRWAIT =~ $re ]]
		then
			echo "Error:  PRIQRWAIT must be a 0 or a 1.  Receiver #$1.  Exiting..."
			logger -t primal "Error:  PRIQRWAIT must be a 0 or a 1.  Receiver #$1.  Exiting..."
			exit 1
		elif [ $PRIQRWAIT -lt 0 ] || [ $PRIQRWAIT -gt 1 ]
		then
			echo "Error:  PRIQRWAIT must be a 0 or a 1.  Receiver #$1.  Exiting..."
			logger -t primal "Error:  PRIQRWAIT must be a 0 or a 1.  Receiver #$1.  Exiting..."
			exit 1
		fi
	fi
fi

PRIPASSTU=`echo "$STRTEMP"|grep PRIPASSTU|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if ! [[ $PRIPASSTU =~ $re ]]
then
    echo "Error:  Passthrough (PRIPASSTU) is not specified in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    logger -t primal "Error:  Passthrough (PRIPASSTU) is not specified in /etc/primal/primal.conf for receiver #$1.  Exiting..."
    exit 1
fi
if [ $PRIPASSTU -lt 0 ] || [ $PRIPASSTU -gt 1 ]
then
    echo "Error:  Passthrough (PRIPASSTU) must be 0 or 1 (to decompress).  Exiting..."
    logger -t primal "Error:  Passthrough (PRIPASSTU) must be 0 or 1 (to decompress).  Exiting...."
    exit 1
fi

PRIPROC=`echo "$STRTEMP"|grep PRIPROC|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ ! -d "$PRIPROC" ]
then
    echo "Error:  Processing directory does not exist for receiver #$RECNUM.  Exiting..."
    echo "`date` Error:  Processing directory does not exist for receiver #$RECNUM.  Exiting..." >> $PRILOGDIR/$PRILFPROC
    exit 1
fi

PRIOUT=`echo "$STRTEMP"|grep PRIOUT|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ ! -d "$PRIOUT" ]
then
    echo "Error:  Outbound directory $PRIOUT does not exist for receiver #$RECNUM.  Exiting..."
    echo "`date` Error:  Outbound directory $PRIOUT does not exist for receiver #$RECNUM.  Exiting..." >> $PRILOGDIR/$PRILFPROC
    exit 1
fi

PRIHOLD=`echo "$STRTEMP"|grep PRIHOLD|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ ! -d "$PRIHOLD" ]
then
    echo "Error:  Holding directory does not exist for receiver #$RECNUM.  Exiting..."
    echo "`date` Error:  Holding directory does not exist for receiver #$RECNUM.  Exiting..." >> $PRILOGDIR/$PRILFPROC
    exit 1
fi

PRISENT=`echo "$STRTEMP"|grep PRISENT|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ ! -d "$PRISENT" ]
then
    echo "Error:  Sent directory does not exist for receiver #$RECNUM.  Exiting..."
    echo "`date` Error:  Sent directory does not exist for receiver #$RECNUM.  Exiting..." >> $PRILOGDIR/$PRILFPROC
    exit 1
fi

PRIERROR=`echo "$STRTEMP"|grep PRIERROR|tr -s " "|cut -d "#" -f1|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
if [ ! -d "$PRIERROR" ]
then
    echo "Error:  Error directory does not exist for receiver #$RECNUM (how ironic).  Exiting..."
    echo "`date` Error:  Error directory does not exist for receiver #$RECNUM (how ironic).  Exiting..." >> $PRILOGDIR/$PRILFPROC
    exit 1
fi

PRIPREFIX=`echo "$STRTEMP"|grep PRITAG=|wc -l`
ISPREFIXED=0
if [ $PRIPREFIX -gt 0 ]
then
    PRITAGS=`echo "$STRTEMP"|grep PRITAG=|tr -s " "|cut -d "#" -f1|tr "\t" " "|cut -d "=" -f2|cut -d ":" -f1|tr -d "\n"`
    PRITAGS=`echo "\"$PRITAGS\""`
	PRIPRES=`echo "$STRTEMP"|grep PRITAG=|tr -s " "|cut -d "#" -f1|tr "\t" " "|cut -d ":" -f2|cut -d ":" -f1|tr "\n" ":"|sed 's/:$/\n/g'`
    PRIPRES=`echo "\"$PRIPRES\""`
	PRIMODS=`echo "$STRTEMP"|grep PRITAG=|tr -s " "|cut -d "#" -f1|tr "\t" " "|cut -d ":" -f3|cut -d " " -f1|tr "\n" ":"|sed 's/:$/\n/g'`
    PRIMODS=`echo "\"$PRIMODS\""`
	PRIINST=`echo "$STRTEMP"|grep PRITAG=|tr -s " "|cut -d "#" -f1|tr "\t" " "|cut -d ":" -f4|cut -d " " -f1|tr "\n" ":"|sed 's/:$/\n/g'`
    PRIINST=`echo "\"$PRIINST\""`
    ISPREFIXED=1
fi

PRIQRPREFIX=`echo "$STRTEMP"|grep PRIQRTAG=|wc -l`
if [ $PRIQRPREFIX -gt 0 ]
then
    PRIQRTAGS=`echo "$STRTEMP"|grep PRIQRTAG=|tr -s " "|cut -d "#" -f1|tr "\t" " "|cut -d "=" -f2|cut -d ":" -f1|tr -d "\n"`
    PRIQRTAGS=`echo "\"$PRIQRTAGS\""`
	PRIQRPRES=`echo "$STRTEMP"|grep PRIQRTAG=|tr -s " "|cut -d "#" -f1|tr "\t" " "|cut -d ":" -f2|cut -d ":" -f1|tr "\n" ":"|sed 's/:$/\n/g'`
    PRIQRPRES=`echo "\"$PRIQRPRES\""`
fi

#This section checks for custom script processing
ISCUSTOM=`echo "$PRIPRES"|grep "@o@"|wc -l`
if [ $ISCUSTOM -gt 0 ]
then
	LIST=`echo "$STRTEMP"|grep "@o@"|cut -d "(" -f2|cut -d ")" -f1|tr "," "."|tr "\n" " "`
	for ii in $LIST
	do
		FILENAMErc=`echo "$1_$ii.bash"`
		if [ ! -e "/home/dicom/scripts/$FILENAMErc" ]
		then
			echo "Error:  Custom processing sepcified for tag $ii but processing script not found.  Exiting..."
			echo "`date` Error:  Custom processing sepcified for tag $ii but processing script not found.  Exiting..." >> $PRILOGDIR/$PRILFPROC
		fi
	done
fi

LC=0
BOLEXIST=1
PRICDCR=0
while [ $LC -le 10 ] && [ $BOLEXIST -gt 0 ]
do
    BOLEXIST=`echo "$STRTEMP"|grep -c PRIDESTHIP$LC`
    if [ $BOLEXIST -gt 0 ]
    then
        PRIDESTHIP[$LC]=`echo "$STRTEMP"|grep PRIDESTHIP$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        PRIDESTPORT[$LC]=`echo "$STRTEMP"|grep PRIDESTPORT$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        PRIDESTAEC[$LC]=`echo "$STRTEMP"|grep PRIDESTAEC$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        PRIDESTCDCR[$LC]=`echo "$STRTEMP"|grep PRIDESTCDCR$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
        ISTHERE1=`echo "$STRTEMP"|grep -c PRIDESTAD$LC`
        if [ $ISTHERE1 -gt 0 ]
        then
            PRIDESTAD[$LC]=`echo "$STRTEMP"|grep PRIDESTAD$LC|tr -s " "|cut -d "=" -f2|tr "\t" " "|cut -d " " -f1`
            if [ ${PRIDESTAD[$LC]} -ne -1 ] && [ ${PRIDESTAD[$LC]} -ne 0 ] && [ ${PRIDESTAD[$LC]} -ne 1 ]
            then
    			echo "Error:  PRIDESTAD for destination $LC for receiver $RECNUM needs to be one of the following: -1, 0 or 1.  Current value is ${PRIDESTAD[$LC]}).  Exiting..."
	    		echo "`date` Error:  PRIDESTAD for destination $LC for receiver $RECNUM needs to be one of the following: -1, 0 or 1.  Current value is ${PRIDESTAD[$LC]}).  Exiting..." >> $PRILOGDIR/$PRILFPROC
		    	exit 1
            fi
        else
            PRIDESTAD[$LC]=-1
        fi
		if ! [[ ${PRIDESTCDCR[$LC]} =~ $re ]]
		then
			echo "Error:  Compression level is not set for Destionation $LC for receiver $RECNUM.  Exiting..."
			echo "`date` Error:  Compression level is not set for Destionation $LC for receiver $RECNUM.  Exiting..." >> $PRILOGDIR/$PRILFPROC
			exit 1
		elif [ ${PRIDESTCDCR[$LC]} -lt 0 -o ${PRIDESTCDCR[$LC]} -gt 2 ]
		then
			echo "Error:  Comression level must be 0 (no change), 1 (compress), 2 (decompress) for destination $LC for receiver $RECNUM (current value is ${PRIDESTCDCR[$LC]}).  Exiting..."
			echo "`date` Error:  Comression level must be 0 (no change), 1 (compress), 2 (decompress) for destination $LC for receiver $RECNUM (current value is ${PRIDESTCDCR[$LC]}).  Exiting..." >> $PRILOGDIR/$PRILFPROC
			exit 1
		fi
		if [ ${PRIDESTCDCR[$LC]} -eq 1 ]
		then
			if [ $PRICDCR -eq 0 ]
			then
				PRICDCR=1
			elif [ $PRICDCR -eq 2 ]
			then
				PRICDCR=3
			fi
		elif [ ${PRIDESTCDCR[$LC]} -eq 2 ]
		then
			if [ $PRICDCR -eq 0 ]
			then
				PRICDCR=2
			elif [ $PRICDCR -eq 1 ]
			then
				PRICDCR=3
			fi
		fi
        let LC=$LC+1
        INTNUMREC=$LC
    fi
done
NUMDEST=$LC

#Build DB connection string
DBNAME=`cat /etc/primal/primal.db|grep DBNAME|cut -d "=" -f2-`
DBUSER=`cat /etc/primal/primal.db|grep DBUSER|cut -d "=" -f2-`
DBPASS=`cat /etc/primal/primal.db|grep DBPASS|cut -d "=" -f2-`
DBHOST=`cat /etc/primal/primal.db|grep DBHOST|cut -d "=" -f2-`
DBPORT=`cat /etc/primal/primal.db|grep DBPORT|cut -d "=" -f2-`

DBCONN=`echo "mysql -u $DBUSER -p"$DBPASS" -h $DBHOST -P $DBPORT $DBNAME"`
DBCONNN=`echo "mysql -N -u $DBUSER -p"$DBPASS" -h $DBHOST -P $DBPORT $DBNAME"`


