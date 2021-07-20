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
#include <mysql/my_global.h>
#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <future>

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

const std::string strProcChainType = "PRIMRCSTOR";

#include "prim_functions.h"

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

std::string fGetMessage() {
    mqd_t fdMQueue;
    char msg_buffer[129];
    struct mq_attr attr;
    ssize_t num_bytes_received = -1;
    msg_buffer[10] = 0;
    std::string strReturn;
    unsigned int prio;

    attr.mq_maxmsg = 1024;
    attr.mq_msgsize = 128;
    attr.mq_flags   = 0;

    /*
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 1024;
    attr.mq_curmsgs = 0;
    */

    fdMQueue = mq_open("/prim_receive", O_CREAT | O_RDONLY, 0666, &attr);
    if (fdMQueue == -1) {
        perror("ERROR:  Couuld not open /prim_receive queue.");
        exit(0);
    }
    while (num_bytes_received == -1)  {
        memset(msg_buffer, 0x00, sizeof(msg_buffer));
        num_bytes_received = mq_receive(fdMQueue, msg_buffer, 128, &prio);
        if(num_bytes_received == -1) {
            std::this_thread::sleep_for (std::chrono::milliseconds(1));
        }
    }
    //mq_close(fdMQueue);
    //mq_unlink("/prim_json");
    strReturn = msg_buffer;
    fflush(stdout);

    return strReturn;
}

std::string fCreateCase(std::string strPrimalID, std::string strTemp) {
    std::size_t intFound, intTemp, intImgNum, intPos;
    std::string strICaseID, strJson, strTemp2, strQuery, strRow, strFilename, strRecNum, strLogMessage;
    MYSQL_ROW row;
    MYSQL_RES *result;
    MYSQL *mconnect2;

    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return "-1";
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return "-1";
    }

    strICaseID = strPrimalID;
    //Remove non-digit characters from PrimalID
    intFound = strICaseID.find("/");
    while(intFound != std::string::npos) {
        strICaseID.erase(intFound);
        intFound = strICaseID.find("/");
    }
    intPos = strPrimalID.find("_");
    if(intPos != std::string::npos) {
        strRecNum=strPrimalID.substr(0,intPos);
    } else {
        strRecNum = "1";
    }
    //Find the location of the file from the database
    strQuery="select ilocation from image where puid='" + strPrimalID + "' group by ilocation;";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        while ((row = mysql_fetch_row(result))) {
            strRow = row[0];
            for (const auto & entry : fs::directory_iterator(strRow)) {
                strLogMessage = "Processing " + entry.path().string();
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                strTemp2=entry.path().string();
                intTemp = strTemp2.find_last_of("/");
                strFilename=strTemp2.substr(intTemp+1);
                intImgNum++;
                if(intImgNum == 1) {
                    strTemp=fDcmDump(entry.path().string());
                    strJson = "[{\"dicom_cases\":{\"dicom_cases_id\":\"" + strPrimalID;
                    strJson.append("\",\"remote_dicom_cases_id\":\"0\",\"cases_id\":null,\"initial_cases_id\":null");
                    strJson.append(",\"status\":\"Receiving\",\"remote_case_status\":\"Processing\",\"status_problem\":\"None\",\"remote_title\":\"");
                    strTemp2=fGetTagValue("0002,0016", strTemp, 0);
                    strJson.append(strTemp2);
                    strJson.append("\",\"local_title\":\"");
                    

                    strJson.append("  \"transactionId\": \"" + strPrimalID + "\",\n");
                    strJson.append("  \"studies\": [\n");
                    strJson.append("    {\n");
                        strJson.append("      \"studyInstanceUid\": \"");
                        strTemp2=fGetTagValue("0020,000d", strTemp, 0);
                        strJson.append(strTemp2 + "\",\n");
                        strJson.append("      \"studyDescription\": \"");
                        strTemp2=fGetTagValue("0008,1030", strTemp, 0);
                        strJson.append(strTemp2 + "\",\n");
                        strJson.append("      \"studyDate\": \"");
                        strTemp2=fGetTagValue("0008,0020", strTemp, 0);
                        strJson.append(strTemp2 + "\",\n");
                        strJson.append("      \"studyTime\": \"");
                        strTemp2=fGetTagValue("0008,0030", strTemp, 0);
                        strJson.append(strTemp2 + "\",\n");
                        strJson.append("      \"patientName\": \"");
                        strTemp2=fGetTagValue("0010,0010", strTemp, 0);
                        strJson.append(strTemp2 + "\",\n");
                        strJson.append("      \"patientBirthDate\": \"");
                        strTemp2=fGetTagValue("0010,0030", strTemp, 0);
                        strJson.append(strTemp2 + "\",\n");
                        strJson.append("      \"patientId\": \"");
                        strTemp2=fGetTagValue("0010,0020", strTemp, 0);
                        strJson.append(strTemp2 + "\",\n");
                        strJson.append("      \"patientSex\": \"");
                        strTemp2=fGetTagValue("0010,0040", strTemp, 0);
                        strJson.append(strTemp2 + "\",\n");
                        strJson.append("      \"seriesList\": [\n");
                        strJson.append("        {\n");
                }
            }
        }
    }
    mysql_free_result(result);
    mysql_close(mconnect2);
    return "OK";
}

