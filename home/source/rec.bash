#!/bin/bash

# Should get path, rec_id, aet, aec


FULLPATH=$1
PUID=`echo "$1"|rev|cut -d '/' -f1|rev`

echo "INSERT INTO receive SET puid = $PUID, fullpath = $FULLPATH, rservername = `hostname`, rec_id = $2, tstartrec = NOW(), senderAET = $3, callingAET = $4;" | mysql -N -u root primal
