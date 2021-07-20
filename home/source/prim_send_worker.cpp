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
#include <fstream>
#include <sstream>
#include <filesystem>
#include <pstreams/pstream.h>
#include <mysql/my_global.h>
#include <mysql/mysql.h>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <csignal>
#include "libssh2_config.h"
#include <libssh2.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

std::mutex mtx;
using namespace std;
namespace fs = std::filesystem;
std::vector<std::string > vecRCcon1;
std::vector<std::string > vecRCopt1;
std::vector<std::string > vecRCcon2;
std::vector<std::string > vecRCact1;

const std::string strProcChainType = "PRIMRCSEND";

#include "prim_functions.h"

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

std::string fPassThrough(std::string strPrimID, std::string strRecNum) {
    std::string strAET, strQuery, strLogMessage;
    std::map<std::string, std::string>::iterator iprimConf;
    MYSQL *mconnect_local;
    MYSQL_ROW row;
    MYSQL_RES *result;

    mconnect_local=mysql_init(NULL);
    mysql_options(mconnect_local,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect_local) {
        strLogMessage="MySQL Initilization failed in fPassThrough.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return "-1";
    }
    mconnect_local=mysql_real_connect(mconnect_local, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect_local) {
        strLogMessage="MySQL connection failed in fPassThrough";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return "-1";
    }
    //Are we going to impersinate the machine that sent this study?
    if (conf1.primConf[strRecNum + "_PRIPASSTU"] == "1") {
        strQuery="select senderAET from receive where PUID = '" + strPrimID + "' limit 1;";
        mysql_query(mconnect_local, strQuery.c_str());
        result = mysql_store_result(mconnect_local);
        while ((row = mysql_fetch_row(result))) {
            strAET=row[0];
        }
        mysql_free_result(result);
        strLogMessage="We are passing through the AET of " + strAET;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    } else {
        iprimConf = conf1.primConf.find(strRecNum + "_PRIAET");
        if(iprimConf != conf1.primConf.end()) {
            strAET=conf1.primConf.find(strRecNum + "_PRIAET")->second;
        } else {
            strAET="PRI_DEFAULT";
        }
    }
    mysql_close(mconnect_local);
    return strAET;
}

std::size_t fConvertDatetoSeconds(std::string strDBDate) {
    std::string strCMD, strReturn;
    std::size_t intDateInSeconds;
    std::stringstream sstream("1");

    if(strDBDate == "0") {
        strCMD = "date +%s";
        strReturn = exec(strCMD.c_str());
        sstream.clear();
        sstream.str(strReturn);
        sstream >> intDateInSeconds;
    } else {
        strCMD = "date +%s -d \"" + strDBDate + "\"";
        strReturn = exec(strCMD.c_str());
        sstream.clear();
        sstream.str(strReturn);
        sstream >> intDateInSeconds;
    }
    return intDateInSeconds;
}

