/*
    This file is part of PRIMAL.

    PRIMAL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PRIMAL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PRIMAL.  If not, see <https://www.gnu.org/licenses/>.
*/
#undef min
#undef max
#include <algorithm>
#include <map>
#include <glob.h>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <pstreams/pstream.h>
//#include <mysql/my_global.h>
#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <openssl/md5.h>
#include <sys/stat.h>

using namespace std;
namespace fs = std::filesystem;
std::vector<std::string > vecRCcon1;
std::vector<std::string > vecRCopt1;
std::vector<std::string > vecRCcon2;
std::vector<std::string > vecRCact1;
std::list<std::string> lstStudies;
std::mutex mtx;

const std::string strProcChainType = "PRIMRCRECV";

#include "prim_functions.h"

MYSQL *mconnect;
MYSQL_ROW row;
MYSQL_RES *result;

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

template <typename Out>
void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

std::size_t fEndReceive(std::string strMessage) {
    std::string strLogMessage, strTemp2, strFilename, strTemp3, strRawDCMdump, strSerIUID, strSerDesc, strModality, strSopIUID;
    std::string strStudyDateTime, strPrimalID, strQuery, strRecNum, strDBREturn, strAEC, strClientID2, strClientID, strClientAET;
    std::string strClientName, strPrefetchNode, strCmd, strResult, strTemp, strFullPath, strTempPath;
    std::size_t intLC2, intTemp, intPos, intImgNum, intFound;
    std::size_t intDone2, intLC;
    std::vector<std::string> vMessage;
    struct stat st;
    struct PatientData pData2;
    time_t t2 = time(0);   // get time now
    struct tm * now2 = localtime( & t2 );
    
    //This message type is to start the receive process and create the JSON file 
    //Need to scrape the DICOM headers.
    //vMessage = split(strMessage, ' ');
    //if(vMessage.size() > 3) {
    //    pData2.callingAETitle=vMessage[2];
    //    pData2.calledAETitle=vMessage[3];
    //}
    //intMessageType=stoi(vMessage[1]);
    //See if the directory exists
    intLC2=0;
    fs::directory_entry d1(strMessage);
    while(! d1.is_directory() && intLC2 < 10) {
        //Let's wait up to 10 seconds for it to appear.
        intLC2++;
        std::this_thread::sleep_for (std::chrono::seconds(1));
        d1.refresh();
    }
    if(! d1.is_directory()) {
        //Directory never appeared.  Disgard message
        return -1;
    }
    //Remove the last character if it's a /
    if(strMessage.back() == '/') {
        strMessage.pop_back();
    }
    intFound = strMessage.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strMessage.substr(intFound + 1);
    } else {
        strPrimalID=strMessage;
    }
    //Make sure last character is a /
    if(strMessage.back() != '/') {
        strMessage.push_back('/');
    }

    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);
    //This is the end of receive
    strLogMessage =strPrimalID+" RECV  Ending receive";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    intDone2 = 0;
    intLC = 0;
    while (intDone2 == 0) {
        strTempPath="/tmp/" + strPrimalID;
        if(stat(strTempPath.c_str(), &st) == 0) {
            fs::remove(strTempPath);
            intDone2 = 1;
        } else if(intLC >= 100) {
            intDone2 = 1;
        } else {
            std::this_thread::sleep_for (std::chrono::milliseconds(100));
            intLC++;
        }
    }
    pData2.strEndRec=std::to_string(now2->tm_year + 1900);
    pData2.strEndRec.append("-");
    pData2.strEndRec+=std::to_string(now2->tm_mon + 1);
    pData2.strEndRec.append("-");
    pData2.strEndRec+=std::to_string(now2->tm_mday);
    pData2.strEndRec.append(" ");
    pData2.strEndRec+=std::to_string(now2->tm_hour);
    pData2.strEndRec.append(":");
    pData2.strEndRec+=std::to_string(now2->tm_min);
    pData2.strEndRec.append(":");
    pData2.strEndRec+=std::to_string(now2->tm_sec);
    strLogMessage = "Updating JSON and creating PKG for " + strPrimalID + ".";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    system(strCmd.c_str());
    intImgNum=0;
    for (const auto & entry : fs::directory_iterator(strMessage)) {
        strTemp2=entry.path().string();
        intTemp = strTemp2.find_last_of("/");
        strFilename=strTemp2.substr(intTemp+1);
        intPos=strFilename.find_last_of(".");
        if(intPos != std::string::npos) {
            strTemp3=strFilename.substr(intPos);
        }
        if(strTemp3 == ".dcm") {
            intImgNum++;
        }
    }
    //std::cout << "fEndReceive Found " << to_string(intImgNum) << " files in " << strMessage << std::endl;
    strQuery="update receive set tendrec = '" + pData2.strEndRec + "', rec_images = " + to_string(intImgNum) + " where puid = '" + strPrimalID + "';";
    mysql_query(mconnect, strQuery.c_str());
    if(*mysql_error(mconnect)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect);
        strLogMessage+="\nstrQuery = " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    }
    if(strMessage.back() == '/') {
        strMessage.pop_back();
    }
    if(conf1.primConf[strRecNum + "_PRIJSON"] == "1") {
        strTemp=strMessage + "/payload.json";
        while(stat(strTemp.c_str(),&st) != 0 && intLC < 20) {
            strLogMessage = "Waiting for " + strTemp + " to appear.";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            std::this_thread::sleep_for (std::chrono::seconds(3));
            intLC++;
        }
    }
    strCmd="mv " + strMessage + " " + conf1.primConf[strRecNum + "_PRIPROC"] + "/";
    strLogMessage + "Moving study to " + strCmd;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    system(strCmd.c_str());
    strFullPath=conf1.primConf[strRecNum + "_PRIPROC"] + "/" + strPrimalID;
    //strCmd = "/usr/local/bin/mq send /prim_process \"" + strFullPath + " 2\" &";
    strCmd = strFullPath + " 2";
    fWriteMessage(strCmd, "/prim_process");
    strLogMessage =strPrimalID + " RECV  Passing to the processing process.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    
    //system(strCmd.c_str());
    //strCmd="/home/dicom/process.bash " + strRecNum + " " + conf1.primConf[strRecNum + "_PRIPROC"] + "/" + strPrimalID + " & >> ";
    //strCmd+=conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"] + " 2>&1";
    //system(strCmd.c_str());
    return 0;
}

