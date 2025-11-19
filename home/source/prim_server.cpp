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

/*
The intent of this new executable is to replace prim_recv, prim_process and prim_send.  
With a large chunck of stuff now being in the DB, let work needs to be done here.
*/

#undef min
#undef max
#include <algorithm>
#include <atomic>
#include <map>
#include <glob.h>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <pstreams/pstream.h>
//#include <mysql/my_global.h>
#include <mysql/mysql.h>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <dirent.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
    volatile sig_atomic_t do_shutdown = 0;
    std::atomic<bool> shutdown_requested = false;
    static_assert( std::atomic<bool>::is_always_lock_free );
}

std::mutex mtx;
using namespace std;
using file_time_type = std::chrono::time_point<std::chrono::file_clock>;
namespace fs = std::filesystem;
std::vector<std::string > vecRCcon1;
std::vector<std::string > vecRCopt1;
std::vector<std::string > vecRCcon2;
std::vector<std::string > vecRCact1;

const std::string strVersionNum = "4.02.07";
const std::string strVersionDate = "2025-09-30";

//const std::string strProcChainType = "PRIMRCSEND";

//#include "prim_functions.h"

std::vector<int> vecPIDs;

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

struct DataBase {
    std::string DBTYPE, DBNAME, DBUSER, DBPASS, DBHOST;
    int intDBPORT;
} mainDB;

struct PatientData {
    std::string strPName, strLastPName;
    std::string strPUID, strLastPUID;
    std::string strSEX, strLastSEX;
    std::string strMRN, strLastMRN;
    std::string strDOB, strLastDOB;
    std::string strPatientComments, strLastPatientComments;
    std::string strACCN, strLastACCN;
    std::string strMod, strLastMod;
    std::string strSIUID="NULL", strLastSIUID;
    std::string strStudyDate, strLastStudyDate;
    std::string strStartDate, strStudyTime;
    std::string strStudyDesc, strLastStudyDesc;
    time_t tmStartDT; //In seconds since Epoch
    time_t tmEndDT; //In seconds since Epoch
    std::string strEndDate;
    std::size_t intNumFiles, intLastNumFiles;
    std::size_t intEOS, intStartReceive=0;
    std::string strPath;
    std::string strDirName;
    std::string strRec;
    std::string strDest;
    std::string strStartRec, strLastStartRec;
    std::string strEndRec, strLastEndRec;
    std::size_t intIsError;
    std::string calledAETitle;
    std::string callingAETitle;
    std::string lastStudyInstanceUID;
    std::size_t intMoved=0, intLastMoved=0;
    std::string strRequestedProcedureID;
} pData;

std::string GetDate() {
    time_t t = time(0);
    struct tm * now = localtime( & t );
    std::string strDate;

    strDate = std::to_string(now->tm_year + 1900);
    strDate += "-";
    if ((now->tm_mon + 1) < 10) {
        strDate += "0";
        strDate += to_string(now->tm_mon + 1);
    } else {
        strDate += to_string(now->tm_mon + 1);
    }
    strDate += "-";
    if ((now->tm_mday) < 10) {
        strDate += "0";
        strDate += to_string(now->tm_mday);
    } else {
        strDate += to_string(now->tm_mday);
    }
    strDate += " ";
    if (now->tm_hour < 10) {
        strDate += "0";
        strDate += to_string(now->tm_hour);
    } else {
        strDate += to_string(now->tm_hour);
    }
    strDate += ":";
    if (now->tm_min < 10) {
        strDate += "0";
        strDate += to_string(now->tm_min);
    } else {
        strDate += to_string(now->tm_min);
    }
    strDate += ":";
    if (now->tm_sec < 10) {
        strDate += "0";
        strDate += to_string(now->tm_sec);
    } else {
        strDate += to_string(now->tm_sec);
    }
    return strDate;
}

std::string fSecToTime(int intSec) {
    std::string strReturn;
    int intDay, intHour, intMin;

    if (intSec < 60) {
        strReturn = std::to_string(intSec) + "s";
    } else if(intSec < 3600) {
        intMin = intSec / 60;
        intSec = intSec % 60;
        strReturn = std::to_string(intMin) + "m " + std::to_string(intSec) + "s";
    } else if(intSec < 86400) {
        intHour = intSec / 3600;
        intSec = intSec % 3600;
        intMin = intSec / 60;
        intSec = intSec % 60;
        strReturn = std::to_string(intHour) + "h " + std::to_string(intMin) + "m " + std::to_string(intSec) + "s";
    } else {
        intDay = intSec / 86400;
        intSec = intSec % 86400;
        intHour = intSec / 3600;
        intSec = intSec % 3600;
        intMin = intSec / 60;
        intSec = intSec % 60;
        strReturn = std::to_string(intDay) + "d " + std::to_string(intHour) + "h " + std::to_string(intMin) + "m " + std::to_string(intSec) + "s";
    }
    return strReturn;
}

