#!/bin/bash -x

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

PDEPTH=`echo "$PRIPROC"|awk -F "/" '{print NF+2}'`
FILENAME=`ls -1 $2|head -1|tail -1`
IMAGEDUMP=`/home/dicom/bin/dcmdump $PRIPROC/$DIRNAME/$FILENAME|egrep '(0002,0010)|(0008,0018)|(0008,0050)|(0010,0010)|(0010,0020)|(0010,0030)|(0010,4000)|(0020,000d)|(0020,000e)'`
TRANSSYN=`echo "$IMAGEDUMP"|grep "(0002,0010)"|head -1|cut -d "=" -f2|cut -d "#" -f1|tr -s " "`
SOPIUID=`echo "$IMAGEDUMP"|grep "(0008,0018)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
ACCN=`echo "$IMAGEDUMP"|grep "(0008,0050)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PNAME=`echo "$IMAGEDUMP"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
MRN=`echo "$IMAGEDUMP"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PDOB=`echo "$IMAGEDUMP"|grep "(0010,0030)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PATIENTCOMMENTS=`echo "$IMAGEDUMP"|grep "(0010,4000)"|head -1|cut -d "[" -f2|cut -d "]" -f1|tr -s " "`
SIUID=`echo "$IMAGEDUMP"|grep "(0020,000d)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SERIUID=`echo "$IMAGEDUMP"|grep "(0020,000e)"|head -1|cut -d "[" -f2|cut -d "]" -f1`

STARTDATE="`date -d "yesterday" +%Y-%m-%d` 00:00:00"
ENDATE="`date +%Y-%m-%d` 23:59:59"

#Search by MRN in patientcomments field
RESULTS=`echo "select p.pname, p.pid, p.pdob, p.puid, p.PatientComments, s.siuid, s.puid, s.StudyDate, r.tstartrec from patient as p left join study as s on p.puid=s.puid left join receive as r on p.puid=r.puid where p.PatientComments like '%$MRN%' and p.puid like '1_%' and r.tstartrec BETWEEN '$STARTDATE' AND '$ENDATE' group by p.pname, p.pid;"|$DBCONNN`

if [ "$RESULTS" == "" ] || [ "$RESULTS" == " " ]
then
    NUMRESULTS=0
else
    NUMRESULTS=`echo "$RESULTS"|wc -l`
fi


#Search by the current MRN in case the patient was normalized
if [ $NUMRESULTS -eq 0 ]
then
	RESULTS=`echo "select p.pname, p.pid, p.pdob, p.puid, p.PatientComments, s.siuid, s.puid, s.StudyDate, r.tstartrec from patient as p left join study as s on p.puid=s.puid left join receive as r on p.puid=r.puid where p.pid like '%$MRN%' and p.puid like '1_%' and r.tstartrec BETWEEN '$STARTDATE' AND '$ENDATE' group by p.pname, p.pid;"|$DBCONNN`
	if [ "$RESULTS" == "" ] || [ "$RESULTS" == " " ]
	then
		NUMRESULTS=0
	else
		NUMRESULTS=`echo "$RESULTS"|wc -l`
	fi
fi

#Search by patient name and dob
if [ $NUMRESULTS -eq 0 ]
then
	RESULTS=`echo "select p.pname, p.pid, p.pdob, p.puid, s.siuid, s.puid, s.StudyDate, r.tstartrec from patient as p left join study as s on p.puid=s.puid left join receive as r on p.puid=r.puid where p.pname='$PNAME' and p.pdob='$PDOB' and p.puid like '1_%' and r.tstartrec BETWEEN '$STARTDATE' AND '$ENDATE' group by p.pname, p.pid;"|$DBCONNN`
	if [ "$RESULTS" == "" ] || [ "$RESULTS" == " " ]
	then
		NUMRESULTS=0
	else
		NUMRESULTS=`echo "$RESULTS"|wc -l`
	fi
fi

if [ $NUMRESULTS -gt 1 ]
then
	echo "`date` ERROR:  Multiple matching patients found for patient $PNAME, MRN $MRN, PRIMAL $DIRNAME." >> $PRILOGDIR/$PRILFPROC 2>&1
	echo "update process set perror='1' where puid='$DIRNAME';|$DBCONN"
	echo "`date` ERROR:  Multiple matching patients found for patient $PNAME, MRN $MRN, PRIMAL $DIRNAME." > $PRIPROC/$DIRNAME/error.txt 2>&1
	exit 95
elif [ $NUMRESULTS -lt 1 ]
then
	echo "`date` ERROR:  No matching patients found for patient $PNAME, MRN $MRN, PRIMAL $DIRNAME." >> $PRILOGDIR/$PRILFPROC 2>&1
	echo "update process set perror='1' where puid='$DIRNAME';|$DBCONN"
	echo "`date` ERROR:  No matching patients found for patient $PNAME, MRN $MRN, PRIMAL $DIRNAME." > $PRIPROC/$DIRNAME/error.txt 2>&1
	exit 95
fi

RESULTS=`echo "$RESULTS"|tr "\t" "|"|tr -s " "`
NEWMRN=`echo "$RESULTS"|cut -d "|" -f2`
echo "$NEWMRN"
exit 0