std::string fGetMD5(std::string strFullFilename) {
    ifstream::pos_type fileSize;
    std::string strReturn;
    std::ifstream file(strFullFilename, std::ifstream::binary);
    MD5_CTX md5Context;
    unsigned char result[MD5_DIGEST_LENGTH];
    char buf[1024 * 16];
    std::stringstream md5string;

    MD5_Init(&md5Context);
    while (file.good()) {
        file.read(buf, sizeof(buf));
        MD5_Update(&md5Context, buf, file.gcount());
    }
    MD5_Final(result, &md5Context);
    md5string << std::hex << std::uppercase << std::setfill('0');
    for (const auto &byte: result)
        md5string << std::setw(2) << (int)byte;

    strReturn = md5string.str();
    return strReturn;
}

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

std::size_t fStartupCheck(std::string strRecNum) {
    std::string strLogMessage;
    std::map<std::string, std::string>::iterator iprimConf;

    iprimConf = conf1.primConf.find(strRecNum + "_PRILFIN");
    if(iprimConf != conf1.primConf.end()) {
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    } else {
        std::cerr << "ERROR:  Could not find log file config entry.  Exiting..." << std::endl;
        return 1;
    }
    iprimConf = conf1.primConf.find(strRecNum + "_PRIIF");
    if(iprimConf != conf1.primConf.end()) {
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    } else {
        strLogMessage = "ERROR:  Could not find inbound directory config entry.  Exiting...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cerr << strLogMessage << std::endl;
        return 1;
    }
    if(! fs::exists(conf1.primConf[strRecNum + "_PRIIF"])) {
        strLogMessage = "ERROR:  Inbound directory " + conf1.primConf[strRecNum + "_PRIIF"] + " does not exist.  Exiting...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cerr << strLogMessage << std::endl;
        return 1;
    }
    iprimConf = conf1.primConf.find(strRecNum + "_PRIPROC");
    if(iprimConf == conf1.primConf.end()) {
        std::cerr << "ERROR:  Could not find processing directory config entry.  Exiting..." << std::endl;
        return 1;
    }
    if(! fs::exists(conf1.primConf[strRecNum + "_PRIPROC"])) {
        strLogMessage = "ERROR:  Processing directory " + conf1.primConf[strRecNum + "_PRIPROC"] + " does not exist.  Exiting...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cerr << strLogMessage << std::endl;
        return 1;
    }
    return 0;
}