std::string exec(const char* cmd) {
    std::array<char, 256> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string fDcmDump(std::string strTemp) {
    std::string strCMD, strReturn, strReadLine;

    strCMD="/home/dicom/bin/dcmdump " + strTemp;
    redi::ipstream proc(strCMD, redi::pstreams::pstdout);
    //while (std::getline(proc.err(), strReadLine))
    while (std::getline(proc, strReadLine))
        strReturn.append(strReadLine + "\n");
    return strReturn;
}

std::size_t fWriteLog(std::string strLogMessage, std::string strLogFile) {
    std::ofstream fpLogFile;
    std::string strLogDate=GetDate();
    fpLogFile.open(strLogFile, std::ios_base::app);
    //std::cout << strLogDate << "   " << strLogMessage << std::endl;
    fpLogFile << strLogDate << "   " << strLogMessage << std::endl;
    fpLogFile << std::flush;
    fpLogFile.close();
    return 0;
}

std::string fGetTagValue(std::string strTagID, std::string strDcmDump, std::size_t intType, std::size_t intOrder){
    std::size_t intFound, intEOL, intBracket, intBracketClose;
    std::string strReturn, strTemp, strTagType;

    if(intOrder == 1) {
        intFound = strDcmDump.find_last_of(strTagID);
    } else {
        intFound = strDcmDump.find(strTagID);
    }
    if(intFound != std::string::npos) {
        intEOL = strDcmDump.find("\n", intFound);
        strTemp = strDcmDump.substr(intFound, intEOL-intFound);
        //Need to determine tag type
        strTagType = strDcmDump.substr(intFound + 11, 2);
        intFound = strTemp.find("(no value available)");
        if(intFound == std::string::npos) {
            //std::cout << "Searching for " << strTagID << " found at " << intFound << " newline at " << intEOL << std::endl;
            //std::cout << strTagID << " : " << strTagType << std::endl;
            intBracket = strTemp.find("[");
            if(intBracket != std::string::npos) {
                //Use brackets to capture just the value
                strTemp = strTemp.substr(intBracket);
                intBracketClose = strTemp.find("]");
                strTemp = strTemp.substr(1, intBracketClose - 1);
                strReturn = strTemp;
            } else {
                //No brackets so gotta guess with spaces
                intBracketClose = strTemp.find_first_of(" ", 14);
                strTemp = strTemp.substr(14, intBracketClose - 14);
                strReturn = strTemp;
            }
            if(strTagType == "DA" && intType != 1) {
                //This is a date
                strTemp = strReturn.substr(0, 4) + "-" + strReturn.substr(4, 2) + "-" + strReturn.substr(6,2);
                strReturn = strTemp;
            } else if (strTagType == "TM" && intType != 1) {
                //This is a time
                strTemp = strReturn.substr(0,2) + ":" + strReturn.substr(2,2) + ":" + strReturn.substr(4,2);
                strReturn = strTemp;
            }
            //std::cout << "Returning "  << strReturn << std::endl;
        } else {
            strReturn="null";
        }
    } else {
        strReturn="null";
    }
    return strReturn;
}

int ReadDBConfFile() {
    std::string strLine, strKey, strValue;
    std::size_t intPOS, intStartPOS, intEndConf=0;
    std::ifstream infile("/etc/primal/primal.db", std::ifstream::in);

    while(!infile.eof() && intEndConf != 1)
    {
        getline(infile,strLine); // Saves the line in STRING.
        intPOS=strLine.find("=");
        if(intPOS!=std::string::npos) {
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
            if (strKey.compare("DBTYPE") == 0) {
                mainDB.DBTYPE=strValue;
            } else if (strKey.compare("DBNAME") == 0) {
                mainDB.DBNAME=strValue;
            } else if (strKey.compare("DBUSER") == 0) {
                mainDB.DBUSER=strValue;
            } else if (strKey.compare("DBPASS") == 0) {
                mainDB.DBPASS=strValue;
            } else if (strKey.compare("DBHOST") == 0) {
                mainDB.DBHOST=strValue;
            } else if (strKey.compare("DBPORT") == 0) {
                mainDB.intDBPORT=atoi(strValue.c_str());
            }
        }
    }
    infile.close();

    return 0;
}

int fStartReceivers() {
    std::string strLogMessage, strQuery, strRecID, strRecNum, strServer, strType, strPort, strDir, strLog, strLL, strAET, strTO, strProcDir, strProcLog, strOutDir, strRecCompLevel, strOutLog, strSentDir;
    std::string strHoldDir, strErrorDir, strDupe, strPassThr, strRetry, strCMD, strStatus, strReturn, strLine;
    int intNumRows;
    std::vector<int> vecTemp;

    MYSQL *mconnect;
    MYSQL *mconnect2;
    ReadDBConfFile();

    MYSQL_ROW row, row2;
    MYSQL_RES *result, *result2;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage="SRECV  MySQL Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return -1;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="SRECV  MySQL connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return -1;
    }
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="SRECV  MySQL 2nd Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return -1;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), "mirth_primal", mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="SRECV  MySQL 2nd connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return -1;
    }

    strLogMessage="Starting the receive process.";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    //Get list of running storescp processes before we start
    strCMD = "ps -ef|grep /home/dicom/bin/storescp|grep -v grep|tr -s \" \"|cut -d \" \" -f2";
    strReturn = exec(strCMD.c_str());
    std::istringstream isLine(strReturn);
    while (std::getline(isLine, strLine)) {
        if(!strLine.empty()) {
            vecTemp.push_back(atoi(strLine.c_str()));
        }
    }

    strQuery="SELECT * FROM conf_rec WHERE active = 1 AND conf_name NOT LIKE '!%';";
    mysql_query(mconnect, strQuery.c_str());
    if(*mysql_error(mconnect)) {
        strLogMessage="SRECV SQL Error: ";
        strLogMessage+=mysql_error(mconnect);
        strLogMessage+="\nQuery: " + strQuery + "\n";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
    }
    result = mysql_store_result(mconnect);
    if(result) {
        intNumRows=mysql_num_rows(result);
        if(intNumRows > 0) {
            while((row = mysql_fetch_row(result))) {
                strRecID = row[0];
                strRecNum = row[1];
                strServer = row[2];
                strType = row[3];
                strPort = row[4];
                strDir = row[5];
                strLog = row[6];
                strLL = row[7];
                if(strLL == "1") {
                    strLL = "fatal";
                } else if(strLL == "2") {
                    strLL = "error";
                } else if(strLL == "3") {
                    strLL = "warn";
                } else if(strLL == "4") {
                    strLL = "info";
                } else if(strLL == "5") {
                    strLL = "debug";
                } else if(strLL == "6") {
                    strLL = "trace";
                } else {
                    strLL = "debug";
                }
                strAET = row[8];
                strTO = row[9];
                strProcDir = row[10];
                strProcLog = row[11];
                strOutDir = row[12];
                strRecCompLevel = row[13];
                strOutLog = row[14];
                strSentDir = row[15];
                strHoldDir = row[16];
                strErrorDir = row[17];
                strDupe = row[18];
                strPassThr = row[19];
                strRetry = row[20];
                strLogMessage = "SRECV  Starting to receive " + strRecNum + " from " + strServer + ".";
                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                //Need to start the receive process.
                if(strType == "1") {
                    strCMD = "/home/dicom/bin/storescp --fork +cl " + strRecCompLevel + " -aet " + strAET + " -tos " + strTO + " -ll " + strLL + " -od " + strDir;
                    strCMD += " -ss " + strRecID + " -xf /home/dicom/bin/storescp.cfg Default -fe \".dcm\" -xcr \"/home/dicom/rec.bash \\\"#p " + strRecID + " #a #c #f\\\" >> /var/log/primal/rec.log 2>&1 \" " + strPort + " >> " + strLog + " 2>&1 &";
                    strLogMessage = "SRECV  " + strCMD;
                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    strStatus = exec(strCMD.c_str());
                    //Get list of running storescp processes now
                    strCMD = "ps -ef|grep /home/dicom/bin/storescp|grep -v grep|tr -s \" \"|cut -d \" \" -f2";
                    strReturn = exec(strCMD.c_str());
                    std::istringstream isLine2(strReturn);
                    while (std::getline(isLine2, strLine)) {
                        strLine.erase(std::remove(strLine.begin(), strLine.end(), ' '), strLine.end());
                        if(!strLine.empty()) {
                            //auto it = std::find(vecTemp.begin(), vecTemp.end(), atoi(strLine.c_str()));
                            //if(it == vecTemp.end()) {
                            if(std::find(vecTemp.begin(), vecTemp.end(), atoi(strLine.c_str())) != vecTemp.end()) {
                                strLogMessage = "SRECV  WARN did not find a new PID.";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            } else {
                                //PID not found must be the one we just started
                                vecPIDs.push_back(atoi(strLine.c_str()));
                                strLogMessage = "SRECV  Started new storescp process with PID: " + strLine;
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                        }
                    }
                }
            }
        }
    }

    strLogMessage="SRECV  Finished the receive process.";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    mysql_thread_end();
    return 0;
}

