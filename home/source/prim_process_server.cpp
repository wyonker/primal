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
#include <exception>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <openssl/md5.h>
#include <sys/stat.h>

std::mutex mtx;
using namespace std;
namespace fs = std::filesystem;
std::vector<std::string > vecRCcon1;
std::vector<std::string > vecRCopt1;
std::vector<std::string > vecRCcon2;
std::vector<std::string > vecRCact1;

const std::string strProcChainType = "PRIMRCPROC";

#include "prim_functions.h"

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

std::string fGetMD5(std::string strFullFilename) {
    ifstream::pos_type fileSize;
    std::string strReturn, strTemp;
    std::ifstream file(strFullFilename, std::ifstream::binary);
    MD5_CTX md5Context;
    unsigned char result[MD5_DIGEST_LENGTH];
    char buf[1024 * 16];
    std::stringstream md5string;
    std::locale loc;

    MD5_Init(&md5Context);
    while (file.good()) {
        file.read(buf, sizeof(buf));
        MD5_Update(&md5Context, buf, file.gcount());
    }
    MD5_Final(result, &md5Context);
    md5string << std::hex << std::uppercase << std::setfill('0');
    for (const auto &byte: result)
        md5string << std::setw(2) << (int)byte;

    strTemp = md5string.str();
    for (std::string::size_type i=0; i<strTemp.length(); ++i)
        strReturn += std::tolower(strTemp[i],loc);

    return strReturn;
}

std::string fGetClientID(std::string strPrimalID) {
    std::string strQuery, strReturn, strLogMessage, strRecNum;

    strRecNum = "1";
    MYSQL *mconnect2;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed IN fGetClientID.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "-1";
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="MySQL connection failed in fGetClientID.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "-1";
    }
    MYSQL_ROW row;
    MYSQL_RES *result = mysql_store_result(mconnect2);

    strQuery = "select sClientID from study where puid = '" + strPrimalID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        strLogMessage+="strQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        row = mysql_fetch_row(result);
        strReturn=row[0];
        mysql_free_result(result);
    } else {
        strReturn = "NULL";
    }
    mysql_close(mconnect2);
    return strReturn;
}

std::string fGetClientName(std::string strPrimalID) {
    std::string strQuery, strReturn, strRecNum, strLogMessage;

    strRecNum = "1";
    MYSQL *mconnect2;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fGetClientName.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "-1";
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="MySQL connection failed in fGetClientName";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "-1";
    }
    MYSQL_ROW row;
    MYSQL_RES *result = mysql_store_result(mconnect2);

    strQuery = "select sClientName from study where puid = '" + strPrimalID + "' limit 1;";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        strLogMessage+="strQuery = " + strQuery + ".";
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        row = mysql_fetch_row(result);
        strReturn=row[0];
        mysql_free_result(result);
    } else {
        strReturn = "NULL";
    }
    mysql_close(mconnect2);
    return strReturn;
}

std::string fGetCallingAET(std::string strPrimalID) {
    std::string strQuery, strReturn, strRecNum, strLogMessage;

    strRecNum = "1";
    MYSQL *mconnect2;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fGetCallingAET.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "-1";
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="MySQL connection failed in fGetCallingAET.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "-1";
    }
    MYSQL_ROW row;
    MYSQL_RES *result = mysql_store_result(mconnect2);

    strQuery = "select callingAET from receive where puid = '" + strPrimalID + "' limit 1;";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        strLogMessage="strQuery = " + strQuery + ".";
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        row = mysql_fetch_row(result);
        strReturn=row[0];
        mysql_free_result(result);
    } else {
        strReturn = "NULL";
    }
    mysql_close(mconnect2);
    return strReturn;
}

std::string fAPICall(std::string strJson, std::string strClientID, std::string strSIUID, std::string strPrimalID, std::string strCaseID){
    std::string strCMD, strReturn, strQuery, strReadLine, strLogMessage, strRecNum, strPrefetch, strPrefetchNode, strClientAET;
    std::string strClientName, strTempVar, strcallingAET, strLine;
    std::size_t intPos, intFound, intFound2, intFound3, intNumRows;
    (void) strCaseID;
    MYSQL *mconnect2;
    MYSQL_ROW row;

    strRecNum = "1";
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fAPICall.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "1";
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed\n";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "1";
    }
    MYSQL_RES *result;

    intPos = strPrimalID.find("_");
    if(intPos != std::string::npos) {
        strRecNum=strPrimalID.substr(0,intPos);
    } else {
        strRecNum = "1";
    }
    strJson.erase(std::remove(strJson.begin(), strJson.end(), '\\'), strJson.end());
    strQuery = "select senderAET from receive where puid = '" + strPrimalID + "';";
    //std::cout << strQuery << std::endl;
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        intNumRows = mysql_num_rows(result);
        if(intNumRows > 0) {
            row = mysql_fetch_row(result);
            strcallingAET = row[0];
        } else {
            strcallingAET = "DEFAULT";
        }
        mysql_free_result(result);
    } else {
        strcallingAET = "DEFAULT";
    }
    strLogMessage = strPrimalID + " PROC " + "Set callingAET to: " + strcallingAET + " in fAPICall.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    //std::cout << strLogMessage << std::endl;
    strPrefetchNode = "0";
    if(fs::exists("/etc/primal/prim_ae_map.conf")) {
        std::ifstream fpAEMap("/etc/primal/prim_ae_map.conf");
        while (std::getline(fpAEMap, strLine)) {
            intFound=strLine.find(",");
            intFound2=strLine.find(",", intFound + 1);
            intFound3=strLine.find(",", intFound2 + 1);
            strClientID = strLine.substr(0, intFound);
            strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
            strClientName = strLine.substr(intFound2 + 1, intFound3 - intFound2 -1);
            strTempVar = strLine.substr(intFound3 + 1);
            intFound2=strcallingAET.find(strClientAET);
            if(intFound2 != std::string::npos) {
                strPrefetchNode = strTempVar;
                strLogMessage = "Found callingAET " + strcallingAET + " setting prefetch to " + strPrefetchNode;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                //std::cout << strLogMessage << std::endl;
            }
        }
        fpAEMap.close();
    }
    intPos=strJson.find("dcm_raw");
    if(intPos != std::string::npos) {
        strJson.erase(0, intPos + 10);
    }
    intPos=strJson.find("[]}\"}");
    if(intPos != std::string::npos) {
        strJson.replace(intPos, 5, "[]}");
    }
    if(strPrefetchNode.compare("1")==0) {
        strPrefetch = "yes";
    } else {
        strPrefetch = "no";
    }
    strCMD = "curl -s --location --request POST 'https://sheridan.candescenthealth.com/api/v1/gateway/create-case' ";
    strCMD += "--form 'challenge=94dfcb464afd2725297f2bbcc384f57a' ";
    strCMD += "--form 'clients_id=" + strClientID + "' ";
    strCMD += "--form 'client_locations_id=0' ";
    strCMD += "--form 'image_count=100' ";
    strCMD += "--form 'prefetching=" + strPrefetch + "' ";
    strCMD += "--form 'parent_id=0' ";
    strCMD += "--form 'rc_ident=b624167640a4561f9d0ea371aa9614cb' ";
    strCMD += "--form 'study_instance_uid=" + strSIUID + "' ";
    strCMD += "--form 'is_archive_case=" + strPrefetch + "' ";
    strCMD += "--form 'dcm_raw=" + strJson + "'";
    //redi::ipstream proc(strCMD, redi::pstreams::pstdout);
    //std::getline(proc, strReturn);
    //while (std::getline(proc, strReadLine))
    //    strReturn.append(strReadLine + "\n");
    strLogMessage=strCMD + "\n\n";
    strReturn = exec(strCMD.c_str());
    strLogMessage+="create-case returned:\n";
    strLogMessage+=strReturn;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);

    intPos = strReturn.find("ERROR");
    if(intPos != std::string::npos) {
        std::cout << strReturn << std::endl;
        strQuery = "update study set sCaseID = '0' where PUID='" + strPrimalID + "';";
        strLogMessage = strPrimalID + " PROC " + "Did NOT Get CaseID.  Setting to 0.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        //std::cout << strLogMessage << std::endl;
    } else {
        strQuery = "update study set sCaseID = '" + strReturn + "' where PUID='" + strPrimalID + "';";
        strLogMessage = strPrimalID + " PROC " + "Got CaseID " + strReturn;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        //std::cout << strLogMessage << std::endl;
    }
    mysql_query(mconnect2, strQuery.c_str());
    mysql_close(mconnect2);
    return strReturn;
}

