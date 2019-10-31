#!/bin/bash

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
SERIESINFO=`dcmdump $FILENAME|egrep '(0010,0010)|(0010,0020)'`
PNAME=`echo "$SERIESINFO"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PAPID=`echo "$SERIESINFO"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
NUMFILES=`ls -1 $PRIPROC/$DIRNAME/*|wc -l`
echo "`date` Started processing study for Patient: $PNAME MRN: $PAPID with $NUMFILES images." >> $PRILOGDIR/$PRILFPROC

ls -1 $PRIPROC/$DIRNAME > /tmp/$DIRNAME.txt
echo "#!/bin/bash" > /tmp/$DIRNAME.bash
echo "" >> /tmp/$DIRNAME.bash
awk -v DIRNAME=$DIRNAME -v PRIPROC=$PRIPROC -v PRIPREFIX=$PRIPREFIX -v PRITAGS=$PRITAGS -v PRILOGDIR=$PRILOGDIR -v PRILFPROC=$PRILFPROC -v PRILL=$PRILL '{
	printf("/home/dicom/rename.bash %s/%s/%s %s %s %s %s %s &\n", PRIPROC, DIRNAME, $0, PRIPREFIX, PRITAGS, PRILOGDIR, PRILFPROC, PRILL);
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
	echo "`date` Processing failed for $DIRNAME.  Study moved to $PRIERROR/$DIRNAME."  >> $PRILOGDIR/$PRILFPROC
else
	mv $PRIPROC/$DIRNAME $PRIOUT/ >> $PRILOGDIR/$PRILFPROC
	/home/dicom/send.bash $1 $PRIOUT/$DIRNAME &
fi

ENDPROCTIME=`date +%s`
PROCTIME=`echo "$ENDPROCTIME-$STARTPROCTIME"|bc 2>/dev/null`
IMGSEC=`echo "scale=2; $NUMFILES/$PROCTIME"|bc 2>/dev/null`
echo "`date` Finished processing study for Patient: $PNAME MRN: $PAPID with $NUMFILES images in $PROCTIME seconds @ $IMGSEC image(s)/sec." >> $PRILOGDIR/$PRILFPROC

rm -f /tmp/$DIRNAME.bash
rm -f /tmp/$DIRNAME.txt
