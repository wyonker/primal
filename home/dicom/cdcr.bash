#!/bin/bash
# Version 3.00.00b1
# Build 1
# 2015-09-11

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

PDEPTH=`echo "$PRIPROC"|awk -F "/" '{print NF+2}'`
RETVAL=0

#We may need to update the transfer syntax for each file
LIST=`ls -1 $PRIPROC/$DIRNAME/* 2>/dev/null|cut -d "/" -f$PDEPTH`
for i in $LIST
do
	#Pull info from image file just received.
	IMAGEDUMP=`/home/dicom/bin/dcmdump $PRIPROC/$DIRNAME/$i|egrep '(0002,0010)|(0008,0018)|(0020,000d)|(0020,000e)'`
	TRANSSYN=`echo "$IMAGEDUMP"|grep "(0002,0010)"|head -1|cut -d "=" -f2|cut -d "#" -f1|tr -s " "`
	SOPIUID=`echo "$IMAGEDUMP"|grep "(0008,0018)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
	SIUID=`echo "$IMAGEDUMP"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
	SERIUID=`echo "$IMAGEDUMP"|grep "(0020,000e)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
	ISFOUND=`echo "select count(*) from image as i left join series as s on s.SERIUID = i.SERIUID where s.SIUID = '$SIUID' and i.SOPIUID = '$SOPIUID' and i.PUID = '$DIRNAME';"|$DBCONN|tail -1`
	if [ $ISFOUND -lt 1 ]
	then
		echo "insert into image (puid, SERIUID, SOPIUID, ifilename, TransferSyntaxUID) values ('$DIRNAME', '$SERIUID', '$SOPIUID', '$i', '$TRANSSYN');"|$DBCONN;
	else
		echo "update image set TransferSyntaxUID = '$TRANSSYN' where SOPIUID = '$SOPIUID' and PUID = '$DIRNAME';"|$DBCONN
	fi
done

if [ $PRICDCR -eq 1 ] || [ $PRICDCR -eq 3 ]
then
	#We need to compress this study.  Let's see if it is compressed already.
	TSUID=`echo "select TransferSyntaxUID from image as i left join series as s on s.SERIUID = i.SERIUID where s.SIUID = '$SIUID' and i.PUID = '$DIRNAME' limit 1;"|$DBCONN|tail -1`
	ISFOUND=`echo "$TSUID"|awk -F "JPEG" '{print NF-1}'`
	ISFOUND2=`echo "$TSUID"|awk -F "MPEG" '{print NF-1}'`
	if [ $ISFOUND -gt 0 ] || [ $ISFOUND2 -gt 0 ]
	then
		#Study is already compressed or it's a MPEG.  Link to the original.
		PDEPTH2=`echo "$PRIPROC"|awk -F "/" '{print NF+2}'`
		LIST=`ls -1 $PRIPROC/$DIRNAME/*.dcm 2>/dev/null|cut -d "/" -f$PDEPTH`
		cd $PRIPROC/$DIRNAME
		for i in $LIST
		do
			PDEPTH3=`echo "$i"|awk -F "." '{print NF-1}'`
			FILENAME=`ls -1 $PRIPROC/$DIRNAME/$i 2>/dev/null|cut -d "/" -f$PDEPTH2|cut -d "." -f-$PDEPTH3`
			ln -s $i $FILENAME.j2k >> $PRILOGDIR/$PRILFIN 2>&1
		done
		sleep 1
	else
		#Need to compress this guy.
		#LIST=`ls -1 $PRIPROC/$DIRNAME/* 2>/dev/null|cut -d "/" -f$PDEPTH`
		PDEPTH2=`echo "$PRIPROC"|awk -F "/" '{print NF+3}'`
        cd $PRIPROC/$DIRNAME
		mkdir $PRIPROC/$DIRNAME/tmp
		/home/dcm4che/bin/dcm2dcm -f --jpll $PRIPROC/$DIRNAME/*.dcm $PRIPROC/$DIRNAME/tmp/ >> $PRILOGDIR/$PRILFIN 2>&1
		if [ $? -eq 1 ]
		then
			RETVAL=95
		fi
		LIST=`ls -1 $PRIPROC/$DIRNAME/tmp/*.dcm 2>/dev/null|cut -d "/" -f$PDEPTH2`
		for i in $LIST
		do
			PDEPTH3=`echo "$i"|awk -F "." '{print NF-1}'`
			FILENAME=`ls -1 $PRIPROC/$DIRNAME/tmp/$i 2>/dev/null|cut -d "/" -f$PDEPTH2|cut -d "." -f-$PDEPTH3`
			mv $PRIPROC/$DIRNAME/tmp/$i $PRIPROC/$DIRNAME/$FILENAME.j2k
		done
		rmdir $PRIPROC/$DIRNAME/tmp
	fi
fi

if [ $PRICDCR -eq 2 ] || [ $PRICDCR -eq 3 ]
then
	#We need to uncompress this study.  Let's see if it's compressed already.
	TSUID=`echo "select TransferSyntaxUID from image as i left join series as s on s.SERIUID = i.SERIUID where s.SIUID = '$SIUID' and i.PUID = '$DIRNAME' limit 1;"|$DBCONN|tail -1`
	ISFOUND=`echo "$TSUID"|awk -F "Lossless" '{print NF-1}'`
	if [ $ISFOUND -lt 1 ]
	then
		#Study is not compressed or is Lossy compressed.  Link to the original.
		PDEPTH2=`echo "$PRIPROC"|awk -F "/" '{print NF+2}'`
		LIST=`ls -1 $PRIPROC/$DIRNAME/*.dcm 2>/dev/null|cut -d "/" -f$PDEPTH`
        cd $PRIPROC/$DIRNAME
        for i in $LIST
        do
			PDEPTH3=`echo "$i"|awk -F "." '{print NF-1}'`
			FILENAME=`ls -1 $PRIPROC/$DIRNAME/$i 2>/dev/null|cut -d "/" -f$PDEPTH2|cut -d "." -f-$PDEPTH3`
            ln -s $i $FILENAME.ucr >> $PRILOGDIR/$PRILFIN 2>&1
        done
		sleep 1
		exit 0
	else
		#Study is compressed.  Need to uncompress it.
		#LIST=`ls -1 $PRIPROC/$DIRNAME/* 2>/dev/null|cut -d "/" -f$PDEPTH`
		PDEPTH2=`echo "$PRIPROC"|awk -F "/" '{print NF+3}'`
        cd $PRIPROC/$DIRNAME
		mkdir $PRIPROC/$DIRNAME/tmp
		/home/dcm4che/bin/dcm2dcm -f -t 1.2.840.10008.1.2 $PRIPROC/$DIRNAME/*.dcm $PRIPROC/$DIRNAME/tmp/ >> $PRILOGDIR/$PRILFIN 2>&1
		if [ $? -eq 1 ]
		then
			RETVAL=95
		fi
		LIST=`ls -1 $PRIPROC/$DIRNAME/tmp/*.dcm 2>/dev/null|cut -d "/" -f$PDEPTH2`
		for i in $LIST
		do
			PDEPTH3=`echo "$i"|awk -F "." '{print NF-1}'`
			FILENAME=`ls -1 $PRIPROC/$DIRNAME/tmp/$i 2>/dev/null|cut -d "/" -f$PDEPTH2|cut -d "." -f-$PDEPTH3`
			mv $PRIPROC/$DIRNAME/tmp/$i $PRIPROC/$DIRNAME/$FILENAME.ucr
		done
		rmdir $PRIPROC/$DIRNAME/tmp
	fi
fi

exit $RETVAL
