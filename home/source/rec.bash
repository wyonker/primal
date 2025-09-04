#!/bin/bash

# Should get path, rec_id, aet, aec
#REV 7

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
STUDYDATE=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0020"|head -1|cut -d "[" -f2|cut -d "]" -f1`
STUDYTIME=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0030"|head -1|cut -d "[" -f2|cut -d "]" -f1|cut -d "." -f1`
MODALITY=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0060"|head -1|cut -d "[" -f2|cut -d "]" -f1`
ORG=`dcmdump "$FULLPATH/$FILENAME" 2>&1|grep "0008,0050"|head -1|cut -d "[" -f2|cut -d "]" -f1|rev|cut -c 1-3|rev`
HOSTNAME=`hostname`

PUID=`echo "$FULLPATH"|rev|cut -d '/' -f1|rev`

#First check to see if we inserted this study already in the receive
NUMSTUDIES=`echo "SELECT COUNT(*) FROM receive WHERE puid = \"$PUID\" AND rservername = \"$HOSTNAME\";" | mysql -N -u root primal`
if [ "$NUMSTUDIES" -eq 0 ]; then
    echo "INSERT INTO receive SET puid = \"$PUID\", fullpath = \"$FULLPATH\", rservername = \"$HOSTNAME\", rec_id = $RECID, tstartrec = NOW(), senderAET = \"$SENDERAET\", callingAET = \"$CALLINGAET\";" | mysql -N -u root primal
fi
#First check to see if we inserted this study already
NUMSTUDIES=`echo "SELECT COUNT(*) FROM study WHERE puid=\"$PUID\" AND SIUID=\"$SIUID\";" | mysql -N -u root primal`
if [ "$NUMSTUDIES" -eq 0 ]; then
    echo "INSERT INTO study SET puid=\"$PUID\", SIUID=\"$SIUID\", sServerName=\"$HOSTNAME\", studyDesc=\"$STUDYDESC\", AccessionNum=\"$ACCN\", StudyDate=\"$STUDYDATE $STUDYTIME\", StudyModType=\"$MODALITY\", sClientID=\"$ORG\", StudyNumImg=1;" | mysql -N -u root primal
else
    `echo "UPDATE study SET StudyNumImg = StudyNumImg + 1 WHERE puid=\"$PUID\" AND SIUID=\"$SIUID\" limit 1;" | mysql -N -u root primal`
fi
#First check to see if we inserted this series already
NUMSERIES=`echo "SELECT COUNT(*) FROM series WHERE puid = \"$PUID\" AND seruid = \"$SERUID\";" | mysql -N -u root primal`
if [ "$NUMSERIES" -eq 0 ]; then
    echo "INSERT INTO series SET puid = \"$PUID\", seruid = \"$SERUID\", iservername = \"$HOSTNAME\", ifilename = \"$FILENAME\", idate = NOW(), ilocation = \"$FULLPATH\", SeriesNumImg=1;" | mysql -N -u root primal
else
    `echo "UPDATE series SET SeriesNumImg = SeriesNumImg + 1 WHERE puid=\"$PUID\" AND seruid=\"$SERUID\" limit 1;" | mysql -N -u root primal`
fi
#add to image table
echo "INSERT INTO image SET puid=\"$PUID\", SOPIUID=\"$SOPIUID\", iservername=\"$HOSTNAME\", ifilename=\"$FILENAME\", idate=NOW(), ilocation=\"$FULLPATH\";" | mysql -N -u root primal