std::string Get_Date_Time() {
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	std::string strTime;

	strTime=std::to_string(now->tm_year + 1900);
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

static int waitsocket(int socket_fd, LIBSSH2_SESSION *session) {
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;
 
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
 
    FD_ZERO(&fd);
 
    FD_SET(socket_fd, &fd);
 
    /* now make sure we wait in the correct direction */ 
    dir = libssh2_session_block_directions(session);

 
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;
 
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;
 
    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
 
    return rc;
}

int fCheckResend(std::string strFullPath, std::size_t intLC) {
    std::size_t intPos, intNumRows, intReturn, intNumRetries, intRetryDelay, intTimestamp, intCurTimestamp=0;
    std::string strPrimalID, strRecNum, strReturn, strLogMessage, strWorkingDirectory, strQuery;
    std::stringstream sstream("1");
    std::map<std::string, std::string>::iterator iprimConf;

    MYSQL *mconnect;
    MYSQL_ROW row;
    MYSQL_RES *result;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect) {
        strLogMessage="MySQL Initilization failed in fPollSendDir.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        std::cout << strLogMessage << std::endl;
        return 0;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="MySQL connection failed in fPollSendDir";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        std::cout << strLogMessage << std::endl;
        return 0;
    }

    if(strFullPath.back() == '/') {
        strFullPath.pop_back();
    }
    //std::cout << "strFullPath: " << strFullPath << std::endl;
    intPos = strFullPath.find_last_of("/");
    if(intPos != std::string::npos) {
        strPrimalID=strFullPath.substr(intPos + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);
    if(strFullPath.back() != '/') {
        strFullPath.push_back('/');
    }

    //td::cout << "strRecNum: " << strRecNum << "   intLC: " << to_string(intLC) << std::endl;
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTRETRY" + to_string(intLC));
    if(iprimConf != conf1.primConf.end()) {
        strReturn = conf1.primConf[strRecNum + "_PRIDESTRETRY" + to_string(intLC)];
        sstream.clear();
        sstream.str(strReturn);
        sstream >> intNumRetries;
    } else {
        //Default is to not retry so let's return a non-zero result (don't retry)
        intNumRetries = 0;
    }
    //std::cout << "Number of retries for dest " << to_string(intLC) << " is set to " << to_string(intNumRetries) << std::endl;
    strQuery = "select serror from send where puid = '" + strPrimalID + "' and tdestnum = " + to_string(intLC) + " and serror is not NULL order by tstartsend desc limit 1;";
    //std::cout << strQuery << std::endl;
    mysql_query(mconnect, strQuery.c_str());
    if(*mysql_error(mconnect)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect);
        strLogMessage+="strQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    }
    result = mysql_store_result(mconnect);
    if(result) {
        intNumRows = mysql_num_rows(result);
        //std::cout << "intNumRows: " << to_string(intNumRows) << std::endl;
        if(intNumRows > 0) {
            row = mysql_fetch_row(result);
            strReturn=row[0];
            //std::cout << "strReturn: " << strReturn << std::endl;
            mysql_free_result(result);
            sstream.clear();
            sstream.str(strReturn);
            sstream >> intReturn;
            if(intReturn > intNumRetries) {
                //std::cout << "We have tried " << to_string(intReturn) << " times.  Max allowed is " << to_string(intNumRetries) << ".  Not trying anymore..." << std::endl;
                strLogMessage=strPrimalID + " SEND  We have tried " + to_string(intReturn) + " times.  Max allowed is " + to_string(intNumRetries) + ".  Not trying anymore...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                return 1;
            } else {
                strLogMessage=strPrimalID + " SEND  We have tried " + to_string(intReturn) + " times.  Max allowed is " + to_string(intNumRetries) + ".  Sending study...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            }
        } else {
            //std::cout << "No response from DB.  Sending study..." << std::endl;
            //No database result so we must not have tried before.
            strLogMessage=strPrimalID + " SEND  We are not sure how many times we have tried (empty result set).  Max allowed is " + to_string(intNumRetries) + ".  Sending study...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            return 0;
        }
    } else {
        //No database result so we must not have tried before.
        //std::cout << "No response from DB.  Sending study..." << std::endl;
        strLogMessage=strPrimalID + " SEND  We are not sure how many times we have tried (no results).  Max allowed is " + to_string(intNumRetries) + ".  Sending study...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 0;
    }
    //Now let's check the time...
    strQuery = "select tendsend from send where puid = '" + strPrimalID + "' and tdestnum = " + to_string(intLC) + " and tendsend is not NULL order by tendsend desc limit 1;";
    mysql_query(mconnect, strQuery.c_str());
    if(*mysql_error(mconnect)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect);
        strLogMessage+="strQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    }
    result = mysql_store_result(mconnect);
    if(result) {
        intNumRows = mysql_num_rows(result);
        if(intNumRows > 0) {
            row = mysql_fetch_row(result);
            strReturn=row[0];
            //std::cout << "DB returned: " << strReturn << std::endl;
            mysql_free_result(result);
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTRTO" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                sstream.clear();
                sstream.str(conf1.primConf[strRecNum + "_PRIDESTRTO" + to_string(intLC)]);
                sstream >> intRetryDelay;
                //std::cout << "Retry delay is configured for: " << to_string(intRetryDelay) << " seconds." << std::endl;
            } else {
                intRetryDelay = 60;
                //std::cout << "Retry delay is not configured.  Setting it to: " << to_string(intRetryDelay) << " seconds." << std::endl;
            }
            intTimestamp = fConvertDatetoSeconds(strReturn);
            intCurTimestamp = fConvertDatetoSeconds("0");
            if(intCurTimestamp > (intTimestamp + intRetryDelay)) {
                strLogMessage=strPrimalID + " SEND  Waited long enough to try again.  Last try timestamp: " + to_string(intTimestamp) + " Delay: " + to_string(intRetryDelay) + " Current timestamp: " + to_string(intCurTimestamp) + " Sending study...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                return 0;
            } else {
                //strLogMessage=strPrimalID + " SEND  Current timestamp is: " + to_string(intCurTimestamp) + " but need to wait until " + to_string(intTimestamp + intRetryDelay) + ".  Waiting...";
                //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                return 2;
            }
        } else {
            strLogMessage=strPrimalID + " SEND  Not sure how long we have waited (empty result set).  Current timestamp: " + to_string(intCurTimestamp) + " Sending study...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            return 0;
        }
    }
    strLogMessage=strPrimalID + " SEND  Not sure how long we have waited (no results).  Current timestamp: " + to_string(intCurTimestamp) + " Sending study...";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    return 0;
}

int fSendStudy(std::string strPrimID, std::string strRecNum, std::size_t intLC, std::string strAET, std::string strPackFileName) {
    std::string strDateTime, strQuery, strHostName, strFullPath, strTemp, strSendType, strCmd, strReturn, strClientID;
    std::string strLine, strDate, strAEC, strFileName, strOldFileName, strReadLine, strPasswd, strDestHIP, strDestPort, strDestUser;
    std::string strDestPath, strClientAET, strClientName, strLogMessage, strCmd2, strLine2, strExecType;
    std::size_t intNumFiles = 0, intLC2, intFound, intFound2, intPos, intImgRec, nread, intLC3, intError=0, intAETMapped=0;
    std::size_t intNumRows, intCondDest;
    int intRetryStatus, intErrors;
    int sock, rc, auth_pw = 1;
    std::map<std::string, std::string>::iterator iprimConf;
    std::stringstream sstream("1");
    char cHostName[1024], mem[1024], *ptr;
    MYSQL *mconnect_local;
    MYSQL_ROW row;
    MYSQL_RES *result;

    LIBSSH2_SESSION *session = NULL;
    LIBSSH2_CHANNEL *channel;
    //const char *fingerprint;
    struct sockaddr_in sin;
    struct stat fileinfo;
    FILE *local;
    
    mconnect_local=mysql_init(NULL);
    mysql_options(mconnect_local,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect_local) {
        strLogMessage=strPrimID + " SEND  MySQL Initilization failed in fSendStudy.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 0;
    }
    mconnect_local=mysql_real_connect(mconnect_local, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    intLC3=0;
    while (!mconnect_local && intLC3 < 60) {
        strLogMessage=strPrimID + " SEND  MySQL connection failed in fSendStudy";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        std::this_thread::sleep_for (std::chrono::seconds(1));
        intLC3++;
    }
    if(intLC3 >= 60) {
        return 0;
    }
    strDateTime = Get_Date_Time();
    gethostname(cHostName, 1024);
    strHostName = cHostName;
    strDestHIP = conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)];
    strLogMessage=strPrimID + " SEND  hostname " + strHostName + " destination " + strDestHIP + ".";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    //Count the number of files in the directory
    iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
    if(iprimConf != conf1.primConf.end()) {
        strFullPath = conf1.primConf.find(strRecNum + "_PRIOUT")->second;
        strFullPath.append("/" + strPrimID + "/");
    } else {
        //I don't even know what to do here.
        strReturn+= "ERROR:  Couldn't figure out what the outbound directory is...  Exiting";
    }

    if(!fs::exists(strFullPath)) {
        strLogMessage = " SEND " + strPrimID + " WARN:  Directory" + strFullPath + " does not exist.  Skipping...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        mysql_close(mconnect_local);
        return 0;
    }
    auto iFullPath = std::filesystem::directory_iterator(strFullPath);
    for (auto& entry : iFullPath) {
        if (entry.is_regular_file()) {
            ++intNumFiles;
        }
    }
    //Should we use conditional destination or not?
    intCondDest = 0;
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTSCRIPT" + to_string(intLC));
    if(iprimConf != conf1.primConf.end()) {
        strTemp = conf1.primConf[strRecNum + "_PRIDESTSCRIPT" + to_string(intLC)];
        strCmd = "/usr/local/scripts/" + conf1.primConf[strRecNum + "_PRIDESTSCRIPT" + to_string(intLC)] + " " + to_string(intLC) + " " + conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID;
        strLogMessage = " SEND " + strPrimID + " INFO:  BASH executing: " + conf1.primConf[strRecNum + "_PRIDESTSCRIPT" + to_string(intLC)];
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        strReturn = exec(strCmd.c_str());
        sstream.clear();
        sstream.str(strReturn);
        getline(sstream, strExecType);
        if(strExecType.compare("REP") == 0) {
            getline(sstream, strDestHIP);
            getline(sstream, strDestPort);
            getline(sstream, strAEC);
            strLogMessage =strPrimID + "SEND  INFO:  BASH " + conf1.primConf[strRecNum + "_PRIDESTSCRIPT" + to_string(intLC)] + " replacing IP: " + strDestHIP + ", port: " + strDestPort + " and AET: " + strAEC + " for dest " + to_string(intLC) + "...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intCondDest = 1;
        }
    }
    //We really need to send to this destination.
    intRetryStatus = fCheckResend(strFullPath, intLC);
    strLogMessage="intRetryStatus = " + to_string(intRetryStatus);
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    if(intRetryStatus == 1 || intRetryStatus == 2) {
        return intRetryStatus;
    }
    if(strDestHIP.find("0.0.0.0") != std::string::npos) {
        //We use 0.0.0.0 to be a dummy destination
        strQuery="insert into send (puid, sservername, tdest, tstartsend, tendsend, timages, serror, complete) values ('";
        strQuery.append(strPrimID + "', '" + strHostName + "', '" + to_string(intLC) + "', '" + strDateTime + "', '" + strDateTime);
        strQuery.append("','" + to_string(intNumFiles) + "', 0, 1);");
        mysql_query(mconnect_local, strQuery.c_str());
        if(*mysql_error(mconnect_local)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect_local);
            strLogMessage+="\nstrQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        }
        strLogMessage=strPrimID + " SEND  Skipping this send because dummy host " + strDestHIP + " found.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTTYPE" + to_string(intLC));
    if(iprimConf != conf1.primConf.end()) {
        strTemp = conf1.primConf[strRecNum + "_PRIDESTTYPE" + to_string(intLC)];
    } else {
        strReturn += "ERROR:  Could not determine destination type.  Using default:  DICOM\n";
        strTemp = "DICOM";
    }
    for (intLC2=0; intLC2<strTemp.length(); intLC2++) {
        strSendType += std::toupper(strTemp[intLC2]);
    }
    strLogMessage=strPrimID + " SEND  Send type is " + strSendType;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    if (strSendType == "SCP") {
        strLogMessage=strPrimID + " SEND  Starting SCP.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        strPackFileName="none";
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPACK" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            if(conf1.primConf[strRecNum + "_PRIDESTPACK" + to_string(intLC)] == "tar.gz") {
                strTemp=conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/";
                for (const auto & entry : fs::directory_iterator(strTemp)) {
                    strFileName=entry.path().string();
                    intPos=strFileName.find(".tar");
                    if(intPos != std::string::npos) {
                        intPos=strFileName.find_last_of("/");
                        if(intPos != std::string::npos) {
                            strPackFileName=strFileName.substr(intPos + 1);
                        } else {
                            strPackFileName=strFileName;
                        }
                        intFound = 1;
                        intNumFiles=1;
                    }
                }
            }
        }
        if(intFound != 1) {
            strLogMessage=" SEND " + strPrimID + " WARN:  SCP selected but no .tar found.  This is not currently handeled.  Skipping...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            return 0;
        }
        if(strPackFileName == "none") {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTCDCR" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "2") {
                    strPackFileName="*.ucr";
                } else if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "1") {
                    strPackFileName="*.j2k";
                } else {
                    strPackFileName="*.dcm";
                }
            } else {
                strPackFileName="*.dcm";
            }
        }
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
        if(intCondDest != 1) {
            if(iprimConf != conf1.primConf.end()) {
                strDestHIP=conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)];
            } else {
                strReturn += "ERROR:  Could not determine the hostname or IP for this destination.  Exiting...\n";
                intError=1;
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPORT" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strDestPort=conf1.primConf[strRecNum + "_PRIDESTPORT" + to_string(intLC)];
            } else {
                strReturn += "ERROR:  Could not determine the port for this destination.  Using 22\n";
                strDestPort="22";
            }
        }
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTUSER" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            strDestUser=conf1.primConf[strRecNum + "_PRIDESTUSER" + to_string(intLC)];
        } else {
            strReturn += "ERROR:  Could not determine the hostname or IP for this destination.  Exiting...\n";
            intError=1;
        }
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPASS" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            strPasswd=conf1.primConf[strRecNum + "_PRIDESTPASS" + to_string(intLC)];
        } else {
            strReturn += "ERROR:  Could not determine the password for this destination.  Exiting...\n";
            intError=1;
        }
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPATH" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            strDestPath=conf1.primConf[strRecNum + "_PRIDESTPATH" + to_string(intLC)];
        } else {
            strDestPath="";
        }
        strLogMessage=strPrimID + " SEND Start SCP send to " + strDestHIP + " on port " + strDestPort;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        strDestPath+=strPackFileName + ".part";
        //std::cout << "SCP Putting file in " << strDestPath << std::endl;
        strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
        strQuery+=", tdest='SCP" + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "'";
        strQuery+=", tstartsend='" + GetDate() + "', tdestnum=" + to_string(intLC) + ";";
        mysql_query(mconnect_local, strQuery.c_str());
        if(*mysql_error(mconnect_local)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect_local);
            strLogMessage+="\nstrQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        }

        rc = libssh2_init(0);
        if(rc != 0) {
            strLogMessage=" SEND " + strPrimID + " libssh2 initialization failed " + to_string(rc);
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intError=1;
        }
        strFullPath+=strPackFileName;
        strLogMessage=" SEND " + strPrimID + " SCP Sending file " + strFullPath + " to " + strDestHIP + " on " + strDestPort;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        local = fopen(strFullPath.c_str(), "rb");
        if(!local) {
            strLogMessage=" SEND " + strPrimID + " SCP Can't open local file " + strFullPath;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intError=1;
        }
        stat(strFullPath.c_str(), &fileinfo);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(-1 == sock) {
            strLogMessage=" SEND " + strPrimID + " failed to create socket!";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intError=1;
        }
        sin.sin_family = AF_INET;
        sin.sin_port = htons(stoi(strDestPort));
        sin.sin_addr.s_addr = inet_addr(strDestHIP.c_str());
        if(connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
            strLogMessage=" SEND " + strPrimID + " SCP ERROR:  Failed to connect!";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intError=1;
        }
        session = libssh2_session_init();
        if(!session) {
            strLogMessage=" SEND " + strPrimID + " SCP ERROR:  Couldn't initiate SSH session.";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intError=1;
        }
        rc = libssh2_session_handshake(session, sock);
        if(rc) {
            strLogMessage=" SEND " + strPrimID + " SCP ERROR:  Failure establishing SSH session: " + to_string(rc);
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intError=1;
        }
        if(auth_pw) {
            /* We could authenticate via password */ 
            if(libssh2_userauth_password(session, strDestUser.c_str(), strPasswd.c_str())) {
                strLogMessage=" SEND " + strPrimID + " Authentication by password failed for user " + strDestUser + " with pass " + strPasswd + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
        } else {
            /* Or by public key */ 
            #define HOME_DIR "/home/username/"
            if(libssh2_userauth_publickey_fromfile(session, strDestUser.c_str(), HOME_DIR ".ssh/id_rsa.pub", HOME_DIR ".ssh/id_rsa", strPasswd.c_str())) {
                strLogMessage=" SEND " + strPrimID + " Authentication by public key failed";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
        }
        channel = libssh2_scp_send64(session, strDestPath.c_str(), fileinfo.st_mode & 0777, (unsigned long)fileinfo.st_size, 1505225516l, 1505225516l);
        if(!channel) {
            char *errmsg;
            int errlen;
            int err = libssh2_session_last_error(session, &errmsg, &errlen, 0);
            strLogMessage=" SEND " + strPrimID + " Unable to open a session.  Error#" + to_string(err);
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intError=1;
        }
        if(intError == 0) {
            //fprintf(stderr, "SCP session waiting to send file\n");
            do {
                nread = fread(mem, 1, sizeof(mem), local);
                if(nread <= 0) {
                    //std::cout << "Reached end of file for " << strFullPath << std::endl;
                    /* end of file */ 
                    break;
                }
                ptr = mem;

                do {
                    /* write the same data over and over, until error or completion */ 
                    rc = libssh2_channel_write(channel, ptr, nread);

                    if(rc < 0) {
                        fprintf(stderr, "ERROR %d\n", rc);
                        break;
                    } else {
                        /* rc indicates how many bytes were written this time */ 
                        ptr += rc;
                        nread -= rc;
                    }
                } while(nread);
            } while(1);

            libssh2_channel_send_eof(channel);
            libssh2_channel_wait_eof(channel);
            libssh2_channel_wait_closed(channel);
        }
        libssh2_channel_free(channel);
        channel = NULL;
        while((channel = libssh2_channel_open_session(session)) == NULL && libssh2_session_last_error(session, NULL, NULL, 0) == LIBSSH2_ERROR_EAGAIN) {
            waitsocket(sock, session);
        }
        if(channel == NULL) {
            strLogMessage=" SEND " + strPrimID + " Error???";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intError=1;
        }
        if(intError == 0) {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPATH" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strCmd="mv " + conf1.primConf[strRecNum + "_PRIDESTPATH"] + "/" + strDestPath + " " + conf1.primConf[strRecNum + "_PRIDESTPATH"] + "/";
            } else {
                strCmd="mv /pis/radconnect/extract/incoming/" + strDestPath + " /pis/radconnect/extract/incoming/";
            }
            intPos = strDestPath.find(".part");
            strDestPath.erase(intPos);
            strCmd+=strDestPath;
            strLogMessage = strPrimID + " " + strProcChainType + " Executing " + strCmd;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            while((rc = libssh2_channel_exec(channel, strCmd.c_str())) == LIBSSH2_ERROR_EAGAIN) { 
                waitsocket(sock, session);
            }
            if(rc != 0) {
                strLogMessage=" SEND " + strPrimID + " Error??????";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
        }
        if(session) {
            libssh2_session_disconnect(session, "Normal Shutdown");
            libssh2_session_free(session);
        }
        close(sock);
        if(local)
            fclose(local);
        //fprintf(stderr, "all done\n");
        libssh2_exit();
        strLogMessage=strPrimID + " SEND Finish SCP send to " + strDestHIP + " on port " + strDestPort;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        strDate = GetDate();
        strQuery="update send set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles) + ", serror=";
        strQuery+=to_string(intError) + " where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC) + ";";
        mysql_query(mconnect_local, strQuery.c_str());
        if(*mysql_error(mconnect_local)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect_local);
            strLogMessage+="\nstrQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        }
        strLogMessage=strPrimID + " SEND  Finished SCP.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    } else if (strSendType == "S3") {
        strPackFileName="none";
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPACK" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            if(conf1.primConf[strRecNum + "_PRIDESTPACK" + to_string(intLC)] == "tar.gz") {
                strPackFileName="*.tar";
            }
        }
        if(strPackFileName == "none") {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTCDCR" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "2") {
                    strPackFileName="*.ucr";
                } else if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "1") {
                    strPackFileName="*.j2k";
                } else {
                    strPackFileName="*.dcm";
                }
            } else {
                strPackFileName="*.dcm";
            }
        }
        //std::cout << "Building S3 send command." << std::endl;
        strCmd="aws s3 cp ";
        iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
        if(iprimConf != conf1.primConf.end()) {
            strCmd.append(conf1.primConf.find(strRecNum + "_PRIOUT")->second);
        } else {
            strReturn += "ERROR:  Could not determine the outbound directory.  Exiting..\n";
            intError=1;
        }
        strCmd+="/" + strPrimID + "/" + strPackFileName + " \"s3://";
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            strCmd.append(conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)]);
        } else {
            strReturn += "ERROR:  Could not determine the hostname or IP for this destination.  Exiting...\n";
            intError=1;
        }
        strCmd+="/";
        time_t t = time(0);
        struct tm * now = localtime( & t );
        strDate = std::to_string(now->tm_year + 1900);
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
        strCmd+=strDate + "/" + strPrimID + "/" + strPackFileName + "\" --no-progress";
        strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
        strQuery+=", tdest='S3:" + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "'";
        strQuery+=", tstartsend='" + GetDate() + "', tdestnum=" + to_string(intLC) + ";";
        mysql_query(mconnect_local, strQuery.c_str());
        if(intError == 0) {
            strLogMessage=" SEND " + strPrimID + " Executing " + strCmd + ".";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            system(strCmd.c_str());
        } else {
            strReturn += "There was an error.  Exiting...\n";
        }
        strCmd2="aws s3 ls " + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "/" + strDate ;
        strCmd2+="/" + strPrimID + "/|wc -l";
        //redi::ipstream proc(strCmd, redi::pstreams::pstderr);
        redi::ipstream proc(strCmd2);
        while (std::getline(proc, strReadLine)) {
            strReturn.append(strReadLine);
        }
        intNumFiles=stoi(strReturn);
        if(intNumFiles < 1) {
            intError=1;
            strLogMessage = strPrimID + " " + strProcChainType + " s3 failed with command " + strCmd;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        }
        strDate=GetDate();
        strQuery="update send set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles) + ", serror=";
        strQuery+=to_string(intError) + " where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC) + ";";
        mysql_query(mconnect_local, strQuery.c_str());
    } else {
        //This should be DICOM...
        strSendType == "DICOM";
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTCDCR" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "2") {
                strPackFileName="*.ucr";
            } else if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "1") {
                strPackFileName="*.j2k";
            } else {
                strPackFileName="*.dcm";
            }
        } else {
            strPackFileName="*.dcm";
        }
        strCmd = "ls -l " + conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/*" + strPackFileName + "|wc -l";
        strReturn = exec(strCmd.c_str());
        if(stoi(strReturn) < 1) {
            //std::cout << "strCmd = " << strCmd << "." << std::endl;
            //std::cout << "strReturn = " << strReturn << "." << std::endl;
            strQuery="select serror from send where puid = '" + strPrimID + "'  and tdestnum = " + to_string(intLC) + " and tendsend is not NULL order by tstartsend desc limit 1;";
            //std::cout << "strQuery: " << strQuery << std::endl;
            mysql_query(mconnect_local, strQuery.c_str());
            if(*mysql_error(mconnect_local)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect_local);
                strLogMessage+="strQuery = " + strQuery + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            }
            result = mysql_store_result(mconnect_local);
            if(result) {
                intNumRows = mysql_num_rows(result);
                if(intNumRows > 0) {
                    row = mysql_fetch_row(result);
                    strReturn=row[0];
                    //std::cout << "strReturn: " << strReturn << std::endl;
                    mysql_free_result(result);
                    sstream.clear();
                    sstream.str(strReturn);
                    sstream >> intErrors;
                    intErrors++;
                } else {
                    intErrors = 1;
                }
            } else {
                if(intCondDest != 1) {
                    strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
                    strQuery+=", tdest='DCM:" + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "'";
                    strQuery+=", tstartsend='" + strDate + "', tdestnum=" + to_string(intLC) + " serror=1;";
                } else {
                    strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
                    strQuery+=", tdest='DCM:" + strDestHIP + "'";
                    strQuery+=", tstartsend='" + strDate + "', tdestnum=" + to_string(intLC) + " serror=1;";
                }
                mysql_query(mconnect_local, strQuery.c_str());
                if(*mysql_error(mconnect_local)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect_local);
                    strLogMessage+="strQuery = " + strQuery + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                }
            }
            strQuery="update send set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles);
            strQuery+=", serror = " + to_string(intErrors) + " where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC);
            strQuery+=" and tendsend is NULL;";
            strLogMessage = strPrimID + " SEND WARN:  There are no " + strPackFileName + " files to send.  Skipping...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            //std::cout << strLogMessage << std::endl;
            return 1;
        }
        //strCmd = "storescu -ll ";
        strCmd = "dcmsend -ll ";
        iprimConf = conf1.primConf.find(strRecNum + "_PRILL");
        if(iprimConf != conf1.primConf.end()) {
            strCmd.append(conf1.primConf.find(strRecNum + "_PRILL")->second);
        } else {
            strCmd.append("debug");
        }
        //strCmd.append(" -xf /home/dicom/bin/storescu.cfg Default");
        strCmd.append(" -dn -nh +sd -nuc ");
        //Hack for IV
        strQuery="select senderAET from receive where puid='" + strPrimID + "';";
        mysql_query(mconnect_local, strQuery.c_str());
        result = mysql_store_result(mconnect_local);
        row = mysql_fetch_row(result);
        strAEC=row[0];
        if(strAEC == "" || strAEC == " ") {
            strAEC=strAET;
        }
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTAEMAP" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            strLogMessage = strPrimID + " SEND Searching map table for client ID of " + strAEC + "...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            //std::cout << strLogMessage << std::endl;
            if(conf1.primConf[strRecNum + "_PRIDESTAEMAP" + to_string(intLC)] == "1") {
                std::ifstream fpAEMap("/var/spool/primal_ae_map.txt");
                while (std::getline(fpAEMap, strLine)) {
                    intFound=strLine.find(",");
                    intFound2=strLine.find(",", intFound + 1);
                    strClientID = strLine.substr(0, intFound);
                    strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
                    strClientName = strLine.substr(intFound2 + 1);
                    intFound=strAEC.find(strClientID);

                    //std::cout << "ClientID = " << strClientID << " strClientAET = " << strClientAET  << " strClientName = " << strClientName << std::endl;
                    intFound2=strAEC.find(strClientID);
                    if(intFound2 != std::string::npos) {
                        strAEC = strClientAET;
                        //std::cout << "Found " << strClientID << " destiantion AEC is now " << strAEC << std::endl;
                        strCmd.append(" -aet " + strAEC + " -aec " + strAET + " ");
                        intAETMapped=1;
                    }
                }
                fpAEMap.close();
                if(intAETMapped != 1) {
                    strCmd.append(" -aet " + strAEC + " -aec ");
                }
            } else if(conf1.primConf[strRecNum + "_PRIDESTAEMAP" + to_string(intLC)] == "2") {
                strLogMessage = strPrimID + " SEND Checking for package.conf file.";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                //std::cout << strLogMessage << std::endl;
                if(fs::exists(conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/package.conf")) {
                    fstream fpPackage;
                    fpPackage.open(conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/package.conf", ios::in);
                    if(fpPackage.is_open()) {
                        while(getline(fpPackage, strLine)) {
                            intPos = strLine.find("CALLED_AE");
                            if(intPos != std::string::npos) {
                                strLine2=strLine;
                                intPos = strLine2.find_last_of(" ");
                                if(intPos != std::string::npos) {
                                    strAEC = strLine2.substr(intPos +1);
                                    strLogMessage = strPrimID + "SEND Found " + strLine2 + " destiantion AEC is now " + strAEC;
                                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                                    //std::cout << strLogMessage << std::endl;
                                    strCmd.append(" -aec " + strAEC + " -aet " + strAET + " ");
                                    intAETMapped=1;
                                }
                                break;
                            }
                        }
                    }
                    fpPackage.close();
                } else {
                    strLogMessage = strPrimID + " SEND Could not find " + conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/package.conf.  Using APEX_1101.";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                    //std::cout << strLogMessage << std::endl;
                    strCmd.append(" -aec APEX_1101 -aet " + strAET + " ");
                }
            }
        } else {
            strCmd.append(" -aet " + strAET + " -aec ");
        }
        if (intAETMapped != 1) {
            if(intCondDest != 1) {
                iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTAEC" + to_string(intLC));
                if(iprimConf != conf1.primConf.end()) {
                    strCmd.append(conf1.primConf[strRecNum + "_PRIDESTAEC" + to_string(intLC)]);
                } else {
                    strCmd.append(" DEFAULT ");
                }
            } else {
                strCmd.append(strAEC);
            }
        }
        strCmd += " +crf " + conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/" + strPrimID + ".info ";
        //strCmd.append(" +rn ");
        if (intCondDest != 1) {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC))->second);
            } else {
                strReturn += "ERROR:  Could not determine the destination host or IP.  Exiting...\n";
                intError=1;
            }
        } else {
            strCmd.append(strDestHIP);
        }
        strCmd.append(" ");
        if (intCondDest != 1) {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPORT" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRIDESTPORT" + to_string(intLC))->second);
            } else {
                strCmd.append("104");
            }
        } else {
            strCmd.append(strDestPort);
        }
        strCmd.append(" ");
        iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
        if(iprimConf != conf1.primConf.end()) {
            strCmd.append(conf1.primConf.find(strRecNum + "_PRIOUT")->second);
        } else {
            strReturn += "ERROR:  Could not determine the outbound directory.  Exiting..\n";
            intError=1;
        }
        strCmd.append("/");
        strCmd.append(strPrimID + "/" + strPackFileName + " >> ");
        iprimConf = conf1.primConf.find(strRecNum + "_PRILOGDIR");
        if(iprimConf != conf1.primConf.end()) {
            strCmd.append(conf1.primConf.find(strRecNum + "_PRILOGDIR")->second);
        } else {
            strReturn += "ERROR:  Could not determine the log directory.  Using /var/log/primal/\n";
            strCmd.append("/var/log/primal");
        }
        strCmd.append("/");
        iprimConf = conf1.primConf.find(strRecNum + "_PRILFOUT");
        if(iprimConf != conf1.primConf.end()) {
            strCmd.append(conf1.primConf.find(strRecNum + "_PRILFOUT")->second);
        } else {
            strReturn += "ERROR:  Could not determine the log file.  Using out.log\n";
            strCmd.append("out.log");
        }
        strCmd.append(" 2>&1");
        strDate=GetDate();
        if (intCondDest != 1) {
            strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
            strQuery+=", tdest='DCM:" + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "'";
            strQuery+=", tstartsend='" + strDate + "', tdestnum=" + to_string(intLC) + ";";
        } else {
            strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
            strQuery+=", tdest='DCM:" + strDestHIP + "'";
            strQuery+=", tstartsend='" + strDate + "', tdestnum=" + to_string(intLC) + ";";
        }
        mysql_query(mconnect_local, strQuery.c_str());
        //std::cout << "strCmd = " << strCmd << std::endl;
        if(intError == 0) {
            //std::cout << "Executing " << strCmd << "." << std::endl;
            strLogMessage="strCmd = " + strCmd + ".";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            intPos=system(strCmd.c_str());
            //std::cout << "Return = " << intPos << std::endl;
        } else {
            //std::cout << "There was an error.  Exiting..." << std::endl;
        }
        strQuery="select rec_images from receive where puid='" + strPrimID + "';";
        mysql_query(mconnect_local, strQuery.c_str());
        result = mysql_store_result(mconnect_local);
        row = mysql_fetch_row(result);
        strReturn=row[0];
        intImgRec = stoi(strReturn);
        //Count the number of good sends
        intNumFiles=0;
        intError=0;
        strTemp=conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/" + strPrimID + ".info";
        //std::cout << "Trying to read " << strTemp << std::endl;
        std::ifstream fpSendInfo(strTemp);
        while (std::getline(fpSendInfo, strLine)) {
            intPos=strLine.find("with status SUCCESS");
            if(intPos != std::string::npos) {
                //std::cout << "Found status line" << std::endl;
                intPos = strLine.find_last_of(" ");
                if(intPos != std::string::npos) {
                    strTemp = strLine.substr(intPos + 1);
                    intNumFiles=stoi(strTemp);
                    //std::cout << "Num files: " << intNumFiles << std::endl;
                } else {
                    intNumFiles=0;
                }
            } else {
                intNumFiles=0;
            }
        }
        fpSendInfo.close();
        strDate = GetDate();
        if(intNumFiles < intImgRec) {
            strQuery="select serror from send where puid = '" + strPrimID + "'  and tdestnum = " + to_string(intLC) + " and tendsend is not NULL order by tstartsend desc limit 1;";
            std::cout << "strQuery: " << strQuery << std::endl;
            mysql_query(mconnect_local, strQuery.c_str());
            if(*mysql_error(mconnect_local)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect_local);
                strLogMessage+="strQuery = " + strQuery + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            }
            result = mysql_store_result(mconnect_local);
            if(result) {
                intNumRows = mysql_num_rows(result);
                if(intNumRows > 0) {
                    row = mysql_fetch_row(result);
                    strReturn=row[0];
                    std::cout << "strReturn: " << strReturn << std::endl;
                    mysql_free_result(result);
                    sstream.clear();
                    sstream.str(strReturn);
                    sstream >> intErrors;
                    intErrors++;
                } else {
                    intErrors = 1;
                }
            }
            strQuery="update send set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles);
            strQuery+=", serror = " + to_string(intErrors) + " where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC);
            strQuery+=" and tendsend is NULL;";
            //strQuery="update send INNER JOIN (SELECT tstartsend from send where puid = '" + strPrimID + "' and tdestnum = " to_string(intLC);
            //strQuery+=" order by tstartsend desc limit 1) as t2 ";
            //strQuery+="using (tstartsend) set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles);
            //strQuery+=", serror = serror + 1 where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC) + ";";
        } else {
            strQuery="update send set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles);
            strQuery+=", serror = 0 where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC) + " and tendsend is NULL;";
        }
        mysql_query(mconnect_local, strQuery.c_str());
        if(*mysql_error(mconnect_local)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect_local);
            strLogMessage+="strQuery = " + strQuery + ".";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        }
    }
    strLogMessage=strPrimID + " SEND  Finished send...";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    mysql_close(mconnect_local);
    return intRetryStatus;
}

std::size_t fUpdateLocation(std::string strPrimalID, std::string strPriOut, std::string strRecNum) {
    std::string strFullPath, strQuery, strFileName, strResult, strDCMdump, strSopIUID, serSerIUID, strLogMessage, strFullFilePath;
    std::size_t intPos, intResult, intNumRows, intIsFound = 0;
    MYSQL *mconnect_local;
    MYSQL_ROW row;

    mconnect_local=mysql_init(NULL);
    mysql_options(mconnect_local,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect_local) {
        strLogMessage = "SEND MySQL initilization failed in fUpdateLocation function.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    mconnect_local=mysql_real_connect(mconnect_local, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect_local) {
        strLogMessage = "SEND MySQL connection failed in fUpdateLocation function.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    MYSQL_RES *result = mysql_store_result(mconnect_local);
    //strLogMessage=strPrimalID + " SEND  Updating location of images in DB...";
    //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    strFullPath=conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimalID;
    if(fs::exists(strFullPath)) {
        for (const auto & entry : fs::directory_iterator(strFullPath)) {
            strFullFilePath=entry.path().string();
            intPos=strFullFilePath.find_last_of("/");
            if(intPos != std::string::npos) {
                strFileName=strFullFilePath.substr(intPos + 1);
            } else {
                strFileName=strFullFilePath;
            }
            intPos=strFileName.find(".dcm");
            if(intPos != std::string::npos) {
                strQuery = "select count(*) from image where puid='" + strPrimalID + "' and ifilename='" + strFileName + "';";
                mysql_query(mconnect_local, strQuery.c_str());
                if(*mysql_error(mconnect_local)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect_local);
                    strLogMessage+="\nstrQuery = " + strQuery;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                }
                result = mysql_store_result(mconnect_local);
                if(result) {
                    intNumRows = mysql_num_rows(result);
                    if(intNumRows > 0) {
                        row = mysql_fetch_row(result);
                        strResult=row[0];
                        intResult=stoi(strResult);
                        if (intResult > 0) {
                            strQuery = "update image set ilocation='" + strPriOut + "/" + strPrimalID + "' where puid = '";
                            strQuery += strPrimalID + "' and ifilename='" + strFileName + "';";
                            mysql_query(mconnect_local, strQuery.c_str());
                            intIsFound = 1;
                        } else {
                            intIsFound = 0;
                        }
                    }
                    mysql_free_result(result);
                }
                if(intIsFound == 0) {
                    //strLogMessage=strPrimalID + " SEND  Adding image location for " + strFileName + ".";
                    //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                    strDCMdump=fDcmDump(strFullFilePath);
                    strSopIUID=fGetTagValue("0008,0018", strDCMdump, 0);
                    serSerIUID=fGetTagValue("0020,000e", strDCMdump, 0);
                    GetDate();
                    strQuery = "insert into image set ilocation='" + strPriOut + "/" + strPrimalID + "', puid='" + strPrimalID;
                    strQuery += "', SOPIUID='" + strSopIUID + "', SERIUID='" + serSerIUID + "', iservername='" + strHostname;
                    strQuery += "', ifilename='" + strFileName + "', idate='" + GetDate() + "';";
                    mysql_query(mconnect_local, strQuery.c_str());
                    if(*mysql_error(mconnect_local)) {
                        strLogMessage="SQL Error: ";
                        strLogMessage+=mysql_error(mconnect_local);
                        strLogMessage+="\nstrQuery = " + strQuery;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                    }
                }
            }
        }
    }
    //strLogMessage=strPrimalID + " SEND  DB update complete.";
    //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    mysql_close(mconnect_local);
    return 0;
}

std::size_t fEndSend(std::string strPrimalID, int intRetryStatus, std::size_t intDestNum) {
    std::string strQuery, strReturn, strRecNum, strTemp, strSource, strDest, strLogMessage;
    std::size_t intPos, intNumSent, intDirExists, intNumRows, intReturn, intNumDest = 0;
    std::map<std::string, std::string>::iterator iprimConf;
    std::stringstream sstream("1");
    MYSQL *mconnect_local;
    MYSQL_ROW row;
    MYSQL_RES *result;
    struct stat st;

    mconnect_local=mysql_init(NULL);
    mysql_options(mconnect_local,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect_local) {
        strLogMessage = "SEND MySQL initilization failed in fEndSend function.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    mconnect_local=mysql_real_connect(mconnect_local, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect_local) {
        strLogMessage = "SEND MySQL connection failed in fEndSend function.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }

    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intNumDest));
    while(iprimConf != conf1.primConf.end()) {
        intNumDest++;
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intNumDest));
    }
    strSource = conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimalID;
    intDirExists = stat(strSource.c_str(),&st);
    strQuery = "select count(*) from send where puid = '" + strPrimalID + "' and tdestnum = " + to_string(intDestNum) + " and serror=0 and complete=1 and tendsend is not NULL;";
    std::cout << "strQuery = " << strQuery << std::endl;
    mysql_query(mconnect_local, strQuery.c_str());
    if(*mysql_error(mconnect_local)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect_local);
        strLogMessage+="strQuery = " + strQuery + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    }
    result = mysql_store_result(mconnect_local);
    if(result) {
        intNumRows = mysql_num_rows(result);
        //std::cout << "intNumRows: " << to_string(intNumRows) << std::endl;
        if(intNumRows > 0) {
            row = mysql_fetch_row(result);
            strReturn=row[0];
            //std::cout << "strReturn: " << strReturn << std::endl;
            mysql_free_result(result);
            sstream.clear();
            sstream.str(strReturn);
            sstream >> intReturn;
        }
    }
    if(intReturn < 1) {
        if(intRetryStatus == 0 || intRetryStatus == 2) {
            if(intRetryStatus == 0) {
                strLogMessage = strPrimalID + " SEND " + "Not moving study because we have not exhausted all retries.";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            }
            return 0;
        }
    }
    if(intDirExists == 0 && intNumDest != 0) {
        strQuery="select count(*) from send where puid = '" + strPrimalID + "' and complete = 1 and serror = 0";
        mysql_query(mconnect_local, strQuery.c_str());
        MYSQL_RES *result = mysql_store_result(mconnect_local);
        row = mysql_fetch_row(result);
        strTemp=row[0];
        mysql_free_result(result);
        intNumSent = stoi(strTemp);
        //!!!This isn't going to work anymore.  Need to rework this check...
        if (intNumSent >= intNumDest) {
            strDest = conf1.primConf[strRecNum + "_PRISENT"] + "/" + strPrimalID;
            //fs::rename(strSource, strDest);
            strLogMessage = strPrimalID + " SEND " + "Copying from " + strSource + " to " + strDest + ".";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            fs::copy(strSource, strDest);
            fs::remove_all(strSource);
        } else {
            strDest = conf1.primConf[strRecNum + "_PRIERROR"] + "/" + strPrimalID;
            //fs::rename(strSource, strDest);
            fs::copy(strSource, strDest);
            fs::remove_all(strSource);
        }
    } else if(intDirExists == 0) {
        strDest = conf1.primConf[strRecNum + "_PRIERROR"] + "/" + strPrimalID;
            //fs::rename(strSource, strDest);
            fs::copy(strSource, strDest);
            fs::remove_all(strSource);
    } else {
        strLogMessage = strPrimalID + " SEND " + "Directory does not exist " + strSource + " for " + strPrimalID;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        mysql_close(mconnect_local);
        return 1;
    }
    intDirExists = stat(strDest.c_str(),&st);
    if(intDirExists == 0) {
        strLogMessage = strPrimalID + " SEND " + "Moved " + strPrimalID + " to " + strDest;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        strQuery = "update image set ilocation = '" + strDest + "' where puid = '" + strPrimalID + "';";
        mysql_query(mconnect_local, strQuery.c_str());
    } else {
        mysql_close(mconnect_local);
        return 2;
    }
    mysql_close(mconnect_local);
    return 0;
}

/*
void signal_handler(int signal) {
    //std::cout << "Rereading configuration files." << std::endl;
    //Need to reload configuration files
    mainDB.DBHOST.clear();
    mainDB.DBUSER.clear();
    mainDB.DBPASS.clear();
    mainDB.DBNAME.clear();
    mainDB.intDBPORT=0;
    ReadDBConfFile();
 
    //std::cout << signal << std::endl;
    conf1.primConf.erase(conf1.primConf.begin(), conf1.primConf.end());
    conf1.ReadConfFile();
    return;
}
*/

