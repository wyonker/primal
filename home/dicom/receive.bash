#!/bin/bash
# Version 3.00.00b1
# Build 1
# 2015-09-11

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

if [ -e /tmp/$DIRNAME.bash ]
then
	rm -f /tmp/$DIRNAME.bash
fi

STARTPROCTIME=`date +%s`
STRTEMP1="`cat /tmp/$DIRNAME` $STARTPROCTIME"
echo "$STRTEMP1" > /tmp/$DIRNAME
FILENAME=`ls -1 $PRIPROC/$DIRNAME/*|head -1`
SERIESINFO=`dcmdump $FILENAME|egrep '(0010,0010)|(0010,0020)|(0020,000d)'`
PNAME=`echo "$SERIESINFO"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PAPID=`echo "$SERIESINFO"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SIUID=`echo "$SERIESINFO"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
NUMFILES=`ls -1 $PRIPROC/$DIRNAME/*|wc -l`
SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
echo "`date` Started processing study for Patient: $PNAME MRN: $PAPID with $NUMFILES images." >> $PRILOGDIR/$PRILFPROC
echo "insert into process (puid, tstartproc) values ('$DIRNAME', '`date +"%Y-%m-%d %H:%M:%S"`');"|mysql primal
echo "insert into event (event_id, SIUID, event_type, num_img, event_uname, ts) values (now(), '$SIUID', 'mod_start', $NUMFILES, '`hostname -a`', '$SDATETIME');"|cqlsh -k test;

#Need to process any custom scripting for tags here.  
#This will call the custom file that has a name in the format of
#<receiver #>_<tag id>.bash 
#Where the , in the tag id is replaced with a .
NEEDCUSTOM=`echo "$PRIPRES"|grep "@o@"|wc -l`
if [ $NEEDCUSTOM -gt 0 ]
then
	TEMPTAGS=`echo "$PRITAGS"|tr ")" "\n"|cut -d "(" -f2`
	TEMPPRES=`echo "$PRIPRES"|tr ":" "\n"`
	TAGLINES=`echo "$TEMPTAGS"|wc -l`
	LC=1
	while [ $LC -lt $TAGLINES ]
	do
		READLINE=`echo "$TEMPPRES"|head -$LC|tail -1`
		ISCUSTOM=`echo "$READLINE"|grep "@o@"|wc -l`
		if [ $ISCUSTOM -gt 0 ]
		then
			TEMPFILENAME=`echo "$TEMPTAGS"|head -$LC|tail -1|tr "," "."`
			TAGPASS=`echo "$TEMPTAGS"|head -$LC|tail -1`
			FILENAME=`echo "$1_$TEMPFILENAME.bash"`
			NEWPRESTEMP=`/home/dicom/scripts/$FILENAME $2 $TAGPASS`
			if [ $? -ne 0 ]
			then
				SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
				echo "`date` Processing failed for $DIRNAME.  Study moved to $PRIERROR/$DIRNAME."  >> $PRILOGDIR/$PRILFPROC
				echo "update process set tendproc='`date +"%Y-%m-%d %H:%M:%S"`', perror='1' where process.puid='$DIRNAME';"|mysql primal;
				echo "insert into event (event_id, SIUID, event_type, num_img, event_uname, ts) values (now(), '$SIUID', 'mod_finished', $NUMFILES, '`hostname -a`', '$SDATETIME');"|cqlsh -k test;
				INTISERROR=1
			fi
		else
			NEWPRESTEMP=`echo "$READLINE"`
		fi
		if [ $LC -eq 1 ]
		then
			NEWPRES=`echo '"'"$NEWPRESTEMP"`
		else
			NEWPRES=`echo "$NEWPRES:$NEWPRESTEMP"`
		fi
		let LC=$LC+1
	done
	PRIPRES=`echo "$NEWPRES"`
fi

ls -1 $PRIPROC/$DIRNAME > /tmp/$DIRNAME.txt
echo "#!/bin/bash" > /tmp/$DIRNAME.bash
echo "" >> /tmp/$DIRNAME.bash
awk -v DIRNAME=$DIRNAME -v PRIPROC=$PRIPROC -v PRIPREFIX=$PRIPREFIX -v PRITAGS=$PRITAGS -v PRILOGDIR=$PRILOGDIR -v PRILFPROC=$PRILFPROC -v PRILL=$PRILL -v PRIPRES="$PRIPRES" -v PRIMODS="$PRIMODS" -v PRIINST="$PRIINST" '{
	printf("/home/dicom/rename.bash %s/%s/%s %s %s %s %s %s %s %s %s &\n", PRIPROC, DIRNAME, $0, PRIPREFIX, PRITAGS, PRILOGDIR, PRILFPROC, PRILL, PRIPRES, PRIMODS, PRIINST);
}' /tmp/$DIRNAME.txt >> /tmp/$DIRNAME.bash
chmod 755 /tmp/$DIRNAME.bash

RENAMELOAD=`ps -ef|egrep -ci "rename.bash|dcmodify|dcmdump"`
while [ $RENAMELOAD -gt 10000 ]
do
	echo "`date` Warning:  $RENAMELOAD rename processes found.  Waiting for patient $PNAME, MRN $PAPID..." >> $PRILOGDIR/$PRILFPROC
	sleep 5
	RENAMELOAD=`ps -ef|egrep -ci "rename.bash|dcmodify|dcmdump"`
done
/tmp/$DIRNAME.bash >> $PRILOGDIR/$PRILFPROC

STILLRUN=`ps -ef|grep rename.bash|grep $DIRNAME|grep -v grep|wc -l`
while [ $STILLRUN -gt 0 ]
do
	echo "`date` $STILLRUN modification(s) still running for Patient $PNAME.  Sleeping..." >> $PRILOGDIR/$PRILFPROC
	STILLRUN=`ps -ef|grep rename.bash|grep $DIRNAME|grep -v grep|wc -l`
	sleep 3
done

if [ -e $PRIPROC/$DIRNAME/error.txt ]
then
	mv $PRIPROC/$DIRNAME $PRIERROR >> $PRILOGDIR/$PRILFPROC
	SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
	echo "`date` Processing failed for $DIRNAME.  Study moved to $PRIERROR/$DIRNAME."  >> $PRILOGDIR/$PRILFPROC
	echo "update process set tendproc='`date +"%Y-%m-%d %H:%M:%S"`', perror='1' where process.puid='$DIRNAME';"|mysql primal;
	echo "insert into event (event_id, SIUID, event_type, num_img, event_uname, src_dest, ts) values (now(), '$SIUID', 'mod_error', $NUMFILES, '`hostname -a`', 'Processing failed', '$SDATETIME');"|cqlsh -k test;
	INTISERROR=1
else
	mv $PRIPROC/$DIRNAME $PRIOUT/ >> $PRILOGDIR/$PRILFPROC
	/home/dicom/send.bash $1 $PRIOUT/$DIRNAME & >> $PRILOGDIR/$PRILFOUT 2>&1
fi

ENDPROCTIME=`date +%s`
STRTEMP1="`cat /tmp/$DIRNAME` $ENDPROCTIME"
echo "$STRTEMP1" > /tmp/$DIRNAME
PROCTIME=`echo "$ENDPROCTIME-$STARTPROCTIME"|bc 2>/dev/null`
IMGSEC=`echo "scale=2; $NUMFILES/$PROCTIME"|bc 2>/dev/null`
echo "`date` Finished processing study for Patient: $PNAME MRN: $PAPID with $NUMFILES images in $PROCTIME seconds @ $IMGSEC image(s)/sec." >> $PRILOGDIR/$PRILFPROC
if [ "$INTISERROR" != "1" ]
then
	SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
	echo "update process set tendproc='`date +"%Y-%m-%d %H:%M:%S"`', perror='0' where process.puid='$DIRNAME';"|mysql primal;
	echo "insert into event (event_id, SIUID, event_type, num_img, event_uname, ts) values (now(), '$SIUID', 'mod_finished', $NUMFILES, '`hostname -a`', '$SDATETIME');"|cqlsh -k test;
fi

rm -f /tmp/$DIRNAME.bash
rm -f /tmp/$DIRNAME.txt