std::size_t fStartReceive(std::string strMessage) {
    std::string strLogMessage, strTemp2, strFilename, strTemp3, strRawDCMdump, strSerIUID, strSerDesc, strModality, strSopIUID;
    std::string strStudyDateTime, strPrimalID, strQuery, strRecNum, strDBREturn, strAEC, strClientID2, strClientID, strClientAET;
    std::string strClientName, strPrefetchNode, strCmd, strResult, strLine, strTemp;
    std::size_t intLC2, intTemp, intPos, intImgNum, intFound, intNumRows, intDBEntries, intFound2, intFound3, intTimeStamp;
    //std::vector<std::string> vMessage;
    std::list<std::string>::iterator itStudies;
    struct PatientData pData2;
    time_t t2 = time(0);   // get time now
    struct tm * now2 = localtime( & t2 );

    //See if the directory exists
    intLC2=0;
    fs::directory_entry d1(strMessage);
    while(! d1.is_directory() && intLC2 < 10) {
        //Let's wait up to 10 seconds for it to appear.
        intLC2++;
        std::this_thread::sleep_for (std::chrono::seconds(1));
        d1.refresh();
    }
    if(! d1.is_directory()) {
        //Directory never appeared.  Disgard message
        return -1;
    }
    intFound = strMessage.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strMessage.substr(intFound + 1);
    } else {
        strPrimalID=strMessage;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);
    strLogMessage =strPrimalID + " RECV  Starting receive for " + strMessage;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    //strAEC=vMessage[3];
    //Needed to get the client ID
    strClientID2 = "NULL";
    if(fs::exists("/etc/primal/prim_ae_map.conf")) {
        strLogMessage = strPrimalID + " RECV " + "Searching conf file for client ID of " + strAEC + "...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cout << strLogMessage << std::endl;
        std::ifstream fpAEMap("/etc/primal/prim_ae_map.conf");
        while (std::getline(fpAEMap, strLine)) {
            intFound=strLine.find(",");
            intFound2=strLine.find(",", intFound + 1);
            strClientID = strLine.substr(0, intFound);
            strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
            strClientName = strLine.substr(intFound2 + 1);
            intFound2=strClientAET.find("DEFAULT");
            if(intFound2 != std::string::npos) {
                strClientID2 = strClientID;
                strLogMessage=strPrimalID + " RECV  Found DEFAULT client ID of " + strClientID2;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
        }
        fpAEMap.close();
    }
    if(fs::exists("/var/spool/primal_ae_map.txt")) {
        strLogMessage = strPrimalID + " RECV " + "Searching map table for client ID of " + strAEC + "...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::ifstream fpAEMap("/var/spool/primal_ae_map.txt");
        while (std::getline(fpAEMap, strLine)) {
            intFound=strLine.find(",");
            intFound2=strLine.find(",", intFound + 1);
            strClientID = strLine.substr(0, intFound);
            strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
            strClientName = strLine.substr(intFound2 + 1);
            intFound2=strAEC.find(strClientID);
            if(intFound2 != std::string::npos) {
                strClientID2 = strClientID;
                strLogMessage=strPrimalID + "  Found " + strClientID + " client ID is now " + strClientID2;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
        }
        fpAEMap.close();
    }
    if(fs::exists("/etc/primal/prim_ae_map.conf")) {
        std::ifstream fpAEMap("/etc/primal/prim_ae_map.conf");
        while (std::getline(fpAEMap, strLine)) {
            intFound=strLine.find(",");
            intFound2=strLine.find(",", intFound + 1);
            intFound3=strLine.find(",", intFound2 + 1);
            strClientID = strLine.substr(0, intFound);
            strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
            strClientName = strLine.substr(intFound2 + 1, intFound3 - intFound2 -1);
            strPrefetchNode = strLine.substr(intFound3);
            intFound2=strAEC.find(strClientID);
            if(intFound2 != std::string::npos) {
                strClientID2 = strClientID;
                strLogMessage=strPrimalID + "  Found " + strClientID + " client ID is now " + strClientID2;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
        }
        fpAEMap.close();
    }
    //create new timestamp
    intTimeStamp = std::time(0);
    strQuery="select senderAET from receive where puid='" + strPrimalID + "'";
    mysql_query(mconnect, strQuery.c_str());
    if(*mysql_error(mconnect)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect);
        strLogMessage+="\nstrQuery = " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    }
    result = mysql_store_result(mconnect);
    pData2.strStartRec=std::to_string(now2->tm_year + 1900);
    pData2.strStartRec.append("-");
    pData2.strStartRec+=std::to_string(now2->tm_mon + 1);
    pData2.strStartRec.append("-");
    pData2.strStartRec+=std::to_string(now2->tm_mday);
    pData2.strStartRec.append(" ");
    pData2.strStartRec+=std::to_string(now2->tm_hour);
    pData2.strStartRec.append(":");
    pData2.strStartRec+=std::to_string(now2->tm_min);
    pData2.strStartRec.append(":");
    pData2.strStartRec+=std::to_string(now2->tm_sec);
    if(result) {
        intNumRows = mysql_num_rows(result);
        if(intNumRows > 0) {
            mysql_free_result(result);
            strQuery = "update receive set rservername = '" + strHostname + "', tstartrec = '" + pData2.strStartRec + "', ttimestamp = '" + to_string(intTimeStamp) + "', rec_images = '1' where puid = '" + strPrimalID + "';";
            mysql_query(mconnect, strQuery.c_str());
            if(*mysql_error(mconnect)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect);
                strLogMessage+="strQuery = " + strQuery + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
        } else {
            strQuery="insert into receive (puid, rservername, tstartrec, ttimestamp, rec_images, senderAET, callingAET) values ('";
            strQuery+=strPrimalID + "', '" + strHostname + "', '";
            strQuery+=pData2.strStartRec + "', ";
            strQuery+=to_string(intTimeStamp);
            strQuery+=", '1', '" + pData2.calledAETitle + "', '" + pData2.callingAETitle + "');";
            mysql_query(mconnect, strQuery.c_str());
            if(*mysql_error(mconnect)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect);
                strLogMessage+="\nstrQuery = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
        }
    }
    //Need to populate the study and patient tables.
    for (const auto & entry : fs::directory_iterator(strMessage)) {
        strTemp2=entry.path().string();
        intTemp = strTemp2.find_last_of("/");
        strFilename=strTemp2.substr(intTemp+1);
        intImgNum++;
        intPos=strFilename.find_last_of(".");
        if(intPos != std::string::npos) {
            strTemp3=strFilename.substr(intPos);
        }
        if(strTemp3 == ".dcm") {
            strLogMessage = "Getting tags from " + entry.path().string();
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            strRawDCMdump=fDcmDump(entry.path().string());
            pData2.strPName=fGetTagValue("0010,0010", strRawDCMdump, 0, 0);
            pData2.strMRN=fGetTagValue("0010,0020", strRawDCMdump, 0, 0);
            pData2.strDOB=fGetTagValue("0010,0030", strRawDCMdump, 0, 0);
            strSerIUID=fGetTagValue("0020,000e", strRawDCMdump, 0, 0);
            strSerDesc=fGetTagValue("0008,103e", strRawDCMdump, 0, 0);
            strModality=fGetTagValue("0008,0060", strRawDCMdump, 0, 0);
            strSopIUID=fGetTagValue("0008,0018", strRawDCMdump, 0, 0);
            pData2.strSIUID=fGetTagValue("0020,000d", strRawDCMdump, 0, 0);
            pData2.strStudyDate=fGetTagValue("0008,0020", strRawDCMdump, 0, 0);
            pData2.strStudyTime=fGetTagValue("0008,0030", strRawDCMdump, 0, 0);
            strStudyDateTime = pData2.strStudyDate + " " + pData2.strStudyTime;
            pData2.strACCN=fGetTagValue("0008,0050", strRawDCMdump, 0, 0);
            pData2.strStudyDesc=fGetTagValue("0008,1030", strRawDCMdump, 0, 0);
            pData2.strPatientComments=fGetTagValue("0010,4000", strRawDCMdump, 0, 0);
            pData2.strRequestedProcedureID=fGetTagValue("0040,1001", strRawDCMdump, 0, 1);
            //Create databse entries to show something is coming in.
            if(intImgNum == 1) {
                strLogMessage = "Updating DB entries for " + strPrimalID + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
            strQuery="select count(*) from study where puid='" + strPrimalID + "'";
            mysql_query(mconnect, strQuery.c_str());
            if(*mysql_error(mconnect)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect);
                strLogMessage+="\nstrQuery = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
            result = mysql_store_result(mconnect);
            if(result) {
                intNumRows = mysql_num_rows(result);
                if(intNumRows > 0) {
                    row = mysql_fetch_row(result);
                    strDBREturn=row[0];
                    intDBEntries=stoi(strDBREturn);
                }
                mysql_free_result(result);
            }
            if(intDBEntries <= 0) {
                strLogMessage = strPrimalID + " RECV  Need to add patient " + pData2.strPName + " to DB.";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                strQuery="select * from patient where puid='" + strPrimalID + "'";
                mysql_query(mconnect, strQuery.c_str());
                if(*mysql_error(mconnect)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect);
                    strLogMessage+="\nstrQuery = " + strQuery;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                }
                result = mysql_store_result(mconnect);
                if(result) {
                    intNumRows = mysql_num_rows(result);
                    if(intNumRows > 0) {
                        strQuery="update patient set pname = '" + pData2.strPName + "', '";
                        strQuery+="pid = '" + pData2.strMRN + "', '";
                        strQuery+="pdob = '" + pData2.strDOB + "', '";
                        strQuery+="PatientComments = '" + pData2.strPatientComments + "' where puid = '" + strPrimalID + "';";
                    } else {
                        strQuery="insert into patient (puid, pname, pid, pdob, PatientComments) values ('" + strPrimalID + "', '";
                        strQuery+=pData2.strPName + "', '" + pData2.strMRN + "', '" + pData2.strDOB + "', '";
                        strQuery+=pData2.strPatientComments + "');";
                    }
                    mysql_free_result(result);
                    mysql_query(mconnect, strQuery.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="\nstrQuery = " + strQuery;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    }
                } else {
                    strQuery="insert into patient (puid, pname, pid, pdob, PatientComments) values ('" + strPrimalID + "', '";
                    strQuery+=pData2.strPName + "', '" + pData2.strMRN + "', '" + pData2.strDOB + "', '";
                    strQuery+=pData2.strPatientComments + "');";
                    mysql_query(mconnect, strQuery.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="\nstrQuery = " + strQuery;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    }
                }
                if(strPrefetchNode.compare("1") == 0) {
                    strQuery="insert into study set puid='" + strPrimalID + "', SIUID='" + pData2.strSIUID + "'";
                    strQuery+=", StudyDate='" + strStudyDateTime + "', AccessionNum='" + pData2.strACCN + "'";
                    strQuery+=", sServerName='" + strHostname + "', StudyDesc='" + pData2.strStudyDesc + "'";
                    strQuery+=", sClientID='" + strClientID2 + "', sClientName='" + strClientName + "'";
                    strQuery+=", sRequestedProcedureID='" + pData2.strRequestedProcedureID + "', sCaseID = '" + strPrefetchNode + "';";
                } else {
                    strQuery="insert into study set puid='" + strPrimalID + "', SIUID='" + pData2.strSIUID + "'";
                    strQuery+=", StudyDate='" + strStudyDateTime + "', AccessionNum='" + pData2.strACCN + "'";
                    strQuery+=", sServerName='" + strHostname + "', StudyDesc='" + pData2.strStudyDesc + "'";
                    strQuery+=", sClientID='" + strClientID2 + "', sRequestedProcedureID='" + pData2.strRequestedProcedureID + "', sClientName='" + strClientName + "';";
                }
                mysql_query(mconnect, strQuery.c_str());
                if(*mysql_error(mconnect)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect);
                    strLogMessage+="\nstrQuery = " + strQuery;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                }
                //Pass a message to the Json service
                if(conf1.primConf[strRecNum + "_PRIJSON"] == "1") {
                    strLogMessage = strPrimalID + " RECV " +  "Creating JSON for " + strPrimalID + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    strCmd = strMessage + " 1";
                    fWriteMessage(strCmd, "/prim_process");
                }
            }
            strQuery="select count(*) from series where puid='" + strPrimalID + "' and SERIUID='" + strSerIUID + "';";
            mysql_query(mconnect, strQuery.c_str());
            if(*mysql_error(mconnect)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect);
                strLogMessage+="\nstrQuery = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
            result = mysql_store_result(mconnect);
            if(result) {
                intNumRows = mysql_num_rows(result);
                if(intNumRows > 0) {
                    row = mysql_fetch_row(result);
                    strDBREturn=row[0];
                    intDBEntries=stoi(strDBREturn);
                }
                mysql_free_result(result);
            }
            if(intDBEntries <= 0) {
                strLogMessage = strPrimalID + " RECV " + "Need to add series " + strSerIUID + " to DB";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                strQuery="insert into series set puid='" + strPrimalID + "', SIUID='" + pData2.strSIUID + "'";
                strQuery+=", SERIUID='" + strSerIUID + "', SeriesDesc='" + strSerDesc + "', Modality='" + strModality + "';";
                mysql_query(mconnect, strQuery.c_str());
            }
            //std::cout << "Need to add the SOPIUID" << std::endl;
            strQuery="insert into image set SOPIUID='" + strSopIUID + "', SERIUID='" + strSerIUID + "', puid='";
            strQuery+=strPrimalID + "', iservername='" + strHostname + "', ifilename='" + strFilename;
            strQuery+="', idate='" + GetDate() + "';";
            mysql_query(mconnect, strQuery.c_str());
            if(*mysql_error(mconnect)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect);
                strLogMessage+="\nstrQuery = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
            //Check to verify the database was updated
            strQuery="select puid from patient where puid = '" + strPrimalID + "';";
            mysql_query(mconnect, strQuery.c_str());
            if(*mysql_error(mconnect)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect);
                strLogMessage+="\nstrQuery = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
            result = mysql_store_result(mconnect);
            if(result) {
                intNumRows = mysql_num_rows(result);
                if(intNumRows > 0) {
                    row = mysql_fetch_row(result);
                    strResult = row[0];
                }
                mysql_free_result(result);
            }
            if(strResult != strPrimalID) {
                strLogMessage = "WARN:  Patient table does not have the Primal ID " + strPrimalID;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
            break;
        }
    }
    return 0;
}

std::size_t fUpdateTimestamp(std::string strRecNum, std::string strFullPath, std::time_t intLastUpdateTime, std::size_t intNumImg) {
    std::size_t intTemp, intImgNum, intRecieveTimeOut, intNumRows, intTimeStamp, intEndTime;
    std::string strPrimalID, strTemp, strFilename, strLogMessage, strQuery;
    std::map<std::string, std::string>::iterator iprimConf;

    //Count number of files in directory
    //Remove the last character if it's a /
    if(strFullPath.back() == '/') {
        strFullPath.pop_back();
    }
    intTemp = strFullPath.find_last_of("/");
    strPrimalID=strFullPath.substr(intTemp+1);
    //Make sure last character is a /
    if(strFullPath.back() != '/') {
        strFullPath.push_back('/');
    }
    std::cout << "Searching " << strFullPath << std::endl;
    intImgNum=0;
    for (const auto & entry : fs::directory_iterator(strFullPath)) {
        strTemp=entry.path().string();
        intTemp = strTemp.find_last_of("/");
        strFilename=strTemp.substr(intTemp+1);
        intTemp=strFilename.find_last_of(".");
        if(intTemp != std::string::npos) {
            strTemp=strFilename.substr(intTemp);
        }
        if(strTemp == ".dcm") {
            intImgNum++;
        }
    }
    std::cout << "Found " << to_string(intImgNum) << " files in directory " << strFullPath << std::endl;
    //create new timestamp
    intTimeStamp = std::time(0);
    //Add receive time out to passed timestamp
    iprimConf = conf1.primConf.find(strRecNum + "_PRIRECTO");
    if(iprimConf != conf1.primConf.end()) {
        std::stringstream sstream(conf1.primConf[strRecNum + "_PRIRECTO"]);
        sstream >> intRecieveTimeOut;
    } else {
        intRecieveTimeOut = 30;
    }
    intEndTime = intLastUpdateTime + intRecieveTimeOut;
    //Query DB for end receive time
    std::cout << "Current time is: " << to_string(intTimeStamp) << " Receive timeout is: " << to_string(intEndTime) << std::endl;
    strQuery="select tendrec from receive where puid = \"" + strPrimalID + "\" and tendrec != NULL;";
    mysql_query(mconnect, strQuery.c_str());
    if(*mysql_error(mconnect)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect);
        strLogMessage+="\nstrQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    }
    result = mysql_store_result(mconnect);
    if(result) {
        intNumRows = mysql_num_rows(result);
        if(intNumRows > 0) {
            mysql_free_result(result);
            std::cout << "tendrec is not NULL for " << strPrimalID << " ending receive." << std::endl;
            fEndReceive(strFullPath);
            return 0;
        }
        mysql_free_result(result);
    }
    strQuery="select rec_images from receive where puid = '" + strPrimalID + "';";
    mysql_query(mconnect, strQuery.c_str());
    if(*mysql_error(mconnect)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect);
        strLogMessage+="\nstrQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    }
    result = mysql_store_result(mconnect);
    if(result) {
        intNumRows = mysql_num_rows(result);
        if(intNumRows > 0) {
            row = mysql_fetch_row(result);
            strTemp=row[0];
            std::stringstream sstream(strTemp);
            sstream >> intNumImg;
            //std::cout << "strTemp = " << strTemp << " intNumImg = " << to_string(intNumImg) << std::endl;
        } else {
            std::cout << "Couldn't get number of images from the DB for " << strPrimalID << ".  Setting to 0." << std::endl;
            intNumImg = 0;
        }
        mysql_free_result(result);
    } else {
        std::cout << "Couldn't get number of images from the DB for " << strPrimalID << ".  Setting to 0." << std::endl;
        intNumImg = 0;
    }
    //if(number of files passed to function not equal to number of files in the directory)
    if(intNumImg != intImgNum) {
        std::cout << "Updating database for " << strPrimalID << " with " << to_string(intImgNum) << " images.  DB had count was: " << to_string(intNumImg) << std::endl;
        strQuery = "update receive set ttimestamp = " + to_string(intTimeStamp) + ", rec_images = " + to_string(intImgNum);
        strQuery += " where puid = \"" + strPrimalID + "\"";
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect);
            strLogMessage+="\nstrQuery = " + strQuery + ".";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        }
        return 0;
    }
    //if(updated passed timestamp < current timestamp)
    if(intEndTime < intTimeStamp ) {
        std::cout << "Timeout has been reached.  Ending receive..." << std::endl;
        fEndReceive(strFullPath);
        return 0;
    }
    return 0;
}