void fCreateJson(std::string strFullPath, std::size_t intMsgType) {
    std::string strPrimalID, strQuery, strRow, strTemp, strJson, strSerIUID, strSerTemp, strTemp2, strFilename, strOldVal;
    std::string strDate, strRecNum, strPrimalIDInt, strTemp3, strLogMessage, strSIUID, strAEC, strPackFileName, strCaseID;
    std::string strLogFile, strClientID, strClientAET, strClientName, strTempPath, strPrefetch, strTempVar, strPrefetchNode;
    std::string strcallingAET, strLine, strParentCaseID;
    std::size_t intImgNum=0, intTemp, intPos, intDirExists=0, intFound, intPrimalIDInt, intNumRows, intFound2, intFound3;
    std::size_t intPrefetchIsSet;
    std::map<std::string, std::string> mapSeriesJson;
    std::map<std::string, std::string>::iterator iprimConf;
    struct stat st;
    MYSQL *mconnect2;
    MYSQL_ROW row;

    strRecNum = "1";
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fCreateJson.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return;
    }

    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="MySQL connection failed in fCreateJson.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return;
    }
    MYSQL_RES *result;

    //See if the directory exists
    if(stat(strFullPath.c_str(),&st) == 0) {
        if(S_ISDIR(st.st_mode)) {
            intDirExists=1;
        } else {
            intDirExists=0;
        }
    } else {
        intDirExists=0;
    }
    if(intDirExists != 1) {
        //Going to need to handle this
        return;
    }
    intImgNum=0;
    intFound = strFullPath.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strFullPath.substr(intFound + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);
    strPrimalIDInt=strPrimalID;
    strPrimalIDInt.erase(std::remove(strPrimalIDInt.begin(), strPrimalIDInt.end(), '_'), strPrimalIDInt.end());
    strLogMessage = strPrimalID + " PROC " + "Creating JSON file.";
    strQuery="select senderAET from receive where puid = '" + strPrimalID + "';";
    mtx.lock();
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        row = mysql_fetch_row(result);
        strcallingAET=row[0];
        mysql_free_result(result);
    } else {
        strcallingAET="DEFAULT";
    }
    mtx.unlock();
    strLogMessage=strPrimalID + " PROC " + " callingAET = " + strcallingAET;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);

    strPrefetchNode="0";
    intPrefetchIsSet=0;
    if(fs::exists("/etc/primal/prim_ae_map.conf")) {
        std::ifstream fpAEMap("/etc/primal/prim_ae_map.conf");
        while (std::getline(fpAEMap, strLine)) {
            if(strLine.find(strcallingAET)) {
                intFound=strLine.find(",");
                intFound2=strLine.find(",", intFound + 1);
                intFound3=strLine.find(",", intFound2 + 1);
                strClientID = strLine.substr(0, intFound);
                strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
                strClientName = strLine.substr(intFound2 + 1, intFound3 - intFound2 -1);
                strTempVar = strLine.substr(intFound3 + 1);
                intFound2=strcallingAET.find(strClientAET);
                if(intFound2 != std::string::npos) {
                    strPrefetchNode = strTempVar;
                    intPrefetchIsSet=1;
                    strLogMessage = strPrimalID + " PROC " + "Found callingAET " + strcallingAET + " setting prefetch to " + strPrefetchNode;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                }
            }
        }
        fpAEMap.close();
    }
    if(intPrefetchIsSet==0) {
        strLogMessage = strPrimalID + " PROC " + "callingAET not found for " + strcallingAET + " setting prefetch to " + strPrefetchNode;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    if(strPrefetchNode.compare("1")==0) {
        strPrefetch = "yes";
    } else {
        strPrefetch = "no";
    }
    for (const auto & entry : fs::directory_iterator(strFullPath)) {
        //strLogMessage = "Processing " + entry.path().string();
        //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        strTemp2=entry.path().string();
        intTemp = strTemp2.find_last_of("/");
        strFilename=strTemp2.substr(intTemp+1);
        intImgNum++;
        intPos=strFilename.find_last_of(".");
        if(intPos != std::string::npos) {
            strTemp3=strFilename.substr(intPos);
        }
        if(strTemp3 == ".dcm") {
            //strLogMessage = "Getting tags from " + entry.path().string();
            //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILFPROC"]);
            strTemp=fDcmDump(entry.path().string());
            if(intImgNum == 1) {
                strQuery="select DicomCasesID, sCaseID from study where puid ='" + strPrimalID + "';";
                //std::string strCMD = "echo \"" + strQuery + "\"|mysql -u root primal";
                //std::string strReturn = exec(strCMD.c_str());
                //std::cout << strReturn << std::endl;
                //std::cout << "strQuery" << std::endl;
                strLogMessage=strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                mysql_query(mconnect2, strQuery.c_str());
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    strLogMessage+="strQuery = " + strQuery + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                }
                mtx.lock();
                result = mysql_store_result(mconnect2);
                if(result) {
                    intNumRows = mysql_num_rows(result);
                    if(intNumRows > 0) {
                        row = mysql_fetch_row(result);
                        strPrimalIDInt = row[0];
                        if(row[1] == NULL) {
                            strCaseID = "0";
                        } else {
                            strCaseID = row[1];
                        }
                    }
                    mysql_free_result(result);
                }
                mtx.unlock();
                strLogMessage="strCaseID: " + strCaseID + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                intPrimalIDInt=stoi(strPrimalIDInt);
                strClientID=fGetClientID(strPrimalID);
                //intFound=strTemp3.find(",");
                //intFound2=strTemp3.find(",", intFound + 1);
                //strClientID = strTemp3.substr(0, intFound);
                strClientAET = fGetCallingAET(strPrimalID);
                strClientName = fGetClientName(strPrimalID);
                strSIUID=fGetTagValue("0020,000d", strTemp, 0);
                if(strPrefetchNode.compare("1")==0) {
                    mtx.lock();
                    strQuery="select study.sCaseID from QR join study on QR.puid=study.puid where QR.SIUID='" + strSIUID + "' and QR.qrstatus='Requested';";
                    strLogMessage=strQuery + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                    mysql_query(mconnect2, strQuery.c_str());
                    if(*mysql_error(mconnect2)) {
                        strLogMessage=mysql_error(mconnect2);
                        strLogMessage+="\n" + strQuery + ".";
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                    }
                    result = mysql_store_result(mconnect2);
                    if(result) {
                        intNumRows = mysql_num_rows(result);
                        if(intNumRows > 0) {
                            row = mysql_fetch_row(result);
                            strParentCaseID=row[0];
                        } else {
                            strParentCaseID="0";
                        }
                        mysql_free_result(result);
                    } else {
                        strParentCaseID="0";
                    }
                    mtx.unlock();
                    strLogMessage="Query: " + strQuery + "\n";
                    strLogMessage+="Parent CaseID: " + strParentCaseID;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                }

                strJson = "{\"challenge\":\"94dfcb464afd2725297f2bbcc384f57a\",";
                strJson += "\"clients_id\":\"" + strClientID + "\",";
                strJson += "\"client_locations_id\":\"0\",";
                strJson += "\"image_count\":\"1\",";
                strJson += "\"prefetching\":\"" + strPrefetch + "\",";
                strJson += "\"parent_id\":\"" + strParentCaseID + "\",";
                strJson += "\"rc_ident\":\"b624167640a4561f9d0ea371aa9614cb\",";
                strJson += "\"study_instance_uid\":\"" + strSIUID + "\",";
                strJson += "\"is_archive_case\":\"" + strPrefetch + "\",";
                strJson += "\"dcm_raw\":\"{";
                strJson += "\\\"dicom_cases\\\":{";
                strJson += "\\\"dicom_cases_id\\\":\\\"" + strPrimalIDInt + "\\\",";
                strJson += "\\\"remote_dicom_cases_id\\\":\\\"0\\\",";
                strJson += "\\\"cases_id\\\":\\\"0\\\",";
                strJson += "\\\"initial_cases_id\\\":\\\"0\\\",";
                strJson += "\\\"status\\\":\\\"Processing\\\",";
                strJson += "\\\"remote_case_status\\\":\\\"Processed\\\",";
                strJson += "\\\"status_problem\\\":\\\"None\\\",";
                strJson += "\\\"remote_title\\\":\\\"" + strcallingAET + "\\\",";
                strJson += "\\\"local_title\\\":\\\"RCONN_" + strClientID + "\\\",";
                strJson += "\\\"num_studies\\\":\\\"1\\\",";
                strJson += "\\\"num_series\\\":\\\"1\\\",";
                strJson += "\\\"num_images\\\":\\\"" + to_string(intImgNum) + "\\\",";
                strDate = GetDate();
                strJson += "\\\"date_created\\\":\\\"" + strDate + "\\\",";
                strJson += "\\\"date_receive_started\\\":\\\"" + strDate + "\\\",";
                strJson += "\\\"date_receive_ended\\\":\\\"" + strDate + "\\\",";
                strJson += "\\\"date_process_start\\\":\\\"0000-00-00 00:00:00\\\",";
                strJson += "\\\"date_process_end\\\":\\\"0000-00-00 00:00:00\\\",";
                strJson += "\\\"date_outgoing_queued\\\":\\\"0000-00-00 00:00:00\\\",";
                strJson += "\\\"date_modified\\\":\\\"" + strDate + "\\\",";
                strJson += "\\\"storage_commitment\\\":\\\"2020-03-29 13:06:30\\\",";
                strJson += "\\\"parent_id\\\":null,";
                strJson += "\\\"send_hash\\\":null,";
                strTemp2 = fGetMD5(entry.path().string());
                strJson += "\\\"rc_ident\\\":\\\"" + strTemp2 + "\\\",";
                strJson += "\\\"registry_id\\\":\\\"7700\\\",";
                if(strPrefetchNode.compare("1")!=0) {
                    strJson += "\\\"flags\\\":\\\"UNASSIGNED, PRIORS_REQUESTED\\\",";
                } else {
                    strJson += "\\\"flags\\\":\\\"UNASSIGNED\\\",";
                }
                strJson += "\\\"priority\\\":\\\"5\\\",";
                strJson += "\\\"split_parent\\\":null,";
                strJson += "\\\"thread_id\\\":\\\"Thread-196721\\\"";
                strJson += "},";
                strJson += "\\\"dicom_outgoing\\\":[";
                strJson += "],";
                strJson += "\\\"application_entity\\\":{";
                strJson += "\\\"application_entity_id\\\":\\\"999\\\",";
                strJson += "\\\"name\\\":\\\"" + strClientName + "\\\",";
                strJson += "\\\"entity_type\\\":\\\"PACS\\\",";
                iprimConf = conf1.primConf.find(strRecNum + "_PRIAET");
                if(iprimConf != conf1.primConf.end()) {
                    strTemp2 = conf1.primConf.find(strRecNum + "_PRIAET")->second;
                }
                strJson += "\\\"remote_title\\\":\\\"" + strTemp2 + "\\\",";
                strJson += "\\\"local_ae_id\\\":\\\"575\\\",";
                strJson += "\\\"host\\\":\\\"0.0.0.0\\\",";
                strJson += "\\\"port\\\":\\\"1040\\\",";
                strJson += "\\\"cstore_decompression\\\":\\\"No\\\",";
                strJson += "\\\"allow_anonymous_connections\\\":\\\"No\\\",";
                strJson += "\\\"active_associations\\\":\\\"0\\\",";
                strJson += "\\\"max_concurrent_associations\\\":\\\"20\\\",";
                strJson += "\\\"orb_status\\\":\\\"Inactive\\\",";
                strJson += "\\\"orb_last_checkin\\\":\\\"0000-00-00 00:00:00\\\",";
                strJson += "\\\"dicom_status\\\":\\\"Unavailable\\\",";
                strJson += "\\\"status\\\":\\\"Enabled\\\",";
                strJson += "\\\"split_studies\\\":\\\"Yes\\\",";
                strJson += "\\\"accession_merge\\\":\\\"No\\\",";
                strJson += "\\\"query_root\\\":\\\"STUDY\\\",";
                strJson += "\\\"query_level\\\":\\\"STUDY\\\",";
                strJson += "\\\"archive_system\\\":\\\"No\\\",";
                strJson += "\\\"client_configuration_id\\\":\\\"569\\\",";
                strJson += "\\\"registry_id\\\":\\\"0\\\",";
                strJson += "\\\"storage_commitment\\\":\\\"345600\\\",";
                strJson += "\\\"comments\\\":null,";
                strJson += "\\\"local_title\\\":\\\"RCONN_1101\\\"";
                strJson += "},";
                strJson += "\\\"patient_level\\\":";
                strJson += "{";
                strJson += "\\\"patient_level_id\\\":\\\"" + strPrimalIDInt + "\\\",";
                strJson += "\\\"dicom_cases_id\\\":\\\"" + strPrimalIDInt + "\\\",";
                strTemp2=fGetTagValue("0010,0010", strTemp, 0);
                strJson += "\\\"patient_name\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0010,0020", strTemp, 0);
                strJson += "\\\"patient_id\\\":\\\"" + strTemp2 + "\\\",";
                strJson += "\\\"alt_patient_id\\\":null,";
                strTemp2=fGetTagValue("0010,0030", strTemp, 0);
                strJson += "\\\"patient_birth_date\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0010,0032", strTemp, 0);
                strJson += "\\\"patient_birth_time\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0010,0040", strTemp, 0);
                strJson += "\\\"patient_sex\\\":\\\"" + strTemp2 + "\\\",";
                strJson += "\\\"patient_age\\\":null,";
                strJson += "\\\"patient_size\\\":null,";
                strTemp2=fGetTagValue("0010,1030", strTemp, 0);
                //strJson += "\\\"patient_weight\\\":" + strTemp2 + ",";
                strJson += "\\\"patient_weight\\\":0,";
                strJson += "\\\"medical_record_locator\\\":null,";
                strJson += "\\\"ethnic_group\\\":null,";
                strJson += "\\\"patient_address\\\":null,";
                strJson += "\\\"patient_phone\\\":null,";
                strJson += "\\\"patient_comments\\\":null,";
                strJson += "\\\"patient_history\\\":null,";
                strJson += "\\\"occupation\\\":null,";
                strJson += "\\\"current_patient_location\\\":null,";
                strJson += "\\\"patient_ins_residence\\\":null,";
                strJson += "\\\"date_created\\\":\\\"" + strDate + "\\\"},";
                strJson += "\\\"study_level\\\":{\\\"" + strPrimalIDInt + "\\\":{";
                strJson += "\\\"study_level_id\\\":\\\"" + strPrimalIDInt + "\\\",";
                strJson += "\\\"dicom_cases_id\\\":\\\"" + strPrimalIDInt + "\\\",";
                strJson += "\\\"patient_level_id\\\":\\\"" + strPrimalIDInt + "\\\",";
                strTemp2=fGetTagValue("0008,0020", strTemp, 0);
                strJson += "\\\"study_date\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0008,0030", strTemp, 0);
                strJson += "\\\"study_time\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0008,0050", strTemp, 0);
                strJson += "\\\"accession_number\\\":\\\"" + strTemp2 + "\\\",";
                strJson += "\\\"study_id\\\":\\\"0\\\",";
                strJson += "\\\"study_instance_uid\\\":\\\"" + strSIUID + "\\\",";
                strTemp2=fGetTagValue("0008,0090", strTemp, 0);
                strJson += "\\\"referring_physician\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0008,1010", strTemp, 0);
                strJson += "\\\"station_name\\\":\\\"" + strTemp2 + "\\\",";
                strJson += "\\\"perf_phy_name\\\":null,";
                strJson += "\\\"name_phy_read_stdy\\\":null,";
                strTemp2=fGetTagValue("0008,1030", strTemp, 0);
                strJson += "\\\"study_description\\\":\\\"" + strTemp2 + "\\\",";
                strJson += "\\\"study_comments\\\":null,";
                strJson += "\\\"requested_procedure_des\\\":null,";
                strJson += "\\\"admit_diag_des\\\":null,";
                strTemp2=fGetTagValue("0008,0080", strTemp, 0);
                strJson += "\\\"institution_name\\\":\\\"" + strTemp2 + "\\\",";
                strJson += "\\\"performed_station\\\":null,";
                strJson += "\\\"requested_priority\\\":null,";
                strJson += "\\\"performed_type_description\\\":null,";
                strJson += "\\\"recognition_code\\\":null,";
                strJson += "\\\"date_created\\\":\\\"" + strDate + "\\\",";
                strTemp2=fGetTagValue("0040,0254", strTemp, 0);
                strJson += "\\\"performed_step_description\\\":null";
                strJson += "}},";
                strJson += "\\\"series_level\\\":{";
            }
            //First let's see if the series for this instances has already been seen
            strSerIUID = fGetTagValue("0020,000e", strTemp, 0);
            std::map<std::string, std::string>::const_iterator got = mapSeriesJson.find (strSerIUID);
            if (got == mapSeriesJson.end()) {
                //Not found.  Need to add.
                intPrimalIDInt++;
                strSerTemp.append("\\\"" + to_string(intPrimalIDInt) + "\\\":{");
                strSerTemp += "\\\"series_level_id\\\":\\\"" + to_string(intPrimalIDInt) + "\\\",";
                strSerTemp += "\\\"dicom_cases_id\\\":\\\"" + strPrimalIDInt + "\\\",";
                strSerTemp += "\\\"study_level_id\\\":\\\"" + strPrimalIDInt + "\\\",";
                strTemp2=fGetTagValue("0008,0021", strTemp, 0);
                strSerTemp += "\\\"series_date\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0008,0031", strTemp, 0);
                strSerTemp += "\\\"series_time\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0008,0060", strTemp, 0);
                strSerTemp += "\\\"modality\\\":\\\"" + strTemp2 + "\\\",";
                strSerTemp += "\\\"conversion_type\\\":\\\"WSD\\\",";
                strTemp2=fGetTagValue("0020,0011", strTemp, 0);
                strSerTemp += "\\\"series_number\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0020,000e", strTemp, 0);
                strSerTemp += "\\\"series_instance_uid\\\":\\\"" + strTemp2 + "\\\",";
                strSerTemp += "\\\"protocol_name\\\":null,";
                strTemp2=fGetTagValue("0008,103e", strTemp, 0);
                strSerTemp += "\\\"series_description\\\":\\\"" + strTemp2 + "\\\",";
                strTemp2=fGetTagValue("0018,0015", strTemp, 0);
                strSerTemp += "\\\"body_part_examined\\\":\\\"" + strTemp2 + "\\\",";
                strSerTemp += "\\\"view_position\\\":null,";
                strSerTemp += "\\\"comments\\\":null,";
                strSerTemp += "\\\"magnetic_field_strength\\\":null,";
                strSerTemp += "\\\"contrast_bolus_route\\\":null,";
                strSerTemp += "\\\"contrast_bolus_agent\\\":null,";
                strSerTemp += "\\\"contrast_bolus_volume\\\":null,";
                strSerTemp += "\\\"angio_flag\\\":null,";
                strSerTemp += "\\\"spacing_between_slices\\\":null,";
                strSerTemp += "\\\"patient_position\\\":null,";
                strSerTemp += "\\\"reconstruction_diameter\\\":null,";
                strSerTemp += "\\\"heart_rate\\\":null,";
                strSerTemp += "\\\"device_manufacturer\\\":null,";
                strSerTemp += "\\\"device_model_name\\\":null,";
                strSerTemp += "\\\"pulse_sequence_name\\\":null,";
                strSerTemp += "\\\"ctdivol\\\":null,";
                strSerTemp += "\\\"dlp\\\":null,";
                strSerTemp += "\\\"date_created\\\":\\\"" + strDate + "\\\",";
                strSerTemp += "\\\"laterality\\\":null,";
                strSerTemp += "\\\"fluoro_timing\\\":null";
                strSerTemp += "},";
            } else {
                strSerTemp.append(mapSeriesJson.find (strSerIUID)->second);
                //strSerTemp.append("            },\n");
                //strSerTemp.append("            {\n");
            }
            mapSeriesJson.erase(strSerIUID);
            //std::cout << strSerTemp << std::endl;
            std::pair<std::map<std::string, std::string>::iterator,bool> ret;
            ret = mapSeriesJson.insert(std::pair<std::string, std::string>(strSerIUID, strSerTemp) );
            strSerTemp.clear();
            strSerIUID.clear();
        }
    }
    for( auto const& [key, val] : mapSeriesJson ) {
        //std::cout << val << std::endl;
        strJson.append(val);
    }
    strJson.pop_back();
    strJson += "},";
    //Adding instance level
    strJson.append("\\\"instance_level\\\":[]}\"}");
    strLogMessage = strPrimalID + " PROC " + " Writing JSON to " + strFullPath + "/payload.json";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    ofstream fdJson(strFullPath + "/payload.json", std::ofstream::trunc);
    mapSeriesJson.clear();
    if(fdJson.is_open()) {
        fdJson << strJson;
        fdJson.close();
        if (intMsgType == 1) {
            fAPICall(strJson, strClientID, strSIUID, strPrimalID, strCaseID);
        }
    } else {
        strLogMessage = strPrimalID + " PROC " + "Unable to open file " + strFullPath + "/" + "payload.json";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    if(intMsgType == 1) {
        strTempPath="/tmp/" + strPrimalID;
        std::ofstream output(strTempPath);
    }
    mysql_close(mconnect2);
    return;
}