void fGetCaseID(std::string strFullPath, std::string strPrimalID, std::string strRecNum) {
    std::string strLine, strCaseID, strQuery, strLogMessage, strClientID, strClientName;
    std::size_t intPos;

    if(strFullPath.back() != '/') {
        strFullPath.push_back('/');
    }
    strLogMessage=" STOR " + strPrimalID + " Searching for CaseID.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    MYSQL *mconnect2;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        cout << "MySQL Initilization failed";
        return;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        cout<<"connection failed\n";
        return;
    }

    if(fs::exists(strFullPath + "package.conf")) {
        std::ifstream fpAEMap(strFullPath + "package.conf");
        while (std::getline(fpAEMap, strLine)) {
            intPos=strLine.find("CASEID = ");
            if(intPos != std::string::npos && intPos < 2) {
                intPos=strLine.find("=");
                strCaseID=strLine.substr(intPos +2);
                strQuery="update study set sCaseID='" + strCaseID + "' where puid='" + strPrimalID + "' limit 1;";
                mysql_query(mconnect2, strQuery.c_str());
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    strLogMessage+="\nstrQuery: " + strQuery;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                } else {
                    strLogMessage=" STOR " + strPrimalID + " Inserted CaseID " + strCaseID + " into study table.";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                }
            }
            intPos=strLine.find("ID = ");
            if(intPos != std::string::npos && intPos < 2) {
                intPos=strLine.find("=");
                strClientID=strLine.substr(intPos +2);
                strQuery="update study set sClientID='" + strClientID + "' where puid='" + strPrimalID + "' limit 1;";
                mysql_query(mconnect2, strQuery.c_str());
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    strLogMessage+="\nstrQuery: " + strQuery;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                } else {
                    strLogMessage=" STOR " + strPrimalID + " Inserted ClientID " + strClientID + " into study table.";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                }
            }
            intPos=strLine.find("NAME = ");
            if(intPos != std::string::npos && intPos < 2) {
                intPos=strLine.find("=");
                strClientName=strLine.substr(intPos +2);
                strQuery="update study set sClientName='" + strClientName + "' where puid='" + strPrimalID + "' limit 1;";
                mysql_query(mconnect2, strQuery.c_str());
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    strLogMessage+="\nstrQuery: " + strQuery;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                } else {
                    strLogMessage=" STOR " + strPrimalID + " Inserted Client Name " + strClientName + " into study table.";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                }
            }
        }
        fpAEMap.close();
    } else {
        strLogMessage=" STOR " + strPrimalID + " WARN:  " + strFullPath + " does not exist.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    }
    strLogMessage=" STOR " + strPrimalID + " Finished searching for CaseID.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    mysql_close(mconnect2);
    return;
}