std::size_t fPollRecvDir(std::string strRecNum) {
    std::size_t intTimeStamp, intNumImg, intNumRows, intFound;
    std::string strTemp2, strCMD, strCmd, strLogMessage, strFilename, strPrimalID, strRawDCMdump, strPName, strMRN;
    std::string strDOB, strSerIUID, strSerDesc, strModality, strSopIUID, strSIUID, strStudyDate, strACCN, strStudyDesc;
    std::string strPatientComments, strTemp, strQuery, strDBReturn, strStartRec, strResult, strStudyTime, strStudyDateTime;
    std::string strFullPath, strFileExtension, strDirName, strWorkingDirectory, strRecNum2;
    std::map<std::string, std::string>::iterator iprimConf;
    //Key for the following maps should be identical.
    std::map<std::string, std::time_t> mapLastChangeTime;
    std::map<std::string, std::size_t> mapNumImg;
    std::map<std::string, std::time_t>::iterator itLastChangeTime;
    std::map<std::string, std::size_t>::iterator itNumImg;
    int intTemp;

    if(!fs::exists(conf1.primConf[strRecNum + "_PRIIF"])) {
        strLogMessage = " RECV WARN:  Directory" + conf1.primConf[strRecNum + "_PRIIF"] + " does not exist.  Skipping...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        return 1;
    }
    iprimConf = conf1.primConf.find(strRecNum + "_PRIIF");
    if(iprimConf != conf1.primConf.end()) {
        strWorkingDirectory = conf1.primConf[strRecNum + "_PRIIF"];
    } else {
        strLogMessage = " RECV ERROR:  Could not find inbound directory for recver # " + strRecNum;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        return 1;
    }

    while(true) {
        for (const auto & entry2 : fs::directory_iterator(conf1.primConf[strRecNum + "_PRIIF"] + "/")) {
            strFullPath=entry2.path().string();
            intTemp = strFullPath.find_last_of("/");
            strDirName=strFullPath.substr(intTemp+1);
            intTemp = strDirName.find_first_of("_");
            strRecNum2 = strDirName.substr(0,intTemp);
            if(strRecNum == strRecNum2) {
                strLogMessage="RECV Receiver # " + strRecNum + " found directory " + strDirName;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                strQuery="select rservername from receive where puid = \"" + strDirName + "\";";
                mysql_query(mconnect, strQuery.c_str());
                if(*mysql_error(mconnect)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect);
                    strLogMessage+="strQuery = " + strQuery + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    std::cout << strLogMessage << std::endl;
                }
                result = mysql_store_result(mconnect);
                if(result) {
                    intNumRows = mysql_num_rows(result);
                    if(intNumRows > 0) {
                        row = mysql_fetch_row(result);
                        strTemp=row[0];
                        intFound=strTemp.find(strHostname);
                        if(intFound != std::string::npos) {
                            mysql_free_result(result);
                            strQuery="select puid, ttimestamp, rec_images from receive where puid = \"" + strDirName + "\";";
                            mysql_query(mconnect, strQuery.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="strQuery = " + strQuery + ".";
                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                            }
                            result = mysql_store_result(mconnect);
                            if(result) {
                                intNumRows = mysql_num_rows(result);
                                if(intNumRows > 0) {
                                    row = mysql_fetch_row(result);
                                    strTemp=row[1];
                                    std::stringstream sstream(strTemp);
                                    sstream >> intTimeStamp;
                                    strTemp=row[2];
                                    std::stringstream sstream2(strTemp);
                                    sstream2 >> intNumImg;
                                    mysql_free_result(result);
                                    strLogMessage="RECV Receiver # " + strRecNum + " updating timestamp of " + to_string(intTimeStamp) + " for " + strDirName;
                                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                                    fUpdateTimestamp(strRecNum, strFullPath, intTimeStamp, intNumImg);
                                } else {
                                    strLogMessage="RECV starting receive for " + strDirName + " 1";
                                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                                    fStartReceive(strFullPath);
                                }
                            } else {
                                strLogMessage="RECV starting receive for " + strDirName + " 2";
                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                                fStartReceive(strFullPath);
                            }
                        } else {
                            mysql_free_result(result);
                            strLogMessage="RECV starting receive for " + strDirName + "3";
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                            fStartReceive(strFullPath);
                        }
                    } else {
                        mysql_free_result(result);
                        strLogMessage="RECV starting receive for " + strDirName + "4";
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                        fStartReceive(strFullPath);
                    }
                } else {
                    strLogMessage="RECV starting receive for " + strDirName + " 5";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    fStartReceive(strFullPath);
                }
            }
        }
        std::this_thread::sleep_for (std::chrono::seconds(3));
    }
    return 0;
}