std::size_t fCreatePkg(std::string strFullPath, std::size_t intMsgType, std::string strPrefetchNode) {
    std::string strPrimalID, strQuery, strRow, strTemp, strJson, strSerIUID, strSerTemp, strTemp2, strFilename, strOldVal;
    std::string strDate, strRecNum, strPrimalIDInt, strTemp3, strLogMessage, strSIUID, strAEC, strPackFileName, strCaseID;
    std::string strClientID, strClientName, strClientAET, strDicomCaseID, strParentCaseID;
    std::size_t intTemp, intPos, intFound, intDirExists;
    std::map<std::string, std::string> mapSeriesJson;
    MYSQL_ROW row;
    std::map<std::string, std::string>::iterator iprimConf;
    struct stat st;
    (void) intMsgType;

    strRecNum = "1";
    MYSQL *mconnect2;
    MYSQL_RES *result;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fCreatePkg.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return 1;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="MySQL connection failed in fCreatePkg.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return 1;
    }
    intFound = strFullPath.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strFullPath.substr(intFound + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);

    strLogMessage = strPrimalID + " PROC Begin creating JSON file." + strPrefetchNode;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    strQuery="select SIUID from study where puid='" + strPrimalID + "';";
    mtx.lock();
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        strLogMessage+="\nstrQuery = " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        row = mysql_fetch_row(result);
        strSIUID=row[0];
        mysql_free_result(result);
    } else {
        strSIUID="0";
    }
    strLogMessage = strPrimalID + " PROC SIUID = " + strSIUID;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    //strJson="First PKG File From DICOM Payload for Study UUID " +  strSIUID + "\n";
    strJson="[client]\n";
    /*
    strQuery="select senderAET from receive where puid='" + strPrimalID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        strLogMessage+="\nstrQuery = " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        row = mysql_fetch_row(result);
        strTemp=row[0];
        mysql_free_result(result);
    } else {
        strTemp="UNK";
    }
    */
    if(strPrefetchNode.compare("1")==0) {
        strQuery="select study.sCaseID from QR join study on QR.puid=study.puid where QR.SIUID='" + strSIUID + "' and QR.qrstatus='Requested';";
        mysql_query(mconnect2, strQuery.c_str());
        if(*mysql_error(mconnect2)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect2);
            strLogMessage+="\nstrQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        }
        result = mysql_store_result(mconnect2);
        if(result) {
            row = mysql_fetch_row(result);
            strParentCaseID=row[0];
            mysql_free_result(result);
        } else {
            strParentCaseID="0";
        }
        strLogMessage="Query: " + strQuery + "\n";
        strLogMessage+="Parent CaseID: " + strParentCaseID;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    strQuery="select DicomCasesID from study where puid='" + strPrimalID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        strLogMessage+="strQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        if((row = mysql_fetch_row(result)) != NULL) {
            if(row[0] != NULL) {
                strDicomCaseID=row[0];
            } else {
                strDicomCaseID="0";
            }
        } else {
            strDicomCaseID="0";
        }
        mysql_free_result(result);
    } else {
        strDicomCaseID="0";
    }
    strQuery="select sCaseID from study where puid='" + strPrimalID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        strLogMessage+="strQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        if((row = mysql_fetch_row(result)) != NULL) {
            if(row[0] != NULL) {
                strCaseID=row[0];
            } else {
                strCaseID="0";
            }
        } else {
            strCaseID="0";
        }
        mysql_free_result(result);
    } else {
        strCaseID="0";
    }
    strQuery="select sClientID from study where puid='" + strPrimalID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        strLogMessage+="strQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        if((row = mysql_fetch_row(result)) != NULL) {
            if(row[0] != NULL) {
                strTemp=row[0];
            } else {
                strTemp="0";
            }
        } else {
            strTemp="0";
        }
        mysql_free_result(result);
    } else {
        strTemp="0";
    }
    strLogMessage = strPrimalID + " PROC DicomCaseID = " + strDicomCaseID + " CaseID = " + strCaseID + " ClientID = " + strTemp;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    mtx.unlock();
    strClientID=fGetClientID(strPrimalID);
    strClientAET = fGetCallingAET(strPrimalID);
    strClientName = fGetClientName(strPrimalID);
    strLogMessage = strPrimalID + " PROC ClientID = " + strClientID + " ClientAET = " + strClientAET + " ClientName = " + strClientName;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    strJson+="ID = " + strClientID + "\n";
    strJson+="LOCATIONS_ID = 0\n";
    strJson+="NAME = " + strClientName + "\n";
    strJson+="FACILITY = N/A\n";
    strJson+="\n";
    strJson+="[install]\n";
    strPackFileName = strFullPath + "/" + strPrimalID + ".tar.gz";
    strJson+="MD5HASH = " + fGetMD5(strPackFileName) + "\n";
    strJson+="PREINSTALL = \n";
    strJson+="POSTINSTALL = \n";
    strJson+="COMPRESSION = 1\n";
    strJson+="PACKAGEFILE = _DICOM.tar.gz\n";
    //Will need to put the Connect Case ID here.
    if(strPrefetchNode.compare("1")==0) {
        strJson+="CASEID = " + strParentCaseID + "\n";
    } else {
        strJson+="CASEID = " + strCaseID + "\n";
    }
    strJson+="DICOMCASEID = \n";
    strJson+="\n";
    strJson+="[dicom]\n";
    strJson+="TRANSFER_SYNTAX = 1.2.840.10008.1.2\n";
    strJson+="CALLING_AE = RCONN\n";
    strJson+="CALLED_AE = " + strClientAET + "\n";
    strJson+="\n";
    strJson+="[replace.create]\n";
    strJson+="17AD,1000 = Radisphere National Radiology Group\n";
    strJson+="17AD,1001 = rConnect v1.4.1\n";
    strJson+="17AD,1002 = \n";
    strJson+="17AD,1003 = 0\n";
    strJson+="17AD,1004 = N/A\n";
    strJson+="17AD,1005 = " + strTemp + "\n";
    strJson+= "\n";
    strJson+="[replace.modify]\n";
    strJson+="\n";
    strJson+="[files]\n";
    //See if the directory still exists
    if(stat(strFullPath.c_str(),&st) == 0) {
        if(S_ISDIR(st.st_mode)) {
            intDirExists=1;
        } else {
            intDirExists=0;
        }
    } else {
        intDirExists=0;
    }
    if(intDirExists != 1) {
        //Going to need to handle this
        strLogMessage="ERROR:  Directory " + strFullPath + " disappeared before the package was created...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        mysql_close(mconnect2);
        return 1;
    }
    for (const auto & entry : fs::directory_iterator(strFullPath)) {
        strTemp2=entry.path().string();
        intTemp = strTemp2.find_last_of("/");
        strFilename=strTemp2.substr(intTemp+1);
        intPos=strFilename.find_last_of(".");
        if(intPos != std::string::npos) {
            strTemp3=strFilename.substr(intPos);
        } else {
            strTemp3="";
        }
        if(strTemp3 == ".dcm") {
            strJson+="FILE = ./" + strPrimalID + "/" + strFilename + ":" + fGetMD5(entry.path().string()) + "\n";
        }
    }
    strLogMessage = strPrimalID + " PROC " + "Writing PKG to " + strFullPath + "/package.conf";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    ofstream fdJson(strFullPath + "/package.conf", std::ofstream::trunc);
    mapSeriesJson.clear();
    if(fdJson.is_open()) {
        fdJson << strJson;
        fdJson.close();
    } else {
        strLogMessage=strPrimalID + " PROC  Unable to open file " + strFullPath + "/package.conf";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    strLogMessage= strPrimalID + " PROC  Completed creating json...";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    mysql_close(mconnect2);
    return 0;
}

