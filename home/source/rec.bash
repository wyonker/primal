#!/bin/bash

# Should get path, rec_id, aet, aec
#REV 15

FULLPATH=`echo "$1"|cut -d " " -f1`
RECID=`echo "$1"|cut -d " " -f2`
SENDERAET=`echo "$1"|cut -d " " -f3`
CALLINGAET=`echo "$1"|cut -d " " -f4`
FILENAME=`echo "$1"|cut -d " " -f5`
SIUID=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0020,000d"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SERUID=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0020,000e"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SOPIUID=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0018"|head -1|cut -d "[" -f2|cut -d "]" -f1`
STUDYDESC=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,1030"|head -1|cut -d "[" -f2|cut -d "]" -f1`
ACCN=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0050"|head -1|cut -d "[" -f2|cut -d "]" -f1`
PNAME=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0010,0010"|head -1|cut -d "[" -f2|cut -d "]" -f1`
TEMP1=`echo "$PNAME"|cut -d "^" -f1`
TEMP2=`echo "$PNAME"|cut -d "^" -f2`
PNAMESEARCH=`echo "$TEMP1"^"$TEMP2"`
MRN=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0010,0020"|head -1|cut -d "[" -f2|cut -d "]" -f1`
DOB=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0010,0030"|head -1|cut -d "[" -f2|cut -d "]" -f1`
SERIESDESC=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,103e"|head -1|cut -d "[" -f2|cut -d "]" -f1`
STUDYDATE=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0020"|head -1|cut -d "[" -f2|cut -d "]" -f1`
STUDYTIME=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0030"|head -1|cut -d "[" -f2|cut -d "]" -f1|cut -d "." -f1`
TEMPY=`echo "$STUDYDATE"|cut -c1-4`
TEMPMO=`echo "$STUDYDATE"|cut -c5-6`
TEMPD=`echo "$STUDYDATE"|cut -c7-8`
TEMPH=`echo "$STUDYTIME"|cut -c1-2`
TEMPM=`echo "$STUDYTIME"|cut -c3-4`
TEMPS=`echo "$STUDYTIME"|cut -c5-6`
STUDYDATETIME=`echo "$TEMPY"-"$TEMPMO"-"$TEMPD" "$TEMPH":"$TEMPM":"$TEMPS"`
MODALITY=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0060"|head -1|cut -d "[" -f2|cut -d "]" -f1`
ORG=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0050"|head -1|cut -d "[" -f2|cut -d "]" -f1|rev|cut -c 1-3|rev`
HOSTNAME=`hostname`

PUID=`echo "$FULLPATH"|rev|cut -d '/' -f1|rev`

#see if the patient exists
UPID=0
UPID=`echo "SELECT id FROM patient WHERE pname=\"$PNAME\" AND dob=\"$DOB\" AND pid=\"$MRN\" AND org=\"$ORG\";" | mysql -N -u root primal`
if [ -z "$UPID" ]; then
    #insert patient
    echo "INSERT INTO patient SET pname=\"$PNAME\", org=\"$ORG\", pid=\"$MRN\", dob=\"$DOB\";" | mysql -N -u root primal
    UPID=`echo "SELECT id FROM patient WHERE pname=\"$PNAME\" AND dob=\"$DOB\" AND pid=\"$MRN\" AND org=\"$ORG\";" | mysql -N -u root primal`
fi

#First check to see if we inserted this study already in the receive
NUMSTUDIES=`echo "SELECT COUNT(*) FROM receive WHERE puid = \"$PUID\" AND rservername = \"$HOSTNAME\";" | mysql -N -u root primal`
if [ "$NUMSTUDIES" -eq 0 ]; then
    echo "INSERT INTO receive SET puid = \"$PUID\", fullpath = \"$FULLPATH\", rservername = \"$HOSTNAME\", SIUID=\"$SIUID\", tstartrec = NOW(), senderAET = \"$SENDERAET\", callingAET = \"$CALLINGAET\", rec_images=1;" | mysql -N -u root primal
else
    `echo "UPDATE receive SET rec_images = rec_images + 1 WHERE puid = \"$PUID\" AND rservername = \"$HOSTNAME\" limit 1;" | mysql -N -u root primal`
fi
#First check to see if we inserted this study already
NUMSTUDIES=`echo "SELECT COUNT(*) FROM study WHERE puid=\"$PUID\" AND SIUID=\"$SIUID\";" | mysql -N -u root primal`
if [ "$NUMSTUDIES" -eq 0 ]; then
    echo "INSERT INTO study SET puid=\"$PUID\", upid=\"$UPID\", SIUID=\"$SIUID\", sServerName=\"$HOSTNAME\", studyDesc=\"$STUDYDESC\", AccessionNum=\"$ACCN\", StudyDate=\"$STUDYDATETIME\", StudyModType=\"$MODALITY\", sClientID=\"$ORG\", StudyNumImg=1;" | mysql -N -u root primal
else
    `echo "UPDATE study SET StudyNumImg = StudyNumImg + 1 WHERE puid=\"$PUID\" AND SIUID=\"$SIUID\" limit 1;" | mysql -N -u root primal`
fi
#First check to see if we inserted this series already
NUMSERIES=`echo "SELECT COUNT(*) FROM series WHERE puid = \"$PUID\" AND SERIUID = \"$SERUID\";" | mysql -N -u root primal`
if [ "$NUMSERIES" -eq 0 ]; then
    echo "INSERT INTO series SET puid = \"$PUID\", SERIUID = \"$SERUID\", SIUID=\"$SIUID\", SeriesDesc=\"$SERIESDESC\", SeriesNumImg=1;" | mysql -N -u root primal
else
    `echo "UPDATE series SET SeriesNumImg = SeriesNumImg + 1 WHERE puid=\"$PUID\" AND SERIUID=\"$SERUID\" limit 1;" | mysql -N -u root primal`
fi
#add to image table
echo "INSERT INTO image SET puid=\"$PUID\", SOPIUID=\"$SOPIUID\", SERIUID=\"$SERUID\", iservername=\"$HOSTNAME\", ifilename=\"$FILENAME\", idate=NOW(), ilocation=\"$FULLPATH\";" | mysql -N -u root primal

