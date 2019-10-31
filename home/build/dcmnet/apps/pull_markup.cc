#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstdio>
#include <ctime>
#include <map>
#include <algorithm>
#include <vector>
#include <functional>
#include <unistd.h>
#include <pstreams/pstream.h>
#include <sys/stat.h>
#include <mysql/my_global.h>
#include <mysql/mysql.h>

using namespace std;
MYSQL *mconnect;

std::string Get_Date_Time() {
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	std::string strTime;

	strTime+=std::to_string(now->tm_year + 1900);
    strTime.append("-");
    strTime+=std::to_string(now->tm_mon + 1);
    strTime.append("-");
    strTime+=std::to_string(now->tm_mday);
    strTime.append(" ");
    strTime+=std::to_string(now->tm_hour);
    strTime.append(":");
    strTime+=std::to_string(now->tm_min);
    strTime.append(":");
    strTime+=std::to_string(now->tm_sec);
    return(strTime);
}

int main(int argc, char* argv[])
{
	std::size_t intPOS, intNumRows, intColSCP, intColStatus, intNumFields, intFoundSerIUID, intStartPOS, intEndPOS, intLength, intLC1=0;
	std::size_t intNumResults, intFoundModality, intFoundSeriesDesc, intDone=0;
	std::string strReturn, strQuery, strCommand, strDateTime, strStatus, strStartDate, strEndDate, strQuery2, strCMD, strSerIUID;
	std::string strReadLine, strReadLine2, strCMD2, strModality, strSeriesDesc;
	FILE *fp;
	char buffer[256];
    time_t t = time(0);   // get time now.  Seconds since epoch
	struct tm * now = localtime( & t );

	if (argc < 7) {
		std::cerr << "Usage: " << argv[0] << " <return receiver> <AET> <Return IP> <Return Port> <AET of destination> <Host name or IP> <port> <local IP address>" << std::endl;
		return 1;
	}

    std::tm tmNow = {};
    std::tm tmLastWeek = {};

	tmNow.tm_year = now->tm_year;
	tmNow.tm_mon = now->tm_mon;
	tmNow.tm_mday = now->tm_mday;
	mktime(&tmNow);

	tmLastWeek.tm_year = now->tm_year;
	tmLastWeek.tm_mon = now->tm_mon;
	tmLastWeek.tm_mday = now->tm_mday;
	tmLastWeek.tm_mday -= 7;
	mktime(&tmLastWeek);


	strEndDate=std::to_string(tmNow.tm_year+1900);
    strEndDate.append("-");
	if(tmNow.tm_mon + 1 < 10) {
		strEndDate.append("0");
	}
    strEndDate+=std::to_string(tmNow.tm_mon + 1);
    strEndDate.append("-");
	if(tmNow.tm_mday < 10) {
		strEndDate.append("0");
	}
    strEndDate+=std::to_string(tmNow.tm_mday);
    strEndDate.append(" ");
    strEndDate.append("18:00:00");

	strStartDate=std::to_string(tmLastWeek.tm_year+1900);
    strStartDate.append("-");
	if(tmLastWeek.tm_mon + 1 < 10) {
		strStartDate.append("0");
	}
    strStartDate+=std::to_string(tmLastWeek.tm_mon + 1);
    strStartDate.append("-");
	if(tmLastWeek.tm_mday < 10) {
		strStartDate.append("0");
	}
    strStartDate+=std::to_string(tmLastWeek.tm_mday);
    strStartDate.append(" ");
    strStartDate.append("18:00:00");

	cout << "Start time: " << strStartDate << endl;
	cout << "End time: " << strEndDate << endl;

	mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
	if (!mconnect) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
    if (!mconnect) {
        cout << "connection failed" << std::endl;
        return 1;
    }

	//Probably should switch this to pull from send and check for error to limit what is in the list to successfully routed studies
	strQuery="select s.SIUID from receive as r left join study as s on r.puid=s.puid where r.tstartrec between '" + strStartDate + "' and '" + strEndDate + "';";
	mysql_query(mconnect, strQuery.c_str());
	MYSQL_RES *result = mysql_store_result(mconnect);
	if (result == NULL)
	{
		cout << "No studies found for the past week." << std::endl;
		return 0;
	}
	intNumFields = mysql_num_fields(result);
	intNumResults = mysql_num_rows(result);
	cout << "Number of results: " << intNumResults << std::endl;
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		//We have a study that has gone through.  Now we need to query the destination to see if there are any new series
		strCMD="/home/dicom/bin/findscu -ll debug -to 60 -ta 60 -td 60 -aet " + string(argv[2]) + " -S -k 0008,0052=SERIES -k \"(0020,000d)=" + string(row[0]) + "\" -k 0008,0020 -k 0008,0060 -k 0008,103e -k 0020,0013 -k 0008,0018 -k 0020,000e -aec " + string(argv[5]) + " " + string(argv[6]) + " " + argv[7];
		//std::cout << "I:  " << strCMD << std::endl;
		redi::ipstream proc(strCMD, redi::pstreams::pstderr);
		while (std::getline(proc.err(), strReadLine)) {
			intFoundModality=strReadLine.find("W: (0008,0060) CS");
			if (intFoundModality!=std::string::npos) {
				intStartPOS=strReadLine.find("[");
				intStartPOS++;
                if (intStartPOS!=std::string::npos) {
					intEndPOS=strReadLine.find("]");
					if (intEndPOS!=std::string::npos) {
                        intLength = intEndPOS-intStartPOS;
                        strModality=strReadLine.substr(intStartPOS, intLength);
					} else {
						strModality="";
					}
				} else {
					strModality="";
				}
			}
			intFoundSeriesDesc=strReadLine.find("W: (0008,103e)");
			if (intFoundSeriesDesc!=std::string::npos) {
				intStartPOS=strReadLine.find("[");
				intStartPOS++;
                if (intStartPOS!=std::string::npos) {
					intEndPOS=strReadLine.find("]");
					if (intEndPOS!=std::string::npos) {
                        intLength = intEndPOS-intStartPOS;
                        strSeriesDesc=strReadLine.substr(intStartPOS, intLength);
					} else {
						strSeriesDesc="";
					}
				} else {
					strSeriesDesc="";
				}
			}
			intFoundSerIUID=strReadLine.find("W: (0020,000e) UI");
			if (intFoundSerIUID!=std::string::npos) {
				intStartPOS=strReadLine.find("[");
				intStartPOS++;
				if (intStartPOS!=std::string::npos) {
					intEndPOS=strReadLine.find("]");
					if (intEndPOS!=std::string::npos) {
						intLength = intEndPOS-intStartPOS;
						strSerIUID=strReadLine.substr(intStartPOS, intLength);
						cout << "I:  Checking if " << strSerIUID << " is in the database." << std::endl;
						strQuery2="select PUID from series where SERIUID = '" + strSerIUID + "';";
						//cout << "Query: " << strQuery2 << std::endl;
						mysql_query(mconnect, strQuery2.c_str());
						MYSQL_RES *result2 = mysql_store_result(mconnect);
						intNumResults = mysql_num_rows(result2);
						if(intNumResults < 1) {
							if (strModality.compare("CC") != 0) {
								if (strSeriesDesc.compare("iCAD CAD") != 0) {
									if (intDone != 1) {
										cout << "strModality = " << strModality << " and SeriesDesc = " << strSeriesDesc << "." << std::endl;
										//Haven't seen this guy before.  Better retrieve it
										strCMD2="/home/dcm4che/bin/movescu -b " + string(argv[2]) + "@" + string(argv[8]) + ":" + string(argv[4]) + " -c " + string(argv[5]) + "@" + argv[6] + ":" + argv[7] + " -m StudyInstanceUID=\"" + row[0] + "\" --dest " + string(argv[2]);
										std::cout << "I:  " << strCMD2 << std::endl;
										redi::ipstream proc2(strCMD2, redi::pstreams::pstderr);
										while (std::getline(proc2.err(), strReadLine2))
											std::cerr << strReadLine2 << std::endl;
										cout << "Study Instance: " << row[0] << " has changed.  Retrieving..." << std::endl;
										intDone=1;
									} else
										cout << "I:  Requested study " << row[0] << " already.  Not requesting again..." << std::endl;
								} else
									cout << "I:  SeriseInstanceUID " << strSerIUID << " is an iCAD series.  Ignoring..." << std::endl;
							} else
								cout << "I:  SeriseInstanceUID " << strSerIUID << " is a CC object.  Ignoring..." << std::endl;
						} else
							std::cout << "I:  Have seen " << strSerIUID << " for study " << row[0] << " before.  Will not retrieve study." << std::endl;
						mysql_free_result(result2);
					} else
						std::cerr << "W:  Found a SeriesInstanceUID line but could not find an ending ].  Malformed response detected!" << std::endl;
				} else
					std::cerr << "W:  Found a SeriesInstanceUID line but could not find an opening [.  Malformed response detected!" << std::endl;
			}
		}
		intDone=0;
	}
	mysql_free_result(result);
	mysql_close(mconnect);
	return 0;
}
