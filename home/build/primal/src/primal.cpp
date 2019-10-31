//============================================================================
// Name        : primal.cpp
// Author      : Willis Yonker
// Version     :
// Copyright   : Nothing yet
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <sys/stat.h>
#include <glob.h>
#include <string>

using namespace std;

class ConfFile
	{
		public:
		//This should be an unordered_map but Eclipse won't let me...
		//std::unordered_map<std::string,std::string> primConf;
		std::map<std::string,std::string> primConf;

		ConfFile(std::string strRec);
		int ValidateConf(void);

	};

	ConfFile::ConfFile(std::string strRec)
	{
	    std::string strLine, strKey, strValue;
	    std::size_t intPOS, intStartPOS, intStartConf=0, intEndConf=0;
	    std::ifstream infile("/etc/primal/primal.conf", std::ifstream::in);

	    while(!infile.eof() && intEndConf != 1)
	    {
	        getline(infile,strLine); // Saves the line in STRING.
	        if(intStartConf == 0)
	        {
	        	intPOS=strLine.find("<scp"+strRec);
	        	if(intPOS!=std::string::npos)
	        		intStartConf=1;
	        } else {
	        	intPOS=strLine.find("</scp"+strRec);
	        	if(intPOS!=std::string::npos)
	        	{
	        		intEndConf=1;
	        	} else {
	        		intPOS=strLine.find("#");
	        		if(intPOS!=std::string::npos)
	        			strLine=strLine.substr(0, intPOS);
	        		intPOS=strLine.find("=");
	        		if(intPOS!=std::string::npos)
	        		{
	        			strKey=strLine.substr(0,intPOS);
	        			strValue=strLine.substr(intPOS+1);
	        			//Remove leading space and tabs
	        			intStartPOS = strKey.find_first_not_of(" \t");
	        			if( string::npos != intStartPOS )
	        			    strKey = strKey.substr( intStartPOS );
	        			intStartPOS = strKey.find_last_not_of(" \t");
	        			if( string::npos != intStartPOS )
	        			    strKey = strKey.substr(0, intStartPOS + 1 );
	        			intStartPOS = strValue.find_first_not_of(" \t");
	        			if( string::npos != intStartPOS )
	        			    strValue = strValue.substr( intStartPOS );
	        			intStartPOS = strValue.find_last_not_of(" \t");
	        			if( string::npos != intStartPOS )
	        			    strValue = strValue.substr(0, intStartPOS + 1 );
	        			primConf[strKey]=strValue;
	        		}
	        	}
	        }
	    }
	    infile.close();
	};

	int ConfFile::ValidateConf(void)
	{
		struct stat sb;

		//PRIPORT
		int intValue = atoi(this->primConf["PRIPORT"].c_str());
		if (intValue < 100 || intValue > 10000)
		{
			cout << "Error:  PRIPORT is : " << intValue << " and must be a number and must be between 100 and 10,000." << endl;
			return 1;
		}
		//PRIDESTPORT0
		intValue = atoi(this->primConf["PRIDESTPORT0"].c_str());
		if (intValue < 100 || intValue > 10000)
		{
			cout << "Error:  PRIDESTPORT0 is : " << intValue << " and must be a number and must be between 100 and 10,000." << endl;
			return 1;
		}
		//PRILL
		std::transform(this->primConf["PRILL"].begin(), this->primConf["PRILL"].end(), this->primConf["PRILL"].begin(), ::tolower);
		if (this->primConf["PRILL"] != "fatal" && this->primConf["PRILL"] != "error" && this->primConf["PRILL"] != "warn" && this->primConf["PRILL"] != "info" && this->primConf["PRILL"] != "debug" && this->primConf["PRILL"] != "trace")
		{
			cout << "Error:  PRILL is : " << this->primConf["PRILL"] << " and must be one of the following:  fatal, error, warn, info, debug or trace..." << endl;
			return 1;
		}
		//PRIIF - Location receiver will store incoming files
		if (stat(this->primConf["PRIIF"].c_str(), &sb) == -1)
		{
			cout << "Error:  PRIIF = " << this->primConf["PRIIF"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
	        return 1;
	    }
		//PRIPROC - Location to put series that are being processed.
		if (stat(this->primConf["PRIPROC"].c_str(), &sb) == -1)
		{
			cout << "Error:  PRIPROC = " << this->primConf["PRIPROC"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
	        return 1;
	    }
		//PRIOUT - Location to put series that are to be sent
		if (stat(this->primConf["PRIOUT"].c_str(), &sb) == -1)
		{
			cout << "Error:  PRIOUT = " << this->primConf["PRIOUT"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
	        return 1;
	    }
		//PRISENT - Location to put studies that have been sent successfully
		if (stat(this->primConf["PRISENT"].c_str(), &sb) == -1)
		{
			cout << "Error:  PRISENT = " << this->primConf["PRISENT"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
	        return 1;
	    }
		//PRILOGDIR - Location to put studies that have been sent successfully
		if (stat(this->primConf["PRILOGDIR"].c_str(), &sb) == -1)
		{
			cout << "Error:  PRILOGDIR = " << this->primConf["PRILOGDIR"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
	        return 1;
	    }
		//PRIERROR - Location to put studies that have been sent successfully
		if (stat(this->primConf["PRIERROR"].c_str(), &sb) == -1)
		{
			cout << "Error:  PRIERROR = " << this->primConf["PRIERROR"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
	        return 1;
	    }
		//PRIRET - Days to keep series that were successfully sent.  0 = forever
		intValue = atoi(this->primConf["PRIRET"].c_str());
		if (intValue < 0 || intValue > 365)
		{
			cout << "Error:  PRIRET is : " << intValue << " and must be a number between 0 and 365." << endl;
			return 1;
		}
		//PRICL - Days to keep series that were successfully sent.  0 = forever
		intValue = atoi(this->primConf["PRICL"].c_str());
		if (intValue < 0 || intValue > 9)
		{
			cout << "Error:  PRICL is : " << intValue << " and must be a number between 0 and 9." << endl;
			return 1;
		}
		//PRIDESTHIP0
		if (this->primConf["PRIDESTHIP0"].empty())
		{
			cout << "Error:  PRIDESTHIP0 must be defined as the Hostname or IP of the first destination." << endl;
			return 1;
		}
		//Need to add checking for additional HIPx
		return 0;
	}

std::string GetDate() {
	time_t t = time(0);
	struct tm * now = localtime( & t );
	std::string strDate;

	strDate = to_string(now->tm_year + 1900);
	if ((now->tm_mon + 1) < 10) {
		strDate += "0";
		strDate += to_string(now->tm_mon + 1);
	} else {
		strDate += to_string(now->tm_mon + 1);
	}
	if ((now->tm_mday) < 10) {
		strDate += "0";
		strDate += to_string(now->tm_mday);
	} else {
		strDate += to_string(now->tm_mday);
	}
	strDate += ":";
	if (now->tm_hour < 10) {
		strDate += "0";
		strDate += to_string(now->tm_hour);
	} else {
		strDate += to_string(now->tm_hour);
	}
	if (now->tm_min < 10) {
		strDate += "0";
		strDate += to_string(now->tm_min);
	} else {
		strDate += to_string(now->tm_min);
	}
	if (now->tm_sec < 10) {
		strDate += "0";
		strDate += to_string(now->tm_sec);
	} else {
		strDate += to_string(now->tm_sec);
	}
	return strDate;
}

int StartSend(std::string strPath) {
	size_t intSlash = strPath.find_last_of("/") + 1;
	std::string strDirName;
	glob_t gl;
	size_t num = 0;

	if(intSlash > 1) {
		strDirName=strPath.substr(intSlash, string::npos);
	} else {
		strDirName=strPath;
	}

	strPath=strPath+"/*";

	if(glob(strPath.c_str(), GLOB_NOSORT, NULL, &gl) == 0)
	  num = gl.gl_pathc;
	globfree(&gl);
	cout << "Number of files: " << num << endl;

	std::string strDate=GetDate();
	cout << strDate << " Directory name is: " << strDirName << " and path is " << strPath << endl;

	return 1;
	/*TEMPVAR=`echo $2|tr "/" "\n"|wc -l`
	DIRNAME=`echo $2|cut -d "/" -f $TEMPVAR`

	RECNUM=`echo "$DIRNAME"|cut -d "_" -f1`

	source readconf.bash

	STARTPROCTIME=`date +%s`
	STRTEMP1="`cat /tmp/$DIRNAME` $STARTSENDTIME"
	echo "$STRTEMP1" > /tmp/$DIRNAME
	FILENAME=`ls -1 $PRIOUT/$DIRNAME/*|head -1`
	SERIESINFO=`dcmdump $FILENAME|egrep '(0010,0010)|(0010,0020)'`
	PNAME=`echo "$SERIESINFO"|grep "(0010,0010)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
	PAPID=`echo "$SERIESINFO"|grep "(0010,0020)"|head -1|cut -d "[" -f2|cut -d "]" -f1`
	NUMFILES=`ls -1 $PRIOUT/$DIRNAME/*|wc -l`
	echo "`date` Started sending study for Patient: $PNAME MRN: $PAPID with $NUMFILES images." >> $PRILOGDIR/$PRILFOUT

	NUMDEST=0
	while [ $NUMDEST -lt $INTNUMREC ]
	do
	    /home/dicom/bin/storescu -xs -ll $PRILL -aet $PRIAET -aec ${PRIDESTAEC[$NUMDEST]} ${PRIDESTHIP[$NUMDEST]} ${PRIDESTPORT[$NUMDEST]} $PRIOUT/$DIRNAME/* >> $PRILOGDIR/$PRILFOUT 2>&1
	    let NUMDEST=$NUMDEST+1
	done
	mv $PRIOUT/$DIRNAME $PRISENT >> $PRILOGDIR/$PRILFOUT 2>&1

	ENDPROCTIME=`date +%s`
	PROCTIME=`echo "$ENDPROCTIME-$STARTPROCTIME"|bc 2>/dev/null`
	STARTRECTIME=`cat /tmp/$DIRNAME|head -1|cut -d " " -f1`
	TOTALTIME=`echo "$ENDPROCTIME-$STARTRECTIME"|bc 2>/dev/null`
	IMGSEC=`echo "scale=2; $NUMFILES/$PROCTIME"|bc 2>/dev/null`
	TOTALIMGSEC=`echo "scale=2; $NUMFILES/$TOTALTIME"|bc 2>/dev/null`
	echo "`date` Finished sending study for Patient: $PNAME MRN: $PAPID with $NUMFILES images in $PROCTIME seconds @ $IMGSEC image(s)/sec.  Total time on PRIMAL = $TOTALTIME seconds @ $TOTALIMGSEC image(s)/sec." >> $PRILOGDIR/$PRILFOUT

	rm -f /tmp/$DIRNAME
	*/
}

int main(int argc, char *argv[]) {
	std::size_t intReturn;

	std::string strSCP = argv[1];
	std::string strDirPath = argv[2];
	ConfFile conf1(strSCP);
	intReturn=conf1.ValidateConf();
	if (intReturn == 1)
		return 1;

	intReturn=StartSend(strDirPath);

	char chrTest[4];
	strcpy(chrTest, "-xs");
	argv[1]=chrTest;

	char chrTest2[4];
	strcpy(chrTest2, "-ll");
	argv[2]=chrTest2;

	char chrTest3[conf1.primConf["PRILL"].length()];
	strcpy(chrTest3, conf1.primConf["PRILL"].c_str());
	argv[3]=chrTest3;

	strcpy(argv[4], "-aet");
	strcpy(argv[5], conf1.primConf["PRIAET"].c_str());
	strcpy(argv[6], "aec");
	strcpy(argv[7], conf1.primConf["PRIDESTAEC0"].c_str());
	strcpy(argv[8], conf1.primConf["PRIDESTHIP0"].c_str());
	strcpy(argv[9], conf1.primConf["PRIDESTPORT0"].c_str());
	strcpy(argv[10], strDirPath.c_str());
	argc = 11;

    /*for( map<string, string>::iterator ii=conf1.primConf.begin(); ii!=conf1.primConf.end(); ++ii)
    {
    	cout << (*ii).first << " : " << (*ii).second << endl;
    }*/

	return 0;
}
