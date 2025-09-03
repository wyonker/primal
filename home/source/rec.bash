#!/bin/bash

# Should get path, rec_id, aet, aec
#REV 4

FULLPATH=`echo "$1"|cut -d " " -f1`
RECID=`echo "$1"|cut -d " " -f2`
SENDERAET=`echo "$1"|cut -d " " -f3`
CALLINGAET=`echo "$1"|cut -d " " -f4`

PUID=`echo "$FULLPATH"|rev|cut -d '/' -f1|rev`

echo "INSERT INTO receive SET puid = $PUID, fullpath = $FULLPATH, rservername = `hostname`, rec_id = $RECID, tstartrec = NOW(), senderAET = $SENDERAET, callingAET = $CALLINGAET;" | mysql -N -u root primal