std::size_t fPackDir(std::string strFullPath, std::size_t intMsgType) {
    std::map<std::string, std::string>::iterator iprimConf, iprimConf2;
    std::string strReturn, strTemp, strPackType, strFileExt, strCmd, strPackFileName, strPrimalID, strRecNum, strLogMessage;
    std::size_t intLC, intFound, intPos, intTar;
    (void) intMsgType;

    intFound = strFullPath.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strFullPath.substr(intFound + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);

    strPackFileName = "*.dcm";
    intLC=0;
    intTar=0;
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
    while(iprimConf != conf1.primConf.end()) {
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPACK" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            if(conf1.primConf[strRecNum + "_PRIDESTPACK" + to_string(intLC)] == "tar.gz") {
                intTar = 1;
            }
        }
        intLC++;
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
    }
    if(intTar == 1) {
        strPackFileName = strPrimalID + "_" + to_string(intLC) + ".tar";
        //strCmd = "(cd " + conf1.primConf[strRecNum + "_PRIPROC"] + " && tar -czf " + strFullPath + "/" + strPrimalID + ".tar.gz ./" + strPrimalID + "/*.dcm)";
        strCmd = "(cd " + conf1.primConf[strRecNum + "_PRIPROC"] + " && tar -cf " + strFullPath + "/" + strPrimalID + ".tar ./" + strPrimalID + "/*.dcm )";
        strLogMessage = strPrimalID + " PROC " + "Tar Command: " + strCmd;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        system(strCmd.c_str());
        strCmd = "pigz " + strFullPath + "/" + strPrimalID + ".tar";
        strLogMessage = strPrimalID + " PROC " + "Compression Command: " + strCmd;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        system(strCmd.c_str());
    }

    return 0;
}