void fLaunchChild() {
    std::string strCMD, strReturn, strDirPath, strLogMessage, strRecNum;
    std::size_t intReturn, intFound, intLC=1;
    std::map<std::string, std::string>::iterator iprimConf;

    strRecNum = "1";
    strLogMessage = " RECV Launching Queue monitor...";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    strCMD = "/usr/local/bin/prim_receive_server QUEUE &";
    system(strCMD.c_str());
    iprimConf = conf1.primConf.find(to_string(intLC) + "_PRIIF");
    while(iprimConf != conf1.primConf.end()) {
        strDirPath = conf1.primConf[to_string(intLC) + "_PRIIF"];
        intFound = 0;
        strCMD = "ps -ef|grep \"prim_receive_server " + to_string(intLC) + "\"|grep -v grep|wc -l";
        strReturn = exec(strCMD.c_str());
        std::stringstream sstream(strReturn);
        sstream >> intReturn;
        if(intFound == 0) {
            if(conf1.primConf[to_string(intLC) + "_PRIRECTYPE"] != "FTP") {
                strLogMessage = "RECV Launching directory monitor for receiver # " + to_string(intLC);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                strCMD = "/usr/local/bin/prim_receive_server " + to_string(intLC) + " &";
                system(strCMD.c_str());
            } else {
                strLogMessage = "RECV Skipping receiver # " + to_string(intLC) + " because it should be handled by PRIM_STORE...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
        } else {
            strLogMessage = "RECV Skipping reciever # " + to_string(intLC) + " because the directory should already be handeled...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        }
        intLC++;
        iprimConf = conf1.primConf.find(to_string(intLC) + "_PRIIF");
    }
    //Need to watch the children and make sure they didn't die.
    while(true) {
        intLC=0;
        iprimConf = conf1.primConf.find(to_string(intLC) + "_PRIIF");
        while(iprimConf != conf1.primConf.end()) {
            strDirPath = conf1.primConf[to_string(intLC) + "_PRIIF"];
            intFound = 0;
            strCMD = "ps -ef|grep \"prim_receive_server " + to_string(intLC) + "\"|grep -v grep|wc -l";
            strReturn = exec(strCMD.c_str());
            std::stringstream sstream(strReturn);
            sstream >> intReturn;
            if(intFound == 0) {
                strCMD = "/usr/local/bin/prim_receive_server " + to_string(intLC) + " &";
                system(strCMD.c_str());
                strLogMessage = " RECV WARN  Relaunching prim_receive for receiver # " + to_string(intLC);
                fWriteLog(strLogMessage, conf1.primConf[to_string(intLC) + "_PRILOGDIR"] + "/" + conf1.primConf[to_string(intLC) + "_PRILFIN"]);
            }
            intLC++;
            iprimConf = conf1.primConf.find(to_string(intLC) + "_PRIIF");
        }
        std::this_thread::sleep_for (std::chrono::seconds(60));
    }
    return;
}

void fQueueMonitor() {
    std::string strMessage, strCallingAETitle, strCalledAETitle, strPrimalID, strQuery, strLogMessage, strRecNum;
    std::size_t intMessageType, intTemp, intNumRows;
    std::vector<std::string> vMessage;
    std::stringstream sstream("1");
    
    std::cout << "Starting queue monitor" << std::endl;
    fWriteLog("Starting queue monitor", conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    while(true) {
        strMessage=fGetMessage("/prim_receive");
        //vMessage[0] = Full path to dicom files
        //vMessage[1] = Message type (1 or 2)
        //vMessage[2] = calling application entity title
        //vMessage[3] = called application entity title
        vMessage = split(strMessage, ' ');
        if(vMessage.size() > 3) {
            strCallingAETitle=vMessage[2];
            strCalledAETitle=vMessage[3];
        }
        sstream.clear();
        sstream.str(vMessage[1]);
        sstream >> intMessageType;
        //intMessageType=stoi(vMessage[1]);
        if(intMessageType != 1) {
            std::cout << "Message type = " << to_string(intMessageType) << std::endl;
        }
        intTemp = vMessage[0].find_last_of("/");
        strPrimalID=vMessage[0].substr(intTemp+1);
        intTemp = strPrimalID.find_first_of("_");
        strRecNum = strPrimalID.substr(0, intTemp);
        strQuery="select count(*) from receive where puid='" + strPrimalID + "'";
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect);
            strLogMessage+="\nstrQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            continue;
        }
        result = mysql_store_result(mconnect);
        //if(result) {
        //    result = mysql_store_result(mconnect_local);
        strQuery="select senderAET from receive where puid='" + strPrimalID + "'";
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect);
            strLogMessage+="\nstrQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            continue;
        }
        result = mysql_store_result(mconnect);
        if(result) {
            intNumRows = mysql_num_rows(result);
            if(intNumRows < 1) {
                mysql_free_result(result);
                strQuery = "insert into receive(puid, rservername, tstartrec, senderAET, callingAET) values('" + strPrimalID + "', '" + strHostname + "', '" + GetDate() + "', '" + strCalledAETitle + "', '" + strCallingAETitle + "');";
                mysql_query(mconnect, strQuery.c_str());
                if(*mysql_error(mconnect)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect);
                    strLogMessage+="strQuery = " + strQuery + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    continue;
                }
            } else {
                mysql_free_result(result);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return;
}


int main(int argc, char** argv) {
    std::string strARG, strRecNum, strLogMessage;
    std::map<std::string, std::string>::iterator iprimConf;

    strRecNum = "1";
    system("export DCMDICTPATH=/opt/primal/home/build/share/dcmtk/dicom.dic");
    ReadDBConfFile();
    conf1.ReadConfFile();
    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage+="MySQL Initilization failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        return 1;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage+="MySQL connection failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        return 1;
    }

    strLogMessage="Starting prim_receive_server version 2.01.02";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    if(argc == 1) {
        std::cout << "Launching children..." << std::endl;
        strLogMessage=" RECV Launching children... ";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        fLaunchChild();
    } else if(argc == 2) {
        strARG = argv[1];
        if(strARG == "QUEUE") {
            fQueueMonitor();
        } else {
            //this could be any of the receiver numbers
            strARG = argv[1];
            fPollRecvDir(strARG);
        }
    } else {
        std::cout << "Wrong number of arguments: " << to_string(argc) << std::endl;
        return 1;
    }
    mysql_close(mconnect);
    return 0;
}