std::size_t fProcFile(std::string strTemp, std::string strRecNum) {
    std::size_t intDBEntries, intPos, intLC2, intFound, intNumRows, intReturn;
    std::string strTemp2, strCMD, strCmd, strLogMessage, strFilename, strPrimalID, strRawDCMdump, strPName, strMRN, strReturn;
    std::string strDOB, strSerIUID, strSerDesc, strModality, strSopIUID, strSIUID, strStudyDate, strACCN, strStudyDesc;
    std::string strPatientComments, strTemp3, strQuery, strDBReturn, strStartRec, strResult, strStudyTime, strStudyDateTime;
    int intLC, intTemp;
    std::stringstream sstream("1");

    mysql_thread_init();
    MYSQL_ROW row;
    MYSQL_RES *result;
    std::map<std::string, std::string>::iterator iprimConf;
    std::vector<std::thread> vecThreads;

    time_t t2 = time(0);   // get time now
    struct tm * now2 = localtime( & t2 );

    MYSQL *mconnect2;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        cout<<"connection failed\n";
        return 1;
    }
    if(!fs::exists(strTemp)) {
        strLogMessage = " STOR WARN:  Directory" + strTemp + " does not exist.  Skipping...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cout << strLogMessage << std::endl;
        return 1;
    }

    intPos = strTemp.find_last_of("/");
    strFilename=strTemp.substr(intPos+1);
    intPos=strFilename.find_last_of(".");
    if(intPos != std::string::npos) {
        strTemp2=strFilename.substr(intPos);
    }
    //Need to set the receiver number
    intPos=strFilename.find_first_of("_");
    /*
    if(intPos != std::string::npos && intPos < strFilename.length()) {
        strTemp3=strFilename;
        strTemp3.erase(0,intPos);
        strTemp3 = strRecNum + strTemp3;
    } else {
        strTemp3 = strRecNum + "_" + strFilename;
    }
    */
    strTemp3 = strRecNum + "_" + fMakePUID() + ".tar";
    strLogMessage = " STOR Renaming " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strFilename + " to " + strTemp3;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    std::cout << strLogMessage << std::endl;
    fs::rename(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strFilename, conf1.primConf[strRecNum + "_PRIIF"] + "/" + strTemp3);
    strFilename = strTemp3;
    intPos = strFilename.find(".");
    if(intPos != std::string::npos) {
        strPrimalID=strFilename.substr(0, intPos);
    } else {
        strPrimalID = strFilename;
    }
    std::cout << "Processing PrimalID = " << strPrimalID << std::endl;
    fs::create_directory(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID);
    fs::rename(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strFilename, conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/" + strFilename);
    strCMD = "(cd " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/ && tar -xf " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/" + strFilename + ")";
    system(strCMD.c_str());
    intLC2=0;
    intFound=0;
    while (intLC2 < 15 && intFound != 1) {
        if(!fs::exists(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/_DICOM.tar.gz")) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } else {
            strCMD = "(cd " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/ && tar -xf " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/_DICOM.tar.gz)";
            system(strCMD.c_str());
            intFound = 1;
        }
        intLC2++;
    }
    if(intFound != 1) {
        strLogMessage = " STOR WARN  Archive " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/_DICOM.tar.gz" + " does not exist.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    }
    //Find all .dcm files and move them to the parent directory
    strCMD = "find " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + " -iname \"*.dcm\" -exec mv {} " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/ \\;";
    system(strCMD.c_str());
    strCMD = "ls -1 " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/*.dcm|wc -l";
    strReturn = exec(strCMD.c_str());
    sstream.clear();
    sstream.str(strReturn);
    sstream >> intReturn;
    if(intReturn < 1) {
        strLogMessage = " STOR ERROR  " + strPrimalID + " File does not contain any .dcm files.  Exiting...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        return 1;
    }

    intLC=0;
    for (const auto & entry2 : fs::directory_iterator(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/")) {
        strTemp2=entry2.path().string();
        intTemp = strTemp2.find_last_of("/");
        strFilename=strTemp2.substr(intTemp+1);
        intPos=strFilename.find(".dcm");
        if(intPos != std::string::npos) {
            intLC++;
            //std::cout << "Found a DICOM file: " << strFilename << std::endl;
            strRawDCMdump=fDcmDump(strTemp2);
            strPName=fGetTagValue("0010,0010", strRawDCMdump, 0);
            strMRN=fGetTagValue("0010,0020", strRawDCMdump, 0);
            strDOB=fGetTagValue("0010,0030", strRawDCMdump, 0);
            strSerIUID=fGetTagValue("0020,000e", strRawDCMdump, 0);
            strSerDesc=fGetTagValue("0008,103e", strRawDCMdump, 0);
            strModality=fGetTagValue("0008,0060", strRawDCMdump, 0);
            strSopIUID=fGetTagValue("0008,0018", strRawDCMdump, 0);
            strSIUID=fGetTagValue("0020,000d", strRawDCMdump, 0);
            strStudyDate=fGetTagValue("0008,0020", strRawDCMdump, 0);
            strStudyTime=fGetTagValue("0008,0030", strRawDCMdump, 0);
            strStudyDateTime = strStudyDate + " " + strStudyTime;
            strACCN=fGetTagValue("0008,0050", strRawDCMdump, 0);
            strStudyDesc=fGetTagValue("0008,1030", strRawDCMdump, 0);
            strPatientComments=fGetTagValue("0010,4000", strRawDCMdump, 0);
            //Create databse entries to show something is coming in.
            if(intLC == 1) {
                strLogMessage = strPrimalID + " STOR " + "Updating DB entries for " + strPrimalID + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            }
            //std::cout << strLogMessage << std::endl;
            strQuery="select count(*) from study where puid='" + strPrimalID + "'";
            mysql_query(mconnect2, strQuery.c_str());
            if(*mysql_error(mconnect2)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect2);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
            }
            result = mysql_store_result(mconnect2);
            if(result) {
                intNumRows=mysql_num_rows(result);
                if(intNumRows > 0) {
                    row = mysql_fetch_row(result);
                    strDBReturn=row[0];
                    intDBEntries=stoi(strDBReturn);
                }
                mysql_free_result(result);
            }
            if(intDBEntries <= 0) {
                strQuery="insert into patient (puid, pname, pid, pdob, PatientComments) values ('" + strPrimalID + "', '";
                strQuery+=strPName + "', '" + strMRN + "', '" + strDOB + "', '" + strPatientComments + "');";
                //std::cout << strQuery << std::endl;
                mysql_query(mconnect2, strQuery.c_str());
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                }
                strQuery="insert into receive (puid, rservername, tstartrec, rec_images, senderAET, callingAET) values ('";
                strQuery+=strPrimalID + "', '" + strHostname + "', '";
                strStartRec=std::to_string(now2->tm_year + 1900);
                strStartRec.append("-");
                strStartRec+=std::to_string(now2->tm_mon + 1);
                strStartRec.append("-");
                strStartRec+=std::to_string(now2->tm_mday);
                strStartRec.append(" ");
                strStartRec+=std::to_string(now2->tm_hour);
                strStartRec.append(":");
                strStartRec+=std::to_string(now2->tm_min);
                strStartRec.append(":");
                strStartRec+=std::to_string(now2->tm_sec);
                strQuery+=strStartRec + "', '1', 'SCP', 'SCP');";
                mysql_query(mconnect2, strQuery.c_str());
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                }
                strQuery="insert into study set puid='" + strPrimalID + "', SIUID='" + strSIUID + "'";
                strQuery+=", StudyDate='" + strStudyDateTime + "', AccessionNum='" + strACCN + "'";
                strQuery+=", sServerName='" + strHostname + "', StudyDesc='" + strStudyDesc + "';";
                mysql_query(mconnect2, strQuery.c_str());
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                }
                //Pass a message to the Json service
                if(conf1.primConf[strRecNum + "_PRIJSON"] == "1") {
                    strLogMessage = strPrimalID + " STOR " +  "Creating JSON for " + strPrimalID + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    strCmd = conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/" + " 1";
                    fWriteMessage(strCmd, "/prim_process");
                    //system(strCmd.c_str());
                }
            }
            strQuery="select count(*) from series where puid='" + strPrimalID + "' and SERIUID='" + strSerIUID + "';";
            mysql_query(mconnect2, strQuery.c_str());
            if(*mysql_error(mconnect2)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect2);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
            }
            if(result) {
                result = mysql_store_result(mconnect2);
                intNumRows = mysql_num_rows(result);
                if(intNumRows >  0) {
                    row = mysql_fetch_row(result);
                    strDBReturn=row[0];
                    intDBEntries=stoi(strDBReturn);
                }
                mysql_free_result(result);
            }
            if(intDBEntries <= 0) {
                strLogMessage = strPrimalID + " STOR " + "Need to add series " + strSerIUID + " to DB";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                strQuery="insert into series set puid='" + strPrimalID + "', SIUID='" + strSIUID + "'";
                strQuery+=", SERIUID='" + strSerIUID + "', SeriesDesc='" + strSerDesc + "', Modality='" + strModality + "';";
                mysql_query(mconnect2, strQuery.c_str());
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                }
            }
            //std::cout << "Need to add the SOPIUID" << std::endl;
            strQuery="insert into image set SOPIUID='" + strSopIUID + "', SERIUID='" + strSerIUID + "', puid='";
            strQuery+=strPrimalID + "', iservername='" + strHostname + "', ifilename='" + strFilename;
            strQuery+="', idate='" + GetDate() + "';";
            mysql_query(mconnect2, strQuery.c_str());
            if(*mysql_error(mconnect2)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect2);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
            }
        }
    }
    //Check to verify the database was updated
    strQuery="select puid from patient where puid = '" + strPrimalID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    }
    if(result) {
        result = mysql_store_result(mconnect2);
        intNumRows = mysql_num_rows(result);
        if(intNumRows > 0) {
            row = mysql_fetch_row(result);
            strResult = row[0];
        }
        mysql_free_result(result);
    }
    if(strResult != strPrimalID) {
        strLogMessage = strPrimalID + " STOR " + "WARN:  Patient table does not have the Primal ID " + strPrimalID;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    } else {
        fGetCaseID(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID + "/", strPrimalID, strRecNum);
    }
    strStartRec=std::to_string(now2->tm_year + 1900);
    strStartRec.append("-");
    strStartRec+=std::to_string(now2->tm_mon + 1);
    strStartRec.append("-");
    strStartRec+=std::to_string(now2->tm_mday);
    strStartRec.append(" ");
    strStartRec+=std::to_string(now2->tm_hour);
    strStartRec.append(":");
    strStartRec+=std::to_string(now2->tm_min);
    strStartRec.append(":");
    strStartRec+=std::to_string(now2->tm_sec);
    strQuery="update receive set tendrec = '" + strStartRec + "', rec_images = " + to_string(intLC) + " where puid = '" + strPrimalID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    }
    //fs::rename(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID, conf1.primConf[strRecNum + "_PRIPROC"] + "/" + strPrimalID);
    fs::copy(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID, conf1.primConf[strRecNum + "_PRIPROC"] + "/" + strPrimalID);
    fs::remove_all(conf1.primConf[strRecNum + "_PRIIF"] + "/" + strPrimalID);
    strTemp=conf1.primConf[strRecNum + "_PRIPROC"] + "/" + strPrimalID + " 2";
    //std::cout << "message = " << strTemp << std::endl;
    fWriteMessage(strTemp, "/prim_process");
    strLogMessage = strPrimalID + " STOR Finished receiving for patient: " + strPName + " MRN: " + strMRN + " Accession#: " + strACCN;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    std::cout << strLogMessage << std::endl;
    mysql_close(mconnect2);
    mysql_thread_end();
    return 0;
}

void fProcMessage() {
    std::string strMessage, strRecNum, strLogMessage, strFullPath, strReturn, strLine, strLine2, strCmd, strDirName, strTemp;
    std::size_t intPos, intDone, intError, intLC, intRand;
    std::map<std::string, std::string>::iterator iprimConf;

    intDone = 0;
    while (intDone != 1) {
        intError = 0;
        strMessage=fGetMessage("/prim_stor");
        strLogMessage = " STOR Got message: " + strMessage;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cout << strLogMessage << std::endl;
        //Parse message
        intPos=strMessage.find_last_of(" ");
        if(intPos != std::string::npos) {
            strRecNum = strMessage.substr(intPos + 1);
            strFullPath = strMessage.substr(0, intPos);
            if(strFullPath.back() != '/') {
                strFullPath += "/";
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIRECTYPE");
            if(iprimConf == conf1.primConf.end()) {
                strLogMessage = " STOR Message " + strMessage + " receiver is not a store process.  Skipping...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                std::cout << strLogMessage << std::endl;
                intError = 1;
            }
        } else {
            strLogMessage = " STOR Message " + strMessage + " does not have a receiver identified.  Skipping";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            std::cout << strLogMessage << std::endl;
            intError = 1;
        }
        strTemp = strFullPath;
        strTemp.pop_back();
        intPos=strTemp.find_last_of("/");
        if(intPos != std::string::npos) {
            strDirName = strTemp.substr(intPos + 1);
        } else {
            intRand = rand() % 10000 + 1;
            strDirName = "Temp" + to_string(intRand);
        }
        strLogMessage = " STOR Creating directory /tmp/" + strDirName;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cout << strLogMessage << std::endl;
        try {
            fs::create_directory("/tmp/" + strDirName);
        }
        catch (fs::filesystem_error& err) {
            strLogMessage = " STOR ERROR: Could not make directory /tmp/" + strDirName + ".  Skipping message...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            std::cout << strLogMessage << std::endl;
            return;
        }
        if(intError == 0) {
            strCmd = "aws s3 ls s3://candh-dicom/" + strFullPath;
            strReturn = exec(strCmd.c_str());
            std::istringstream istr(strReturn);
            intLC = 0;
            while(std::getline(istr, strLine)) {
                //Don't really want to do anything here.
                std::cout << strLine << " " << to_string(intLC) << std::endl;
                if(strLine.length() > 1) {
                    strLine2 = strLine;
                }
                intLC++;
            }
            intPos=strLine2.find_last_of(" ");
            if(intPos != std::string::npos) {
                strFullPath += strLine2.substr(intPos + 1);
                //std::cout << strFullPath << std::endl;
                strCmd = "aws s3 ls s3://candh-dicom/" + strFullPath;
                strReturn = exec(strCmd.c_str());
                std::istringstream istr(strReturn);
                while(std::getline(istr, strLine)) {
                    //std::cout << strLine << std::endl;
                    intPos=strLine.find_last_of(" ");
                    if(intPos != std::string::npos) {
                        strLine2 = strLine.substr(intPos + 1);
                    } else {
                        strLine2 = strLine;
                    }
                    strLogMessage = " STOR Downloading " + strFullPath + "/" + strLine2 + " to /tmp/" + strDirName + "/" + strLine;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    std::cout << strLogMessage << std::endl;
                    strCmd = "aws s3 cp s3://candh-dicom/" + strFullPath + strLine2 + " /tmp/" + strDirName;
                    system(strCmd.c_str());
                    std::cout << strCmd << std::endl;
                    strCmd = "ls -l /tmp/" + strDirName + "/*|wc -l";
                    strReturn=exec(strCmd.c_str());
                    strLogMessage = " STOR Downlaoded " + strReturn + " files.";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    std::cout << strLogMessage << std::endl;
                }
                strCmd = "(cd /tmp/" + strDirName + " && tar -cf " + strDirName + ".tar * )";
                system(strCmd.c_str());
                strCmd = "mv /tmp/" + strDirName + "/" + strDirName + ".tar " + conf1.primConf[strRecNum + "_PRIIF"] + "/" + strDirName + ".tar";
                system(strCmd.c_str());
            } else {
                std::cout << "Didn't find a space..." << std::endl;
            }
        }
        strLogMessage = " STOR Removing directory /tmp/" + strDirName;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cout << strLogMessage << std::endl;
        try {
            fs::remove_all("/tmp/" + strDirName);
        } catch (fs::filesystem_error& err) {
            strLogMessage = " STOR ERROR:  Could not remove directory /tmp/" + strDirName;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            std::cout << strLogMessage << std::endl;
        }
    }
}

int main(int argc, char *argv[]) {
    std::size_t intReturn, intPos, intMaxThreads, intNumThreads, intFoundThread, intDone=0, intArgSet=0;
    std::string strTemp, strTemp2, strCMD, strCmd, strRecNum, strLogMessage, strFilename, strPrimalID, strRawDCMdump, strPName, strMRN;
    std::string strDOB, strSerIUID, strSerDesc, strModality, strSopIUID, strSIUID, strStudyDate, strACCN, strStudyDesc;
    std::string strPatientComments, strTemp3, strQuery, strDBReturn, strStartRec, strResult;
    int intLC;
    std::map<std::string, std::string>::iterator iprimConf;
    std::vector<std::thread> vecThreads;

    mysql_library_init(0, NULL, NULL);
    MYSQL *mconnect;
    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    ReadDBConfFile();
    //mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        cout<<"connection failed\n";
        return 1;
    }
    intMaxThreads = std::thread::hardware_concurrency();
    strLogMessage = "Starting prim_store_server version 1.11.00 with " + to_string(intMaxThreads) + " threads.";
    std::cout << strLogMessage << std::endl;
    conf1.ReadConfFile();
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    intLC=0;
    while (intLC < argc) {
        strTemp=argv[intLC];
        if (strTemp == "-ss") {
            intArgSet=1;
            intLC++;
            strRecNum=argv[intLC];
        } else {
            intLC++;
        }
    }
    if(intArgSet==0) {
        std::cerr << "ERROR:  Could not find receiver name in list of arguments.  Exiting..." << std::endl;
        return 1;
    }
    intReturn = fStartupCheck(strRecNum);
    if(intReturn > 0) {
        return intReturn;
    }
    strLogMessage = "Launching message queue thread";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    std::cout << strLogMessage << std::endl;
    std::thread thMessage(fProcMessage);
    strLogMessage = "Checking directory " + conf1.primConf[strRecNum + "_PRIIF"];
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    std::cout << strLogMessage << std::endl;
    while(intDone != 1) {
        for (const auto & entry : fs::directory_iterator(conf1.primConf[strRecNum + "_PRIIF"])) {
            strTemp=entry.path().string();
            //strLogMessage = "Found file " + strTemp;
            //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
            //std::cout << strLogMessage << std::endl;
            intPos = strTemp.find_last_of("/");
            strFilename=strTemp.substr(intPos+1);
            intPos=strFilename.find_last_of(".");
            if(intPos != std::string::npos) {
                strTemp2=strFilename.substr(intPos);
            } else {
                strTemp2 = "N/A";
            }
            if(strTemp2 == ".gz" || strTemp2 == ".tar") {
                if(vecThreads.size() < intMaxThreads) {
                    vecThreads.emplace_back(fProcFile, strTemp, strRecNum);
                    intNumThreads++;
                    strLogMessage = " STOR Started processing: " + strTemp;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                    std::cout << strLogMessage << std::endl;
                } else {
                    intFoundThread=0;
                    while(intFoundThread == 0) {
                        for (long unsigned int i=0; i<vecThreads.size(); i++) {
                            if(vecThreads[i].joinable()) {
                                vecThreads[i].join();
                                std::thread th(fProcFile, strTemp, strRecNum);
                                vecThreads[i] = std::move(th);
                                strLogMessage = " STOR Started Processing: " + strTemp;
                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
                                intFoundThread=1;
                                break;
                            }
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        std::this_thread::sleep_for (std::chrono::seconds(10));
    }
    mysql_close(mconnect);
    mysql_library_end();
    return 0;
}