std::size_t fPackDir2(std::string strFullPath, std::size_t intMsgType) {
    std::map<std::string, std::string>::iterator iprimConf, iprimConf2;
    std::string strReturn, strTemp, strPackType, strFileExt, strCmd, strPackFileName, strPrimalID, strRecNum, strLogMessage;
    std::size_t intLC, intFound, intPos, intTar;
    struct stat st;
    (void) intMsgType;

    strRecNum="1";
    intFound = strFullPath.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strFullPath.substr(intFound + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);

    strPackFileName = strFileExt;
    intLC=0;
    intTar=0;
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
    while(iprimConf != conf1.primConf.end()) {
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPACK" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            if(conf1.primConf[strRecNum + "_PRIDESTPACK" + to_string(intLC)] == "tar.gz") {
                intTar = 1;
            }
        }
        intLC++;
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
    }
    iprimConf = conf1.primConf.find(strRecNum + "_PRIJSON");
    if(iprimConf != conf1.primConf.end()) {
        if(conf1.primConf[strRecNum + "_PRIJSON"] == "1" && intTar == 1) {
            //See if the file exists
            strTemp=strFullPath + "/" + strPrimalID + ".tar.gz";
            if(stat(strTemp.c_str(),&st) != 0) {
                strLogMessage="ERROR:  Cannot rename " + strTemp + " as it does not exist...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
            } else {
                fs::rename(strFullPath + "/" + strPrimalID + ".tar.gz", strFullPath + "/_DICOM.tar.gz");
                strCmd = "(cd " + conf1.primConf[strRecNum + "_PRIPROC"] + "/" + strPrimalID + " && tar -cf " + strPrimalID;
                strCmd += ".tar _DICOM.tar.gz payload.json package.conf >> " + conf1.primConf[strRecNum + "_PRILFPROC"] + " 2>&1)";
                system(strCmd.c_str());
                fs::remove(conf1.primConf[strRecNum + "_PRIPROC"] + "/" + strPrimalID + "/_DICOM.tar.gz");
            }
        }
    }

    return 0;
}

