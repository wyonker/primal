#!/bin/bash

LASTMONTH=`date +%m -d "last month"`

if [ $"LASTMONTH" == "12" ]
then
        YEARSEARCH=`date +%Y -d "last year"`
else
        YEARSEARCH=`date +%Y`
fi

CURSTUDIES=`echo "select count(*) from patient as p join receive as r on p.puid=r.puid join study as s on s.puid=p.puid where s.StudyDate between date_sub(r.tstartrec, interval 48 hour) and date_add(r.tstartrec, interval 48 hour) and s.StudyDate like '$YEARSEARCH-$LASTMONTH%';"|mysql -u root -N primal`
TOTALSTUDIES=`echo "select count(*) from patient as p join receive as r on p.puid=r.puid join study as s on s.puid=r.puid where s.StudyDate like '$YEARSEARCH-$LASTMONTH%';"|mysql -u root -N primal`
let PRIORSTUDIES=$TOTALSTUDIES-$CURSTUDIES

#mail -s "Study Count for `hostname`" yonkerw@ccf.org << INLINE
echo "`hostname` `date`  Studies for $YEARSEARCH-$LASTMONTH "
echo "Total: $TOTALSTUDIES   Current:  $CURSTUDIES   Priors:  $PRIORSTUDIES"
#INLINE

exit 0