int fRecShutdown() {
    std::string strLogMessage, strCMD;

    strLogMessage="Stopping the receive process.";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    //Now kill all the storescp processes we started
    for (auto pid : vecPIDs) {
        strCMD = "kill -9 " + std::to_string(pid);
        strLogMessage = "Killing storescp process with PID: " + std::to_string(pid);
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        exec(strCMD.c_str());
    }
    strLogMessage="Finished stopping the processes.";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    return 0;
}

void fEndReceive() {
    std::string strLogMessage, strQuery, strID, strPUID, strFullPath, strServerName, strRecID, strDateTime, strThisFilename, strTemp3, strRawDCMdump, strSerIUID, strSerDesc, strModality, strSopIUID, strStudyDateTime;
    std::string strQuery2, strRecTimeout, strQuery3, strQuery4, strThisServerName;
    int intNumRows, intRecTimeout;
    std::vector<std::string> filenames;
    struct PatientData pData2;

    MYSQL *mconnect;
    MYSQL *mconnect2;
    ReadDBConfFile();

    MYSQL_ROW row, row2;
    MYSQL_RES *result, *result2;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage="RECV  MySQL Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="RECV  MySQL connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="RECV  MySQL 2nd Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), "mirth_primal", mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="RECV  MySQL 2nd connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }

    strThisServerName = exec("hostname");
    strThisServerName.pop_back(); // Remove trailing newline

    strLogMessage="Starting the end receive process.";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    while(1) {
        strQuery = "SELECT id, puid, fullpath, servername, rec_id, tstartrec FROM receive WHERE complete = 0 AND servername = '" + strThisServerName + "';";
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage="RECV  SQL Error: ";
            strLogMessage+=mysql_error(mconnect);
            strLogMessage+="\nQuery: " + strQuery + "\n";
            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        }
        result = mysql_store_result(mconnect);
        if(result) {
            intNumRows=mysql_num_rows(result);
            if(intNumRows > 0) {
                while((row = mysql_fetch_row(result))) {
                    strID = row[0];
                    strPUID = row[1];
                    strFullPath = row[2];
                    strServerName = row[3];
                    strRecID = row[4];
                    strDateTime = row[5];
                    //Get the timeout
                    strQuery2="SELECT rec_time_out FROM conf_rec WHERE conf_rec_id = \"" + strRecID + "\" limit 1;";
                    mysql_query(mconnect, strQuery2.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="RECV  SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="\nQuery: " + strQuery2 + "\n";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result2 = mysql_store_result(mconnect);
                    if(result2) {
                        while((row2 = mysql_fetch_row(result2))) {
                            strRecTimeout = row2[0];
                        }
                    }
                    try {
                        intRecTimeout = stoi(strRecTimeout);
                    } catch (...) {
                        strLogMessage = strPUID + " RECV  Invalid timeout value, using default of 30 seconds.";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        strLogMessage = strPUID + " RECV  Query: " + strQuery2;
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        intRecTimeout = 30;
                    }
                    //Let's see if the directory exists
                    if (!std::filesystem::exists(std::filesystem::path(strFullPath))) {
                        strLogMessage = strPUID + " RECV  Directory does not exist.  Setting receive to complete.";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        strQuery3="UPDATE receive SET complete=1, tendrec=NOW(), rerror=1 WHERE puid = \"" + strPUID + "\";";
                        mysql_query(mconnect, strQuery3.c_str());
                        continue;
                    }
                    //First let's see if the time out has been reached.
                    auto temp = std::filesystem::path(strFullPath);
                    auto ftime = fs::last_write_time(temp);
                    auto now = fs::file_time_type::clock::now();
                    //need to compare ftime to now and get the difference
                    std::chrono::seconds duration(intRecTimeout);
                    if ((now - ftime) > duration) {
                        strLogMessage = strPUID + " RECV  Ending receive";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        strQuery3="UPDATE receive SET complete=1, tendrec=NOW() WHERE puid = \"" + strPUID + "\";";
                        mysql_query(mconnect, strQuery3.c_str());
                        if(*mysql_error(mconnect)) {
                            strLogMessage="RECV  SQL Error: ";
                            strLogMessage+=mysql_error(mconnect);
                            strLogMessage+="\nQuery: " + strQuery3 + "\n";
                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        }
                        strQuery4="INSERT INTO process SET puid=\"" + strPUID + "\", servername=\"" + strServerName + "\", tstartproc=NOW(), complete=0;";
                        mysql_query(mconnect, strQuery4.c_str());
                        if(*mysql_error(mconnect)) {
                            strLogMessage="RECV  SQL Error: ";
                            strLogMessage+=mysql_error(mconnect);
                            strLogMessage+="\nQuery: " + strQuery4 + "\n";
                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        }
                    } else {
                        strLogMessage = strPUID + " RECV  Not yet time to end receive.";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                }
            }
        }
        if(do_shutdown) {
            mysql_library_end();
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    
    mysql_thread_end();
    return;
}


int fRuleTag(std::string strPUID, int intConf_proc_id) {
    (void) strPUID;
    (void) intConf_proc_id;
    return 0;
}

int fRuleDate(std::string strPUID, int intConf_proc_id) {
    (void) strPUID;
    (void) intConf_proc_id;
    return 0;
}

int fRuleTime(std::string strPUID, int intConf_proc_id) {
    (void) strPUID;
    (void) intConf_proc_id;
    return 0;
}

int fRuleDateTime(std::string strPUID, int intConf_proc_id) {
    (void) strPUID;
    (void) intConf_proc_id;
    return 0;
}

int fRuleScript(std::string strPUID, int intConf_proc_id) {
    (void) strPUID;
    (void) intConf_proc_id;
    return 0;
}

int fRuleHL7(std::string strPUID, int intConf_proc_id) {
    (void) strPUID;
    (void) intConf_proc_id;
    return 0;
}

void fProcess() {
    std::string strQuery, strQuery2, strQuery3, strLogMessage, strPUID, strID, strPservername, strTstartproc, strRecID, strConf_proc_id, strConf_rec_id, strProc_name, strProc_type;
    std::string strProc_tag, strProc_operator, strProc_cond, strProc_action, strProc_order, strProc_dest, strProc_active;
    int intProc_type, intNumRows, intConf_proc_id;
    [[maybe_unused]] int intReturn;

    MYSQL *mconnect;
    MYSQL *mconnect2;
    ReadDBConfFile();

    MYSQL_ROW row, row2, row3;
    MYSQL_RES *result, *result2, *result3;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage="SEND  MySQL Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="SEND  MySQL connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }

    strLogMessage="Starting the processing thread.";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    while(1) {
        strQuery="SELECT * FROM process WHERE complete = 0;";
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect);
            strLogMessage+="\nQuery: " + strQuery + "\n";
            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        }
        result = mysql_store_result(mconnect);
        if(result) {
            intNumRows=mysql_num_rows(result);
            if(intNumRows > 0) {
                while((row = mysql_fetch_row(result))) {
                    strID = row[0];
                    strPUID = row[1];
                    strPservername = row[2];
                    strTstartproc = row[3];

                    strLogMessage="Processing PUID: " + strPUID;
                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

                    strQuery2="SELECT rec_id FROM receive WHERE puid = '" + strPUID + "';";
                    mysql_query(mconnect, strQuery2.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="\nQuery: " + strQuery2 + "\n";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result2 = mysql_store_result(mconnect);
                    if(result2) {
                        intNumRows=mysql_num_rows(result2);
                        if(intNumRows > 0) {
                            while((row2 = mysql_fetch_row(result2))) {
                                strRecID = row2[0];
                            }
                        }
                        mysql_free_result(result2);
                    }

                    strQuery3="SELECT * FROM conf_proc WHERE conf_rec_id = " + strRecID + " ORDER BY proc_order;";
                    mysql_query(mconnect, strQuery3.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="\nQuery: " + strQuery3 + "\n";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result3 = mysql_store_result(mconnect);
                    if(result3) {
                        intNumRows=mysql_num_rows(result3);
                        if(intNumRows > 0) {
                            while((row3 = mysql_fetch_row(result3))) {
                                strConf_proc_id = row3[0];
                                intConf_proc_id = stoi(row3[0]);
                                strConf_rec_id = row3[1];
                                strProc_name = row3[2];
                                strProc_type = row3[3];
                                intProc_type = stoi(strProc_type);
                                strProc_tag = row3[4];
                                strProc_operator = row3[5];
                                strProc_cond = row3[6];
                                strProc_action = row3[7];
                                strProc_order = row3[8];
                                strProc_dest = row3[9];
                                strProc_active = row3[10];

                                if(intProc_type == 1) {
                                    //Tag modification
                                    intReturn = fRuleTag(strPUID, intConf_proc_id);
                                } else if(intProc_type == 2) {
                                    //Date
                                    intReturn = fRuleDate(strPUID, intConf_proc_id);
                                } else if(intProc_type == 3) {
                                    //Time
                                    intReturn = fRuleTime(strPUID, intConf_proc_id);
                                } else if(intProc_type == 4) {
                                    //Date-Time
                                    intReturn = fRuleDateTime(strPUID, intConf_proc_id);
                                } else if(intProc_type == 5) {
                                    //Script
                                    intReturn = fRuleScript(strPUID, intConf_proc_id);
                                } else if(intProc_type == 6) {
                                    //HL7
                                    intReturn = fRuleHL7(strPUID, intConf_proc_id);
                                }
                            }
                            strLogMessage="Processing done, ending processing.";
                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            strQuery = "UPDATE process SET complete=1, tendproc=NOW() WHERE PUID=\"" + strPUID + "\" limit 1;";
                            mysql_query(mconnect, strQuery.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="\nQuery: " + strQuery + "\n";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                            strQuery = "INSERT INTO send SET puid=\"" + strPUID + "\", servername=\"" + strPservername + "\", tstartsend=NOW(), complete=0;";
                            mysql_query(mconnect, strQuery.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="\nQuery: " + strQuery + "\n";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }    
                        } else {
                            strLogMessage="No processing to be done, ending processing.";
                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            strQuery = "UPDATE process SET complete=1, tendproc=NOW() WHERE PUID=\"" + strPUID + "\" limit 1;";
                            mysql_query(mconnect, strQuery.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="\nQuery: " + strQuery + "\n";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                            strQuery = "INSERT INTO send SET puid=\"" + strPUID + "\", servername=\"" + strPservername + "\", tstartsend=NOW(), complete=0;";
                            mysql_query(mconnect, strQuery.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="\nQuery: " + strQuery + "\n";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                        }
                        mysql_free_result(result3);
                    }
                }
            }
        }
        if(do_shutdown) {
            mysql_library_end();
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    mysql_thread_end();

}

void fSend() {
    std::string strRecNum, strAET, strFileExt, strHostName, strDateTime, strFullPath, strCmd, strSendType, strPrimalID, strReturn, strDate;
    std::string strLogMessage, strCMD, strID, strPUID, strServerName, strDestNum, strDest, strOrg, strStartSend, strEndSend, strImages, strError, strRetry, strComplete;
    std::string strQuery, strQuery2, strQuery3, strQuery4, strLocation, strSendPort, strSendHIP, strSendAEC, strSendAET, strStatus;
    std::string strSendOrder, strSendPass, strSendRetry, strSendCompression, strSendTimeOut, strSendOrg, strSendName, strRecId;
    std::string strSendActive, strSendUser, strMPID, strAccn, strQuery5, strMPAccn, strNewAccn, strTime, strDestLocation, strStudyID;

    int intNumRows, intStartSec, intNowSec, intDateCheck, intLC, intDone, intSend=0;

    MYSQL *mconnect;
    MYSQL *mconnect2;
    ReadDBConfFile();

    /*
    strLogMessage="mainDB.DBTYPE = " + mainDB.DBTYPE;
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
    strLogMessage="mainDB.DBNAME = " + mainDB.DBNAME;
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
    strLogMessage="mainDB.DBUSER = " + mainDB.DBUSER;
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
    strLogMessage="mainDB.DBHOST = " + mainDB.DBHOST;
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
    strLogMessage="mainDB.intDBPORT = " + std::to_string(mainDB.intDBPORT);
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
    */

    MYSQL_ROW row;
    MYSQL_ROW row2;
    MYSQL_ROW row3;
    MYSQL_ROW row4;
    MYSQL_ROW row5;

    MYSQL_RES *result;
    MYSQL_RES *result2;
    MYSQL_RES *result3;
    MYSQL_RES *result4;
    MYSQL_RES *result5;

    strLogMessage="Starting the send thread.";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage="SEND  MySQL Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    intLC=0;
    intDone=0;
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    while ((intLC <= 5)  && (intDone = 0)) {
        if (!mconnect) {
            if(intLC <= 5) {
                strLogMessage="SEND  MySQL connection failed.  'Trying again...";
                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
            } else {
                strLogMessage="SEND  MySQL connection failed.  'Out of retries!";
                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                return;
            }
            std::this_thread::sleep_for (std::chrono::seconds(3));
            mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
        } else {
            intDone=1;
        }
        intLC++;
    }
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="SEND  MySQL 2nd Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), "mirth_primal", mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="SEND  MySQL 2nd connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }

    strRecNum = "1";

    intLC=1;
    while (1) {
        //HL7 branch
        strQuery = "SELECT send.id, send.puid, send.servername, send.tdestnum, send.tdest, send.tstartsend, send.complete FROM send WHERE send.complete > 4;";
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage="SEND  SQL Error: ";
            strLogMessage+=mysql_error(mconnect);
            strLogMessage+="strQuery = " + strQuery + ".";
            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        }
        result = mysql_store_result(mconnect);
        if(result) {
            intNumRows = mysql_num_rows(result);
            if(intNumRows > 0) {
                while ((row = mysql_fetch_row(result))) {
                    strID = row[0];
                    strPUID = row[1];
                    strServerName = row[2];
                    strDestNum = row[3];
                    strDest = row[4];
                    strStartSend = row[5];
                    strComplete = row[6];
                    strQuery = "SELECT studyID FROM study_puid WHERE puid = \"" + strPUID + "\";";
                    mysql_query(mconnect, strQuery.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SEND  SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="strQuery = " + strQuery + ".";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result2 = mysql_store_result(mconnect);
                    if(result2) {
                        intNumRows = mysql_num_rows(result2);
                        if(intNumRows > 0) {
                            while ((row2 = mysql_fetch_row(result2))) {
                                strStudyID = row2[0];
                            }
                        }
                    }
                    strQuery = "SELECT AccessionNum, org FROM study WHERE studyID = \"" + strStudyID + "\";";
                    mysql_query(mconnect, strQuery.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SEND  SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="strQuery = " + strQuery + ".";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result3 = mysql_store_result(mconnect);
                    if(result3) {
                        intNumRows = mysql_num_rows(result3);
                        if(intNumRows > 0) {
                            while ((row3 = mysql_fetch_row(result3))) {
                                strAccn = row3[0];
                                strOrg = row3[1];
                            }
                        }
                    }
                    strLogMessage = strPUID + " SEND  Processing send for " + strAccn + ".";
                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    if(strAccn.size() > 3) {
                        strNewAccn = strAccn.substr(0, strAccn.size()-3);
                    }
                    strCMD = "date +\%s -d \"" + strStartSend + "\"";
                    strDate = exec(strCMD.c_str());
                    intDateCheck = !strDate.empty() && std::all_of(strDate.begin(), strDate.end(), ::isdigit);
                    if (intDateCheck == 0) {
                        intStartSec = stoi(strDate.c_str());
                    } else {
                        strLogMessage = strPUID + " SEND  WARN: Unable to determine start time.  Skipping...";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        continue;
                    }
                    //intStartSec = stoi(exec(strCMD.c_str()));
                    strCMD = "date +\%s";
                    intNowSec = stoi(exec(strCMD.c_str()));
                    strTime = fSecToTime(intNowSec - intStartSec);
                    if(intLC == 100) {
                        //Limit log message to every 5 minutes or whatever 100 loops adds up to.
                        strLogMessage = strPUID + " SEND  Found " + strAccn + " truncated to " + strNewAccn + ", has been waiting to send for " + strTime + ".";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    strQuery5="SELECT * FROM rec WHERE accn = '" + strNewAccn + "' AND send_status=0 ORDER BY rec_date DESC;";
                    mysql_query(mconnect2, strQuery5.c_str());
                    if(*mysql_error(mconnect2)) {
                        strLogMessage="SEND  SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="strQuery5 = " + strQuery5 + ".";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result5 = mysql_store_result(mconnect2);
                    if(result5) {
                        intNumRows = mysql_num_rows(result5);
                        if(intNumRows > 0) {
                            while ((row5 = mysql_fetch_row(result5))) {
                                //We have an unsent match in primal and mirth_primal.  Let's send it.
                                strMPID = row5[0];
                                strMPAccn = row5[2];
                                strLogMessage = strPUID + " SEND  Found " + strMPAccn + " HL7 message.  Sending...";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                intSend=1;
                            }
                        } else {
                            //Need to see if it's older than 3 days.
                            //strCMD = "date +\%s -d \"" + strStartSend + "\"";
                            //intStartSec = stoi(exec(strCMD.c_str()));
                            //strCMD = "date +\%s";
                            //intNowSec = stoi(exec(strCMD.c_str()));
                            if ((intNowSec - intStartSec) > 432000) {
                                strLogMessage = strPUID + " SEND  " + strAccn + " has been waiting to send for more than 5 days.  Let's send";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                intSend = 2;
                            }
                            if (strComplete == "6") {
                                strLogMessage = strPUID + " SEND  " + strAccn + " has been manually queued to send.  Let's send";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                intSend = 3;
                            }
                        }
                        if(intSend > 0) {
                            strQuery2="SELECT * FROM conf_send WHERE conf_send_id = " + strDestNum + " limit 1;";
                            mysql_query(mconnect, strQuery2.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SEND  SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="strQuery2 = " + strQuery2 + ".";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                            result2 = mysql_store_result(mconnect);
                            if(result2) {
                                intNumRows = mysql_num_rows(result2);
                                if(intNumRows > 0) {
                                    while ((row2 = mysql_fetch_row(result2))) {
                                        strRecId = row2[1];
                                        strSendName = row2[2];
                                        strSendOrg = row2[3];
                                        strSendAET = row2[4];
                                        strSendAEC = row2[5];
                                        strSendHIP = row2[6];
                                        strSendType = row2[7];
                                        strSendPort = row2[8];
                                        strSendTimeOut = row2[9];
                                        strSendCompression = row2[10];
                                        strSendRetry = row2[11];
                                        strSendUser = row2[12];
                                        strSendPass = row2[13];
                                        strSendOrder = row2[14];
                                        strSendActive = row2[15];
                                        strLogMessage = strPUID + " SEND  " + strAccn + " Sending to AET " + strSendAET;
                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                    }
                                    //mysql_free_result(result2);
                                } else {
                                    strLogMessage = strPUID + " SEND  " + strAccn + " No destination found.  Not sending.";
                                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                    continue;
                                }
                            } else {
                                strLogMessage = strPUID + " SEND  " + strAccn + " No results found for destination query.  Not sending.";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                continue;                               
                            }
                            if(strSendActive == "0") {
                                strLogMessage = strPUID + " SEND  " + strAccn + " " + strSendName + " is not active.  Not sending.";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                continue;
                            }
                            if(strRecId.empty()) {
                                strLogMessage = strPUID + " SEND  " + strAccn + " No receiving configuration assigned.  Not sending.";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                continue;
                            }
                            strQuery3 = "SELECT DISTINCT ilocation FROM image WHERE puid = '" + strPUID + "';";
                            mysql_query(mconnect, strQuery3.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SEND  SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="strQuery3 = " + strQuery3 + ".";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                            result3 = mysql_store_result(mconnect);
                            if(result3) {
                                intNumRows = mysql_num_rows(result3);
                                if(intNumRows > 0) {
                                    while ((row3 = mysql_fetch_row(result3))) {
                                        strLocation = row3[0];
                                    }
                                    if(strcmp(strOrg.c_str(), strSendOrg.c_str()) != 0) {
                                        strLogMessage = strPUID + " SEND  " + strAccn + " is not for this organization.  Not sending.";
                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                    } else {
                                        //Now we have all the info we need to send.  Let's build the command.  We need to do this for each ilocation.
                                        strLogMessage = strPUID + " SEND  Sending " + strAccn + " to " + strSendHIP + " at location " + strLocation + ".";
                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                        fWriteLog(strLogMessage, "/var/log/primal/prim_server_out.log");
                                        strCMD = "dcmsend -ll debug -aet " + strSendAET + " -aec " + strSendAEC + " " + strSendHIP + " " + strSendPort + " " + strLocation + "/*.dcm >> /var/log/primal/prim_server_out.log 2>&1";
                                        //fWriteLog(strCMD, "/var/log/primal/primal.log");
                                        strStatus = exec(strCMD.c_str());
                                        strLogMessage = strPUID + " SEND  Finished sending " + strAccn + " to " + strSendHIP + ".";
                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                        strQuery4 = "UPDATE send SET complete = 1, tendsend = NOW() WHERE id = '" + strID + "';";
                                        mysql_query(mconnect, strQuery4.c_str());
                                        if(*mysql_error(mconnect)) {
                                            strLogMessage="SEND  SQL Error: ";
                                            strLogMessage+=mysql_error(mconnect);
                                            strLogMessage+="strQuery3 = " + strQuery4 + ".";
                                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                        }
                                        strQuery4 = "DELETE FROM send WHERE puid = \"" + strPUID + "\" AND tdest = \"0\" limit 1;";
                                        mysql_query(mconnect, strQuery4.c_str());
                                        if(*mysql_error(mconnect)) {
                                            strLogMessage="SEND  SQL Error: ";
                                            strLogMessage+=mysql_error(mconnect);
                                            strLogMessage+="strQuery3 = " + strQuery4 + ".";
                                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                        }
                                        //fWriteLog(strQuery4, "/var/log/primal/primal.log");
                                        strQuery4 = "UPDATE rec SET send_status = 1 WHERE id = '" + strMPID + "';";
                                        mysql_query(mconnect2, strQuery4.c_str());
                                        if(*mysql_error(mconnect2)) {
                                            strLogMessage="SEND  SQL Error: ";
                                            strLogMessage+=mysql_error(mconnect2);
                                            strLogMessage+="strQuery4 = " + strQuery4 + ".";
                                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                        }
                                        //fWriteLog(strQuery4, "/var/log/primal/primal.log");
                                        strQuery = "SELECT sent_directory FROM conf_rec WHERE conf_rec_id = " + strRecId + " limit 1;";
                                        mysql_query(mconnect, strQuery.c_str());
                                        if(*mysql_error(mconnect)) {
                                            strLogMessage="SEND  SQL Error: ";
                                            strLogMessage+=mysql_error(mconnect);
                                            strLogMessage+="strQuery = " + strQuery + ".";
                                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                        }
                                        result4 = mysql_store_result(mconnect);
                                        if(result4) {
                                            intNumRows = mysql_num_rows(result4);
                                            if(intNumRows > 0) {
                                                while ((row4 = mysql_fetch_row(result4))) {
                                                    strDestLocation = row4[0];
                                                    const char* source_dir = strLocation.c_str();
                                                    const char* destination_dir = strDestLocation.c_str();
                                                    if (std::rename(source_dir, destination_dir) != 0) {
                                                        strLogMessage = "Failed to move directory";
                                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                                    } else {
                                                        strLogMessage = "Successfully moved " + std::string(source_dir) + " directory to " + std::string(destination_dir) + ".";
                                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                                    }
                                                }
                                            }
                                        }
                                        //Need to move study to sent directory
                                    }
                                } else {
                                    strLogMessage = strPUID + " SEND  " + strAccn + " No images found.  Not sending.";
                                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                }
                            } else {
                                strLogMessage = strPUID + " SEND  " + strAccn + " No results found for image query.  Not sending.";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                continue;
                            }
                        }
                    }

                    strLogMessage = strPUID + " SEND  Send processing complete.";
                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                }
            }
        }
        //Regular branch
        strQuery = "SELECT id, puid, servername, tdestnum, tdest, tstartsend, complete FROM send WHERE complete = 0;";
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage="SEND  SQL Error: ";
            strLogMessage+=mysql_error(mconnect);
            strLogMessage+="strQuery = " + strQuery + ".";
            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        }
        result = mysql_store_result(mconnect);
        if(result) {
            intNumRows = mysql_num_rows(result);
            if(intNumRows > 0) {
                while ((row = mysql_fetch_row(result))) {
                    strID = row[0];
                    strPUID = row[1];
                    strServerName = row[2];
                    strDestNum = row[3];
                    strDest = row[4];
                    strStartSend = row[5];
                    strComplete = row[6];
                    strQuery = "SELECT studyID FROM study_puid WHERE puid = \"" + strPUID + "\";";
                    mysql_query(mconnect, strQuery.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SEND  SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="strQuery = " + strQuery + ".";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result2 = mysql_store_result(mconnect);
                    if(result2) {
                        intNumRows = mysql_num_rows(result2);
                        if(intNumRows > 0) {
                            while ((row2 = mysql_fetch_row(result2))) {
                                strStudyID = row2[0];
                            }
                        }
                    }
                    strQuery = "SELECT AccessionNum, org FROM study WHERE studyID = \"" + strStudyID + "\";";
                    mysql_query(mconnect, strQuery.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SEND  SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="strQuery = " + strQuery + ".";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result3 = mysql_store_result(mconnect);
                    if(result3) {
                        intNumRows = mysql_num_rows(result3);
                        if(intNumRows > 0) {
                            while ((row3 = mysql_fetch_row(result3))) {
                                strAccn = row3[0];
                                strOrg = row3[1];
                            }
                        }
                    }

                    strLogMessage = strPUID + " SEND  Processing send for " + strAccn + ".";
                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    if(strAccn.size() > 3) {
                        strNewAccn = strAccn.substr(0, strAccn.size()-3);
                    }
                    strCMD = "date +\%s -d \"" + strStartSend + "\"";
                    strQuery2="SELECT * FROM conf_send WHERE conf_send_id = " + strDestNum + " limit 1;";
                    mysql_query(mconnect, strQuery2.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SEND  SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="strQuery2 = " + strQuery2 + ".";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result2 = mysql_store_result(mconnect);
                    if(result2) {
                        intNumRows = mysql_num_rows(result2);
                        if(intNumRows > 0) {
                            while ((row2 = mysql_fetch_row(result2))) {
                                strRecId = row2[1];
                                strSendName = row2[2];
                                strSendOrg = row2[3];
                                strSendAET = row2[4];
                                strSendAEC = row2[5];
                                strSendHIP = row2[6];
                                strSendType = row2[7];
                                strSendPort = row2[8];
                                strSendTimeOut = row2[9];
                                strSendCompression = row2[10];
                                strSendRetry = row2[11];
                                strSendUser = row2[12];
                                strSendPass = row2[13];
                                strSendOrder = row2[14];
                                strSendActive = row2[15];
                                strLogMessage = strPUID + " SEND  " + strAccn + " Sending to AET " + strSendAET;
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                            //mysql_free_result(result2);
                        } else {
                            strLogMessage = strPUID + " SEND  " + strAccn + " No destination found.  Not sending.";
                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            continue;
                        }
                    } else {
                        strLogMessage = strPUID + " SEND  " + strAccn + " No results found for destination query.  Not sending.";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        continue;                               
                    }
                    if(strSendActive == "0") {
                        strLogMessage = strPUID + " SEND  " + strAccn + " " + strSendName + " is not active.  Not sending.";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        continue;
                    }
                    if(strRecId.empty()) {
                        strLogMessage = strPUID + " SEND  " + strAccn + " No receiving configuration assigned.  Not sending.";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        continue;
                    }
                    strQuery3 = "SELECT DISTINCT ilocation FROM image WHERE puid = '" + strPUID + "';";
                    mysql_query(mconnect, strQuery3.c_str());
                    if(*mysql_error(mconnect)) {
                        strLogMessage="SEND  SQL Error: ";
                        strLogMessage+=mysql_error(mconnect);
                        strLogMessage+="strQuery3 = " + strQuery3 + ".";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    result3 = mysql_store_result(mconnect);
                    if(result3) {
                        intNumRows = mysql_num_rows(result3);
                        if(intNumRows > 0) {
                            while ((row3 = mysql_fetch_row(result3))) {
                                strLocation = row3[0];
                            }
                            if(strSendType == "1") {
                                //Dicom type
                                //Now we have all the info we need to send.  Let's build the command.  We need to do this for each ilocation.
                                strLogMessage = strPUID + " SEND  Sending " + strAccn + " to " + strSendHIP + " at location " + strLocation + ".";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                fWriteLog(strLogMessage, "/var/log/primal/prim_server_out.log");
                                strCMD = "dcmsend -ll debug -aet " + strSendAET + " -aec " + strSendAEC + " " + strSendHIP + " " + strSendPort + " " + strLocation + "/*.dcm >> /var/log/primal/prim_server_out.log 2>&1";
                                //fWriteLog(strCMD, "/var/log/primal/primal.log");
                                strStatus = exec(strCMD.c_str());
                            } else if (strSendType == "2") {
                                //SCP type
                            } else if (strSendType == "3") {
                                //FTP type
                            } else if (strSendType == "4") {
                                //Archive type
                                strLogMessage = strPUID + " SEND  Sending " + strAccn + " to archive at " + strSendHIP + " " + strSendPort + ".";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                fWriteLog(strLogMessage, "/var/log/primal/prim_server_out.log");
                                strCMD = "dcmsend -ll debug -aet " + strSendAET + " -aec " + strSendAEC + " " + strSendHIP + " " + strSendPort + " " + strLocation + "/*.dcm >> /var/log/primal/prim_server_out.log 2>&1";
                                strStatus = exec(strCMD.c_str());
                            }
                            strLogMessage = strPUID + " SEND  Finished sending " + strAccn + " to " + strSendHIP + ".";
                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            strQuery4 = "UPDATE send SET complete = 1, tendsend = NOW() WHERE id = '" + strID + "';";
                            mysql_query(mconnect, strQuery4.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SEND  SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="strQuery3 = " + strQuery4 + ".";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                            strQuery = "SELECT sent_directory FROM conf_rec WHERE conf_rec_id = " + strRecId + " limit 1;";
                            mysql_query(mconnect, strQuery.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SEND  SQL Error: ";
                                strLogMessage+=mysql_error(mconnect);
                                strLogMessage+="strQuery = " + strQuery + ".";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            }
                            result4 = mysql_store_result(mconnect);
                            if(result4) {
                                intNumRows = mysql_num_rows(result4);
                                if(intNumRows > 0) {
                                    while ((row4 = mysql_fetch_row(result4))) {
                                        strDestLocation = row4[0];
                                        const char* source_dir = strLocation.c_str();
                                        const char* destination_dir = strDestLocation.c_str();
                                        if (std::rename(source_dir, destination_dir) != 0) {
                                            strLogMessage = "Failed to move directory";
                                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                        } else {
                                            strLogMessage = "Successfully moved " + std::string(source_dir) + " directory to " + std::string(destination_dir) + ".";
                                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                        }
                                    }
                                }
                            }
                            //Need to move study to sent directory
                        }
                    } else {
                        strLogMessage = strPUID + " SEND  " + strAccn + " No images found.  Not sending.";
                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    }
                    strLogMessage = strPUID + " SEND  Send processing complete.";
                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                }
            }
        }
        mysql_free_result(result);
        if(do_shutdown) {
            mysql_library_end();
            return;
        }
        intSend=0;
        if(intLC == 100) {
            strLogMessage = "SEND  Sleeping for 5 minutes.";
            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        }
        std::this_thread::sleep_for (std::chrono::seconds(3));
        if(intLC > 99) {
            intLC=1;
        } else {
            intLC++;
        }
    }

    mysql_thread_end();
    return;
}

void signal_handler(int signal) {
    std::cout << "Rereading configuration files." << std::endl;
    //Need to reload configuration files
    mainDB.DBHOST.clear();
    mainDB.DBUSER.clear();
    mainDB.DBPASS.clear();
    mainDB.DBNAME.clear();
    mainDB.intDBPORT=0;
    ReadDBConfFile();
 
    std::cout << signal << std::endl;
    return;
}

void signal_handler2([[maybe_unused]] int signal) {
    do_shutdown = 1;
    shutdown_requested = true;
    const char str[] = "received signal\n";
    write(STDERR_FILENO, str, sizeof(str) - 1);

}

int main() {
    std::string strLogMessage;

    {
        struct sigaction action;
        action.sa_handler = signal_handler2;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGINT, &action, NULL);
    }

    strLogMessage = "Starting prim_server version " + strVersionNum + ".";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    mysql_library_init(0, NULL, NULL);
    
    fStartReceivers();
    //DCMTK will get the DICOM, just need to start working on it when it's done.
    std::thread receive(fEndReceive);
    std::thread process(fProcess);
    std::thread send(fSend);

    while( !do_shutdown ) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    fRecShutdown();
    receive.join();
    process.join();
    send.join();

    mysql_library_end();
    return 0;
}