std::size_t fProcessMsg(std::string strMessage) {
    std::string strFullPath, strPrimalID, strQuery, strRow, strTemp, strJson, strSerIUID, strSerTemp, strTemp2, strFilename, strOldVal;
    std::string strDate, strRecNum, strPrimalIDInt, strTemp3, strLogMessage, strSIUID, strAEC, strPackFileName, strCaseID, strNewPath;
    std::string strDBREturn, strJsonFile, strTempPath, strPatientName, strPID, strACCN, strcallingAET, strPrefetchNode;
    std::string strClientID, strClientAET, strClientName, strTempVar, strLine, strCmd, strMyDicomCasesID, strFirstDicomCasesID;
    std::size_t intDone2, intPos, intFound, intMsgType, intReturn, intLC, intDBEntries, intError, intNumRows, intFound2;
    std::size_t intFound3, intProcError;
    std::map<std::string, std::string> mapSeriesJson;
    std::map<std::string, std::string>::iterator iprimConf;
    mysql_thread_init();
    MYSQL *mconnect2;
    struct stat sb;
    MYSQL_ROW row;

    strRecNum="1";
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fProcessMsg.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return 1;
    }
    //mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="MySQL connection failed in fProcessMsg.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return 1;
    }

    strFullPath=strMessage;
    //Parse message
    intPos=strFullPath.find_last_of(" ");
    if(intPos == std::string::npos) {
        strLogMessage = " PROC ERROR:  Message " + strMessage + " did not contain a type.  Skipping...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        //std::cout << strLogMessage << std::endl;
        mysql_close(mconnect2);
        return 1;
    }
    intMsgType=stoi(strFullPath.substr(intPos));
    strFullPath.erase(intPos);
    if(strFullPath.back() == '/') {
        strFullPath.pop_back();
    }
    intFound = strFullPath.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strFullPath.substr(intFound + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);
    strPrimalIDInt=strPrimalID;

    if(!fs::exists(strFullPath)) {
        strLogMessage = " PROC WARN:  Directory" + strFullPath + " does not exist.  Skipping...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        //std::cout << strLogMessage << std::endl;
        mysql_close(mconnect2);
        return 1;
    }

    //Start doing the work
    if(intMsgType == 1) {
        strJsonFile = strFullPath + "/payload.json";
        if (stat(strJsonFile.c_str(), &sb) != 0) {
            strLogMessage = "PROC Got the following message: " + strMessage;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
            iprimConf = conf1.primConf.find(strRecNum + "_PRIJSON");
            if(iprimConf != conf1.primConf.end()) {
                if(conf1.primConf[strRecNum + "_PRIJSON"] == "1") {
                    fCreateJson(strFullPath, intMsgType);
                }
            }
        }
    } else if(intMsgType == 2) {
        strLogMessage = "PROC Got the following message: " + strMessage;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        strQuery="select pname, pid from patient where puid ='" + strPrimalID + "';";
        mysql_query(mconnect2, strQuery.c_str());
        if(*mysql_error(mconnect2)) {
            strLogMessage=mysql_error(mconnect2);
            strLogMessage+="\n" + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        }
        MYSQL_RES *result = mysql_store_result(mconnect2);
        if(result) {
            intNumRows = mysql_num_rows(result);
            if(intNumRows > 0) {
                row = mysql_fetch_row(result);
                strPatientName = row[0];
                strPID = row[1];
            }
            mysql_free_result(result);
        }
        strQuery="select count(*) from study where puid='" + strPrimalID + "';";
        mysql_query(mconnect2, strQuery.c_str());
        result = mysql_store_result(mconnect2);
        row = mysql_fetch_row(result);
        strDBREturn=row[0];
        intDBEntries=stoi(strDBREturn);
        mysql_free_result(result);
        if(intDBEntries < 1) {
            strACCN = "NULL";
        } else {
            strQuery="select AccessionNum from study where puid ='" + strPrimalID + "';";
            mysql_query(mconnect2, strQuery.c_str());
            result = mysql_store_result(mconnect2);
            row = mysql_fetch_row(result);
            strACCN = row[0];
            //std::cout << "strQuery = " << strQuery << std::endl;
            //std::cout << "Accession Number: " << strACCN << std::endl;
            mysql_free_result(result);
        }

        strLogMessage = strPrimalID + " PROC " + " Started processing: " + strMessage + " Patient: " + strPatientName + " MRN: " + strPID + " ACCN: " + strACCN;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        strQuery="insert into process set puid='" + strPrimalID + "', pservername='" + strHostname + "', ";
        strQuery+=" tstartproc='" + GetDate() + "';";
        strLogMessage = strPrimalID + " PROC " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        mysql_query(mconnect2, strQuery.c_str());
        iprimConf = conf1.primConf.find(strRecNum + "_PRIJSON");
        if(iprimConf != conf1.primConf.end()) {
            if(conf1.primConf[strRecNum + "_PRIJSON"] == "1") {
                fCreateJson(strFullPath, intMsgType);
            }
        }
        intDone2=0;
        intLC=0;
        intReturn=fRulesChain(strFullPath);
        strLogMessage = strPrimalID + " PROC " + "Finished processing rules chain.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        //std::cout << strLogMessage << std::endl;
        if(intReturn == 3) {
            intError = 0;
            try {
                fs::copy(strFullPath, conf1.primConf[strRecNum + "_PRIHOLD"] + "/" + strPrimalID);
            } catch (fs::filesystem_error& err) {
                strLogMessage = strPrimalID + " PROC " + " ERROR:  Could not move " + strFullPath + ".  Skipping...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                //std::cout << strLogMessage << std::endl;
                intError = 1;
            }
            if(intError == 0) {
                try {
                    fs::remove_all(strFullPath);

                } catch (fs::filesystem_error& err) {
                    strLogMessage = strPrimalID + " PROC " + " ERROR:  Could not remove " + strFullPath + ".  Skipping...";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                    //std::cout << strLogMessage << std::endl;
                }
            }
            strQuery="update process set tendproc='" + GetDate() + "' where puid='" + strPrimalID + "';";
            mysql_query(mconnect2, strQuery.c_str());
            fWriteMessage(strNewPath, "/prim_send");
            strLogMessage = strPrimalID + " PROC " + "Stopped processing: " + strMessage + " Patient: " + strPatientName + " MRN: " + strPID + " ACCN: " + strACCN;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
            //std::cout << strLogMessage << std::endl;
            mysql_close(mconnect2);
            return 0;
        }
        while(intDone2 != 1 && intLC < 1000) {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPACK" + to_string(intLC));
                if(conf1.primConf[strRecNum + "_PRIDESTPACK" + to_string(intLC)] == "tar.gz") {
                    strLogMessage = strPrimalID + " PROC " + "Creating .tar.gz for receiver " + strRecNum + " desination " + to_string(intLC);
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                    //std::cout << strLogMessage << std::endl;
                    intReturn=fPackDir(strFullPath, intMsgType);
                    if(intReturn == 0) {
                        strPrefetchNode="0";
                        intReturn=fCreatePkg(strFullPath, intMsgType, strPrefetchNode);
                    }
                    if(intReturn == 0) {
                        fPackDir2(strFullPath, intMsgType);
                    }
                    intDone2 = 1;
                } else {
                    strLogMessage = strPrimalID + " PROC " + "No compression for receiver " + strRecNum + " desination " + to_string(intLC);
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                }
            } else {
                intDone2 = 1;
                //strLogMessage = strPrimalID + " PROC " + "intDone2 triggered for receiver " + strRecNum + " desination " + to_string(intLC);
                //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
            }
            intLC++;
        }
        strQuery = "select senderAET from receive where puid = '" + strPrimalID + "';";
        //strLogMessage=strPrimalID + " PROC " + strQuery;
        //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        mysql_query(mconnect2, strQuery.c_str());
        if(*mysql_error(mconnect2)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect2);
            strLogMessage+="\nstrQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        }
        result = mysql_store_result(mconnect2);
        if(result) {
            intNumRows = mysql_num_rows(result);
            if(intNumRows > 0) {
                row = mysql_fetch_row(result);
                strcallingAET = row[0];
            } else {
                strcallingAET = "DEFAULT";
            }
            mysql_free_result(result);
        } else {
            strcallingAET = "DEFAULT";
        }
        strLogMessage = strPrimalID + " PROC " + "Set callingAET to: " + strcallingAET;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        strPrefetchNode = "0";
        if(fs::exists("/etc/primal/prim_ae_map.conf")) {
            std::ifstream fpAEMap("/etc/primal/prim_ae_map.conf");
            while (std::getline(fpAEMap, strLine)) {
                intFound=strLine.find(",");
                intFound2=strLine.find(",", intFound + 1);
                intFound3=strLine.find(",", intFound2 + 1);
                strClientID = strLine.substr(0, intFound);
                strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
                strClientName = strLine.substr(intFound2 + 1, intFound3 - intFound2 -1);
                strTempVar = strLine.substr(intFound3 + 1);
                intFound2=strcallingAET.find(strClientAET);
                if(intFound2 != std::string::npos) {
                    strPrefetchNode = strTempVar;
                    strLogMessage = strPrimalID + " PROC " + "Found callingAET " + strcallingAET + " setting prefetch to " + strPrefetchNode;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                }
            }
            fpAEMap.close();
        }
        //std::cout << strLogMessage << std::endl;
        if(strPrefetchNode.compare("0") == 0) {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIQRHIP0");
            if(iprimConf != conf1.primConf.end()) {
                strQuery="select DicomCasesID from study where puid='" + strPrimalID + "';";
                if(mysql_query(mconnect2, strQuery.c_str())) {
                    strLogMessage=mysql_error(mconnect2);
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                }
                result = mysql_store_result(mconnect2);
                if(result) {
                    intNumRows = mysql_num_rows(result);
                    if(intNumRows > 0){
                        row = mysql_fetch_row(result);
                        strMyDicomCasesID=row[0];
                        mysql_free_result(result);
                    }
                }
                strQuery="select DicomCasesID from study where AccessionNum = '" + strACCN + "' order by DicomCasesID limit 1;";
                if(mysql_query(mconnect2, strQuery.c_str())) {
                    strLogMessage=mysql_error(mconnect2);
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                }
                result = mysql_store_result(mconnect2);
                if(result) {
                    intNumRows = mysql_num_rows(result);
                    if(intNumRows > 0){
                        row = mysql_fetch_row(result);
                        strFirstDicomCasesID=row[0];
                        mysql_free_result(result);
                    }
                }
                mtx.unlock();
                if(strMyDicomCasesID.compare(strFirstDicomCasesID) == 0) {
                    strLogMessage = strPrimalID + " PROC " + "Sarting C-find for " + strPrimalID + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                    fWriteMessage(strFullPath + " 1", "/prim_qr");
                } else {
                    strLogMessage = strPrimalID + " PROC " + "Skipping C-FIND for " + strPrimalID + " because we are not the first (" + strMyDicomCasesID + " != " + strFirstDicomCasesID + ").";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                    strCmd = "touch " + strFullPath + "/cfind.json";
                    system(strCmd.c_str());
                }
            }
        } else {
            strLogMessage = strPrimalID + " PROC " + "Skipping C-find for " + strPrimalID + " because it's a prior AET.";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
            strCmd = "touch " + strFullPath + "/cfind.json";
            system(strCmd.c_str());
            //std::cout << strLogMessage << std::endl;
        }
        iprimConf = conf1.primConf.find(strRecNum + "_PRIQRHIP0");
        if(iprimConf != conf1.primConf.end()) {

            strTemp=strFullPath + "/cfind.json";
            while(! fs::exists(strTemp) && intLC < 20) {
            //while(stat(strTemp.c_str(),&st) != 0 && intLC < 20) {
                strLogMessage = strPrimalID + " PROC " + "Waiting for " + strTemp + " to appear.";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                std::this_thread::sleep_for (std::chrono::seconds(3));
                intLC++;
            }
        }

        //End of processing.  Need to move the directory and call the sender
        strQuery="select perror from process where puid = \"" + strPrimalID + "\" and perror is not NULL";
        mysql_query(mconnect2, strQuery.c_str());
        if(*mysql_error(mconnect2)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect2);
            strLogMessage+="\nstrQuery = " + strQuery + ".";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        }
        result = mysql_store_result(mconnect2);
        if(result) {
            intNumRows = mysql_num_rows(result);
            if(intNumRows > 0) {
                row = mysql_fetch_row(result);
                strTemp=row[0];
                std::stringstream sstream(strTemp);
                sstream >> intProcError;
                if(intProcError > 0) {
                    if (stat(strFullPath.c_str(), &sb) == 0) {
                        strNewPath=conf1.primConf[strRecNum + "_PRIERROR"] + "/" + strPrimalID;
                        strCmd="mv " + strFullPath + " " + conf1.primConf[strRecNum + "_PRIERROR"] + "/";
                        system(strCmd.c_str());
                        strQuery = "update image set ilocation = '" + conf1.primConf[strRecNum + "_PRIERROR"] + "/" + strFullPath + "' where puid = '" + strPrimalID + "';";
                        mysql_query(mconnect2, strQuery.c_str());
                        if(*mysql_error(mconnect2)) {
                            strLogMessage="SQL Error: ";
                            strLogMessage+=mysql_error(mconnect2);
                            strLogMessage+="\nstrQuery = " + strQuery + ".";
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                        }
                        strQuery="update process set tendproc='" + GetDate() + "' where puid='" + strPrimalID + "';";
                        mysql_query(mconnect2, strQuery.c_str());
                        strLogMessage = strPrimalID + " PROC " + "Finished processing: " + strMessage + " Patient: " + strPatientName + " MRN: " + strPID + " ACCN: " + strACCN;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                    }
                }
            }
            mysql_free_result(result);
        }
        if (stat(strFullPath.c_str(), &sb) == 0) {
            strNewPath=conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimalID;
            strCmd="mv " + strFullPath + " " + conf1.primConf[strRecNum + "_PRIOUT"] + "/";
            system(strCmd.c_str());
            //fs::rename(strFullPath, strNewPath);
        }
        strQuery="update process set tendproc='" + GetDate() + "' where puid='" + strPrimalID + "';";
        //std::cout << strQuery << std::endl;
        mysql_query(mconnect2, strQuery.c_str());
        //fWriteMessage(strNewPath, "/prim_send");
        strLogMessage = strPrimalID + " PROC " + "Finished processing: " + strMessage + " Patient: " + strPatientName + " MRN: " + strPID + " ACCN: " + strACCN;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        //std::cout << strLogMessage << std::endl;
    }
    mysql_close(mconnect2);
    mysql_thread_end();
    return 0;
}