std::size_t fLoopSend(std::string strFullPath) {
    std::string strLogMessage, strPrimalID, strRecNum, strAET, strPackFileName;
    std::size_t intReturn, intPos, intDestNum, intDone;
    int intRetryStatus;
    std::map<std::string, std::string>::iterator iprimConf;

    mysql_thread_init();
    MYSQL *mconnect;
    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect) {
        strLogMessage="MySQL Initilization failed in fLoopSend.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="MySQL connection failed in fLoopSend.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    //Parse message
    //intPos=strFullPath.find_last_of(" ");
    //intMsgType=stoi(strFullPath.substr(intPos));
    //strFullPath.erase(intPos);
    if(strFullPath.back() == '/') {
        strFullPath.pop_back();
    }
    intPos = strFullPath.find_last_of("/");
    if(intPos != std::string::npos) {
        strPrimalID=strFullPath.substr(intPos + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);

    intDestNum=0;
    intDone=0;
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intDestNum));
    if(iprimConf == conf1.primConf.end()) {
        strLogMessage=" SEND " + strPrimalID + "  " + strRecNum + "_PRIDESTHIP" + to_string(intDestNum) + ".  Not found...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 0;
    }
    while(intDone == 0) {
        strAET = fPassThrough(strPrimalID, strRecNum);
        strAET.erase(std::remove(strAET.begin(), strAET.end(), '"'), strAET.end());
        strPackFileName="none";
        strLogMessage=strPrimalID + " SEND " + " Starting send for destination # " + to_string(intDestNum) + " primalID " + strPrimalID;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        fUpdateLocation(strPrimalID, conf1.primConf[strRecNum + "_PRIOUT"], strRecNum);
        intRetryStatus = fSendStudy(strPrimalID, strRecNum, intDestNum, strAET, strPackFileName);
        intReturn = fEndSend(strPrimalID, intRetryStatus, intDestNum);
        if(intReturn == 2) {
            strLogMessage =strPrimalID + " SEND " + " ERROR:  Tried to move " + strPrimalID + " but failed";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        } else if (intReturn == 1) {
            strLogMessage =strPrimalID + " SEND " + " ERROR:  Failed to move " + strPrimalID;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        }
        intDestNum++;
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intDestNum));
        if(iprimConf == conf1.primConf.end()) {
            intDone=1;
        }
    }
    mysql_close(mconnect);
    mysql_thread_end();
    return intReturn;
}

int main(int argc, char** argv) {
    //std::string strFullPath, strPrimalID, strQuery, strRow, strTemp, strJson, strSerIUID, strSerTemp, strTemp2, strFilename, strOldVal;
    //std::size_t intDone=0, intImgNum=0, intTemp;
    //std::map<std::string, std::string> mapSeriesJson;
    //MYSQL_ROW row;
    std::string strRecNum, strAET, strFileExt, strHostName, strDateTime, strFullPath, strCmd, strSendType, strPrimalID, strReturn;
    std::string strLogMessage, strCMD;
    std::map<std::string, std::string>::iterator iprimConf;

    mysql_library_init(0, NULL, NULL);
    ReadDBConfFile();
    conf1.ReadConfFile();
    strRecNum = "1";
    if(argc == 2) {
        strFullPath = argv[1];
        strLogMessage = "Starting prim_send_worker version 2.10.05 for " + strFullPath + ".";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        fLoopSend(strFullPath);
        mysql_library_end();
    } else {
        std::cout << "Wrong number of arguments: " << to_string(argc) << std::endl;
        strLogMessage = "Wrong number of arguments: " + to_string(argc);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    }
    return 0;
}