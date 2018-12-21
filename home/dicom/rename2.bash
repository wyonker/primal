#!/bin/bash
# Version 3.00.00b20
# Build 8
# 2015-12-10
# License GPLv3

TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`
FILENAMErn2=`echo $3`

RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

source readconf.bash

if [ -e /tmp/$DIRNAME.bash ]
then
    rm -f /tmp/$DIRNAME.bash
fi

DICOMDIR="/home/dicom/bin"
DEBUG=0

let TEMPVAR=$TEMPVAR-1
PREFIX=$PRIPREFIX
TAGIDS=`echo $PRITAGS|sed 's/)/) /g'`
TAGIDS=`echo $TAGIDS|sed 's/ $//g'`
TAGIDS=`echo "$TAGIDS"|sed 's/^"//g'`
TAGIDS=`echo "$TAGIDS"|sed 's/"$//g'`
PRIINST=`echo "$PRIINST"|sed 's/^"//g'`
PRIINST=`echo "$PRIINST"|sed 's/"$//g'`
PRIPRES=`echo "$PRIPRES"|cut -d "\"" -f2`
PRIMODS=`echo "$PRIMODS"|sed 's/^"//g'`
PRIMODS=`echo "$PRIMODS"|sed 's/"$//g'`

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
            TEMPFILENAMEcp1=`echo "$TEMPTAGS"|head -$LC|tail -1|tr "," "."`
            TAGPASS=`echo "$TEMPTAGS"|head -$LC|tail -1`
            FILENAMEcp1=`echo "$1_$TEMPFILENAMEcp1.bash"`
            NEWPRESTEMP=`/home/dicom/scripts/$FILENAMEcp1 $1 $2 $TAGPASS`
            if [ $? -ne 0 ]
            then
                SDATETIME=`date "+%Y-%m-%d %H:%M:%S"`
                echo "`date` Processing failed for $DIRNAME.  Study moved to $PRIERROR/$DIRNAME."  >> $PRILOGDIR/$PRILFPROC
                echo "update process set tendproc='`date +"%Y-%m-%d %H:%M:%S"`', perror='1' where process.puid='$DIRNAME';"|$DBCONN
                touch $PRIPROC/$DIRNAME/error.txt
				exit 95
            fi
        else
            NEWPRESTEMP=`echo "$READLINE"`
        fi
        if [ $LC -eq 1 ]
        then
            NEWPRES=`echo "$NEWPRESTEMP"`
        else
            NEWPRES=`echo "$NEWPRES:$NEWPRESTEMP"`
        fi
        let LC=$LC+1
    done
	PRIPRES=`echo "$NEWPRES"`
fi

#FILENAMErn2=`echo $1|cut -d "/" -f8`
#RAWDUMP=`$DICOMDIR/dcmdump $1|sed '/(/G'|/usr/bin/sed -n 's/    (0008,0050)/ /'`
#RAWDUMP=`$DICOMDIR/dcmdump $1|sed '/(/G'`
#Building dcmdump command line to pull only the tags we will need
INTLC=1
for i in $TAGIDS
do
	LOOPTEMP=`echo "$i"|cut -d "(" -f2|cut -d ")" -f1`
	if [ $INTLC -gt 1 ]
	then
		CMDLINE=`echo "$CMDLINE +P $LOOPTEMP"`
	else
		#We will always need the 2 digit modality code so we will put that in front
		CMDLINE="+P 0008,0060 +P $LOOPTEMP"
	fi
	let INTLC=$INTLC+1
done
CMDLINE=`echo "$CMDLINE +p"`

RAWDUMP=`$DICOMDIR/dcmdump $CMDLINE $PRIPROC/$DIRNAME/$3|sed '/(/G'`

#Getting the modailty for a new IMPAX requirement
MODALITY=`echo "$RAWDUMP"|grep "(0008,0060)"|cut -d "[" -f2|cut -d "]" -f1|head -1`
LC=0
for i in $TAGIDS
do
	let LLC=$LC+1
	if [ `echo "$RAWDUMP"|grep -c "$i"` -eq 0 ]
	then
		INTINSERT=`echo "$PRIINST"|cut -d ":" -f$LLC`
		if [ $DEBUG -gt 0 ]
		then
			echo "`date` Tag $i doesn't exist for $PRIPROC/$DIRNAME/$3.  Skipping..." >> $PRILOGDIR/$PRILFPROC 2>&1
		fi
		if [ "$INTINSERT" == "1" ]
		then
			TAG[$LC]=""
			MOD[$LC]="-nrc -i"
			PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
			TAGID[$LC]=$i
		else	
			MOD[$LC]="NA"
			STRTEMP=""
		fi
		let LC=$LC+1
	elif [ `echo "$RAWDUMP"|grep -c "$i"` -gt 1 ]
	then
		RAWDUMP=`echo "$RAWDUMP"|grep -v " $PRIPROC/$DIRNAME/$3"`
		if [ `echo "$RAWDUMP"|grep -c "$i"` -gt 1 ]
		then
			#If there are multiple entries for a tag, lets see if the values are the same.  If so, change anyway
			INTDUP=`echo "$RAWDUMP"|grep -c "$i"`
			INTERROR=0
			INTMLINE=1
			LC2=0
			while [ $LC2 -le $INTDUP ]
			do
				let LC2=$LC2+1
				if [ $LC2 -eq 1 ]
				then
					STRTEMP=`echo "$RAWDUMP"|grep "$i"|head -1`
					STRVAL=`echo "$STRTEMP"|cut -d "[" -f2|cut -d "]" -f1`
				elif [ $LC2 -lt $INTDUP ]
				then
					STRTEMP=`echo "$RAWDUMP"|grep "$i"|head -2|tail -1`
					STRVAL2=`echo "$STRTEMP"|cut -d "[" -f2|cut -d "]" -f1`
					if [ "$STRVAL" != "$STRVAL2" ]
					then
						INTERROR=1
					fi
				else
					STRTEMP=`echo "$RAWDUMP"|grep "$i"|tail -1`
					STRVAL2=`echo "$STRTEMP"|cut -d "[" -f2|cut -d "]" -f1`
                    if [ "$STRVAL" != "$STRVAL2" ]
                    then
                        INTERROR=1
                    fi
				fi
			done
			INTMODALL=`echo "$PRIMODS"|cut -d ":" -f$LLC`
			if [ $INTERROR -eq 1 ] && [ $INTMODALL -ne 1 ]
			then
				echo "`date`  ERROR:  Tag $i in file $3 has `echo "$RAWDUMP"|grep -c "$i"` entries that are not identical.  Will not process or send.  Here is a list of the tags in question:\n  `echo "$RAWDUMP"|grep "$i"`" >> $PRILOGDIR/$PRILFPROC 2>&1
				echo "`date`  ERROR:  Tag $i in file $3 has `echo "$RAWDUMP"|grep -c "$i"` entries that are not identical.  Will not process or send.  Here is a list of the tags in question:\n  `echo "$RAWDUMP"|grep "$i"`" >> $DIRNAME/error.txt 2>&1
				exit 1
			elif [ $INTERROR -eq 1 ] && [ $INTMODALL -eq 1 ]
			then
				echo "`date`  WARNING:  Tag $i in file $3 has `echo "$RAWDUMP"|grep -c "$i"` entries that are not identical, however, the modall flag is set.  If this is not the desired behavior, please change it..." >> $PRILOGDIR/$PRILFPROC 2>&1
				ISBRACKET=`echo "$RAWDUMP"|grep "$i"|head -1|awk -F "[" '{print NF-1}'`
				if [ $ISBRACKET -lt 1 ]
				then
					TAG[$LC]=`echo "$RAWDUMP"|grep "$i"|head -1|tr -s " "|cut -d " " -f3`
					MOD[$LC]="NA"
					PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
				else
					TAG[$LC]=`echo "$RAWDUMP"|grep "$i"|head -1|cut -d "[" -f2|cut -d "]" -f1`
					MOD[$LC]="-ma"
					PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
				fi
			else
				#echo "`date` Found multiple duplicate tages for $i in file $1.  Prefixing them all..." >> $PRILOGDIR/$PRILFPROC 2>&1
				ISBRACKET=`echo "$RAWDUMP"|grep "$i"|head -1|awk -F "[" '{print NF-1}'`
				if [ $ISBRACKET -lt 1 ]
				then
					TAG[$LC]=`echo "$RAWDUMP"|grep "$i"|head -1|tr -s " "|cut -d " " -f3`
					MOD[$LC]="NA"
					PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
				else
					TAG[$LC]=`echo "$RAWDUMP"|grep "$i"|head -1|cut -d "[" -f2|cut -d "]" -f1`
					MOD[$LC]="-ma"
					PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
				fi
			fi
		else
			echo "`date` Tag $i in file $3 had multiple values.  Only using the non-nested value." >> $PRILOGDIR/$PRILFPROC 2>&1
			ISBRACKET=`echo "$RAWDUMP"|grep "$i"|head -1|awk -F "[" '{print NF-1}'`
			if [ $ISBRACKET -lt 1 ]
			then
				TAG[$LC]=`echo "$RAWDUMP"|grep "$i"|tr -s " "|cut -d " " -f3`
				MOD[$LC]="NA"
				PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
			else
				TAG[$LC]=`echo "$RAWDUMP"|grep "$i"|cut -d "[" -f2|cut -d "]" -f1`
				MOD[$LC]="-m"
				PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
			fi
		fi
		INTMLINE=0
		TAGID[$LC]=$i
		let LC=$LC+1
	else
		STAGID=`echo $i|cut -d "(" -f2|cut -d ")" -f1`
		#TEMPTAG=`$DICOMDIR/dcmdump +P $STAGID +p $PRIPROC/$DIRNAME/$3|sed '/(/G'|cut -d " " -f1`
		TEMPTAG=`echo "$RAWDUMP"|grep "$STAGID"|sed '/(/G'|cut -d " " -f1`
		LC3=0
		NUMSUB=`echo "$TEMPTAG"|awk -F "." '{print NF-1}'`
		while [ $LC3 -le $NUMSUB ]
		do
			let INTTEMP=$LC3+1
			STRTEMP[$LC3]=`echo $TEMPTAG|cut -d "." -f$INTTEMP`
			let LC3=$LC3+1
		done
		LC3=0
		while [ $LC3 -le $NUMSUB ]
		do
			let INTTEMP=$LC3+1
			if [ $LC3 -eq $NUMSUB ]
			then
				TAGID[$LC]=`echo "${TAGID[$LC]}${STRTEMP[$LC3]}"`
			else
				TAGID[$LC]=`echo "${TAGID[$LC]}${STRTEMP[$LC3]}[$LC3]."`
			fi
			let LC3=$LC3+1
		done
		#TAGID[$LC]=`echo ${TAGID[$LC]}|sed 's/\(.$\)//g'`
		ISBRACKET=`echo "$RAWDUMP"|grep "$i"|head -1|awk -F "[" '{print NF-1}'`
		if [ $ISBRACKET -lt 1 ]
		then
			#If the tag exisists but has no value specified
			#TAG[$LC]=`echo "$RAWDUMP"|grep "$i"|cut -d " " -f3`
			TAG[$LC]=""
			MOD[$LC]="-m"
			PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
		else
			TAG[$LC]=`echo "$RAWDUMP"|grep "$i"|cut -d "[" -f2|cut -d "]" -f1`
			MOD[$LC]="-m"
			PRE[$LC]=`echo "$PRIPRES"|cut -d ":" -f$LLC`
		fi
        let LC=$LC+1
	fi
done

LC2=0
while [ $LC2 -lt $LC ]
do
	if [ "${MOD[$LC2]}" != "NA" ]
	then
		NUMMATCH=`echo "${TAG[$LC2]}"|grep "~"|wc -l`
		if [ $NUMMATCH -gt 0 ]
		then
			LC97=32
			while [ $LC97 -lt 127 ]
			do
				NUMMATCH=`ECHO "${TAG[$LC2]}"|grep ""|wc -l`
				let LC97=$LC97+1
			done
		fi
		#Added 20150803
		#Need to escape backslashes
		if [[ "${TAG[$LC2]}" == *\\* ]]
		then
			TAG[$LC2]=`echo "${TAG[$LC2]}"|sed -e 's~\\\~\\\\\\\~g'`
			echo "${TAG[$LC2]} escaped" >> /tmp/$FILENAMErn2.test 
		fi
		if [ "${TAG[$LC2]}" == "" ] && [ "${TAGID[$LC2]}" == "(0031,1020)" ]
		then
			NEWTAG[$LC2]=""
		else
			NEWTAG[$LC2]=`echo "${PRE[$LC2]}"|sed -e "s~@t@~${TAG[$LC2]}~g"`
			NEWTAG[$LC2]=`echo "${NEWTAG[$LC2]}"|sed -e "s~@m@~$MODALITY~g"`
		fi
		MODIFYSTR="$MODIFYSTR${MOD[$LC2]} \"${TAGID[$LC2]}=${NEWTAG[$LC2]}\" "
	fi
	let LC2=$LC2+1
done

if [ $DEBUG -eq 1 ]
then
	echo "First rename" >> $PRILOGDIR/$PRILFPROC 2>&1
    LC2=0
    while [ $LC2 -lt $LC ]
    do
        echo "TAGID[$LC2] = ${NEWTAG[$LC2]}" >> $PRILOGDIR/$PRILFPROC 2>&1
        let LC2=$LC2+1
    done
fi

#$DICOMDIR/dcmodify `echo $MODIFYSTR$PRIPROC/$DIRNAME/$3|tr "\'" " "` >> $PRILOGDIR/$PRILFPROC 2>&1
echo "$DICOMDIR/dcmodify $MODIFYSTR $PRIPROC/$DIRNAME/$3" > /tmp/$FILENAMErn2.dcmod
chmod 755 /tmp/$FILENAMErn2.dcmod
/tmp/$FILENAMErn2.dcmod
rm -f /tmp/$FILENAMErn2.dcmod
#It could be the 2nd dcmdump is executing on the original file before dcmodify has a change to swap it
#This should fix that
sync

#This is an error checking section.  For each field that is modified above, there should be a coresponding check below...
#RAWDUMP=`$DICOMDIR/dcmdump $PRIPROC/$DIRNAME/$3|sed '/(/G'`
RAWDUMP=`$DICOMDIR/dcmdump $CMDLINE $PRIPROC/$DIRNAME/$3|sed '/(/G'`
LC2=0
NUMTAGS=${#TAG[*]}
while [ $LC2 -lt $NUMTAGS ]
do
	if [ "${MOD[$LC2]}" != "NA" ]
    then
		ISPATH=`echo ${TAGID[$LC2]}|awk -F "." '{print NF}'`
		if [ $ISPATH -gt 1 ]
		then
			TAGTEMP=`echo ${TAGID[$LC2]}|cut -d "." -f$ISPATH`
		else
			TAGTEMP=${TAGID[$LC2]}
		fi
		TAGTEMP=`echo "$TAGTEMP"|cut -d "(" -f2|cut -d ")" -f1`
		if [ "${MOD[$LC2]}" == "-nrc -i" ]
		then
			MODTAG[$LC2]=`echo "${NEWTAG[$LC2]}"`
		else
			ISBRACKET=`echo "$RAWDUMP"|grep "$TAGTEMP"|head -1|awk -F "[" '{print NF-1}'`
			if [ $ISBRACKET -lt 1 ]
			then
				MODTAG[$LC2]=""
			else
				MODTAG[$LC2]=`echo "$RAWDUMP"|grep "$TAGTEMP"|head -1|cut -d "[" -f2|cut -d "]" -f1`
			fi
		fi
		if [ "${MODTAG[$LC2]}" != "${NEWTAG[$LC2]}" ] && [ "${MODTAG[$LC2]}" != " " ]
		then
			echo "`date` Error:  Modification failed!  Please check old value: ${MODTAG[$LC2]} new value: ${NEWTAG[$LC2]} for tag ${TAGID[$LC2]} in file $3" >> $PRILOGDIR/$PRILFPROC 2>&1
			echo "`date` Error:  Modification failed!  Please check old value: ${MODTAG[$LC2]} new value: ${NEWTAG[$LC2]} for tag ${TAGID[$LC2]} in file $3" >> $PRILOGDIR/rawdump.txt 2>&1
			echo "`date` TAGTEMP = $TAGTEMP and the value searched for is " >> $PRILOGDIR/rawdump.txt 2>&1
			echo "$RAWDUMP"|grep "$TAGTEMP"|head -1 >> $PRILOGDIR/rawdump.txt 2>&1
			echo "RAWDUMP:  $RAWDUMP" >> $PRILOGDIR/rawdump.txt 2>&1
			echo "dcmodify line: " >> $PRILOGDIR/rawdump.txt 2>&1
			echo "$DICOMDIR/dcmodify `echo $MODIFYSTR$PRIPROC/$DIRNAME/$3|tr "\'" " "`" >> $PRILOGDIR/rawdump.txt 2>&1
			touch $PRIPROC/$DIRNAME/error.txt
			echo >> $PRILOGDIR/rawdump.txt 2>&1
			echo >> $PRILOGDIR/rawdump.txt 2>&1
			exit 1
		else
			STROUTPUT=" $STROUTPUT ${TAGID[$LC2]} of ${NEWTAG[$LC2]}"
		fi
	fi
	let LC2=$LC2+1
done

if [ "$PRILL" == "debug" ] || [ "$PRILL" == "trace" ]
then
	echo "`date` Processed $FILENAMErn2 with$STROUTPUT" >> $PRILOGDIR/$PRILFPROC 2>&1
fi
exit 0