int main() {
    std::string strFullPath, strPrimalID, strQuery, strRow, strTemp, strJson, strSerIUID, strSerTemp, strTemp2, strFilename, strOldVal;
    std::string strDate, strRecNum, strPrimalIDInt, strTemp3, strLogMessage, strSIUID, strAEC, strPackFileName, strCaseID, strNewPath;
    std::string strDBREturn, strMessage, strJsonFile, strTempPath, strPatientName, strPID, strACCN;
    std::size_t intFoundThread, intDone=0, intNumThreads = 0, intMaxThreads = 2;
    std::map<std::string, std::string> mapSeriesJson;
    std::map<std::string, std::string>::iterator iprimConf;
    std::vector<std::thread> vecThreads;
    char* pPath;
    char cPath[]="DCMDICTPATH=/opt/primal/home/build/share/dcmtk/dicom.dic";
    putenv( cPath );
    mysql_library_init(0, NULL, NULL);
    MYSQL *mconnect;

    strRecNum="1";
    pPath = getenv ("DCMDICTPATH");
    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect) {
        strLogMessage="MySQL Initilization failed in main.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return 1;
    }
    ReadDBConfFile();
    //mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="MySQL connection failed in main.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return 1;
    }
    intMaxThreads = std::thread::hardware_concurrency();
    strLogMessage = "Starting prim_process_server version 1.01.00 with " + to_string(intMaxThreads) + " threads.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    system("export DCMDICTPATH=/opt/primal/home/build/share/dcmtk/dicom.dic");
    conf1.ReadConfFile();
    strRecNum = "1";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    strLogMessage = "DCMDICTPATH";
    strLogMessage += pPath;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    intNumThreads=0;
    while(intDone != 1) {
        strMessage=fGetMessage("/prim_process");
        if(vecThreads.size() < intMaxThreads) {
            vecThreads.emplace_back(fProcessMsg, strMessage);
            intNumThreads++;
            strLogMessage = " PROC Thread " + to_string(intNumThreads) + " started processing: " + strMessage;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
            //std::cout << strLogMessage << std::endl;
        } else {
            intFoundThread=0;
            while(intFoundThread == 0) {
                for (long unsigned int i=0; i<vecThreads.size(); i++) {
                    if(vecThreads[i].joinable()) {
                        vecThreads[i].join();
                        std::thread th(fProcessMsg, strMessage);
                        vecThreads[i] = std::move(th);
                        strLogMessage = " PROC Thread " + to_string(i) + " started processing: " + strMessage;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                        intFoundThread=1;
                        break;
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    mysql_close(mconnect);
    mysql_library_end();
    return 0;
}