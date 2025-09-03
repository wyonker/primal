#!/bin/bash

# Should get path, rec_id, aet, aec


FULLPATH=`echo "$1"|cut -d " " -f1`
SENDERAET=`echo "$1"|cut -d " " -f3`
CALLINGAET=`echo "$1"|cut -d " " -f4`

PUID=`echo "$1"|rev|cut -d '/' -f1|rev`

echo "INSERT INTO receive SET puid = $PUID, fullpath = $FULLPATH, rservername = `hostname`, rec_id = $2, tstartrec = NOW(), senderAET = $SENDERAET, callingAET = $CALLINGAET;" | mysql -N -u root primal
