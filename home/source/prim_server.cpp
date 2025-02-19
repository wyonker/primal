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
#include <csignal>
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

MYSQL *mconnect;
MYSQL *mconnect2;

const std::string strVersionNum = "4.00.10";
const std::string strVersionDate = "2025-02-13";

//const std::string strProcChainType = "PRIMRCSEND";

//#include "prim_functions.h"

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
};

int fStartReceivers() {
    std::string strLogMessage, strQuery, strRecID, strRecNum, strServer, strType, strPort, strDir, strLog, strLL, strAET, strTO, strProcDir, strProcLog, strOutDir, strRecCompLevel, strOutLog, strSentDir, strHoldDir, strErrorDir, strDupe, strPassThr, strRetry, strCMD;
    int intNumRows;

    MYSQL_ROW row;
    MYSQL_RES *result;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage="MySQL Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return -1;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="MySQL connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return -1;
    }

    strQuery="SELECT * FROM conf_rec WHERE active = 1;";
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
                strRecID = row[0];
                strRecNum = row[1];
                strServer = row[2];
                strType = row[3];
                strPort = row[4];
                strDir = row[5];
                strLog = row[6];
                strLL = row[7];
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
                strLogMessage = "Starting to receive " + strRecNum + " from " + strServer + ".";
                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                //Need to start the receive process.
                if(strType == "DICOM") {
                    strCMD = "/home/dicom/bin/storescp --fork +cl " + strRecCompLevel + " -aet " + strAET + " -tos " + strTO + " -ll " + strLL + " -od " + strDir;
                    strCMD += " -ss " + strDir + " -xf /home/dicom/bin/storescp.cfg Default -fe \".dcm\" -xcr \"/home/dicom/rec.bash \"" + strRecID + " #p #a #c\"" + strPort + ">> " + strLog + " 2>&1 &";
                }
            }
        }
    }
    return 0;
}

void fEndReceive() {
    std::string strLogMessage, strQuery, strID, strPUID, strFullPath, strServerName, strRecID, strDateTime, strThisFilename, strTemp3, strRawDCMdump, strSerIUID, strSerDesc, strModality, strSopIUID, strStudyDateTime;
    std::string strQuery2;
    int intNumRows;
    std::size_t intPOS;
    std::vector<std::string> filenames;
    struct PatientData pData2;

    //Not ready yet.
    return;

    MYSQL_ROW row;
    MYSQL_RES *result;

    strQuery = "SELECT id, puid, fullpath, rservername, rec_id, tstartrec FROM receive WHERE complete = 0;";
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
                strFullPath = row[2];
                strServerName = row[3];
                strRecID = row[4];
                strDateTime = row[5];
            }
            //First let's see if the time out has been reached.

            const std::filesystem::path study{strFullPath};
            fs::directory_entry d1(strFullPath);
            if(d1.is_directory()) {
                strLogMessage = GetDate() + "   " + strPUID + " RECV  Ending receive";
                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                for (auto const& dir_entry : std::filesystem::directory_iterator{strFullPath}) {
                    intPOS=dir_entry.path().string().find_last_of("/");
                    if(intPOS != std::string::npos) {
                        strThisFilename = dir_entry.path().string().substr(intPOS+1);
                        intPOS=strThisFilename.find_last_of(".");
                        if(intPOS != std::string::npos) {
                            strTemp3=strThisFilename.substr(intPOS);
                        }
                        if(strTemp3 == ".dcm") {
                            strLogMessage = GetDate() + "   " + strPUID + " Getting tags from " + dir_entry.path().string();
                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                            //strRawDCMdump=fDcmDump(strFullPath.path().string());
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

                            //write to image
                        }
                    }
                }
                //write to study
                //strQuery2="INSERT INTO study SET puid = '" + strPUID + "', fullpath = '" + strFullPath + "', rservername = '" + hostname + "', rec_id = " + strRecID;
                //strQuery2+= ", tstartrec = '" + strDateTime + "', tendrec = '" + getDate() + "', senderAET = '" + strAET + "', callingAET = '" + strServerName + "', complete = 1;";
                //write to series
                //write to patient
                //write to receive
            }
        }
    }
    
    /*
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
    */
    
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
    int intProc_type, intNumRows, intConf_proc_id, intReturn;

    //Not ready yet.
    return;

    MYSQL_ROW row, row2, row3;
    MYSQL_RES *result, *result2, *result3;

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

                strQuery2="SELECT rec_id FROM reecieve WHERE puid = '" + strPUID + "';";
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
                    }
                    mysql_free_result(result3);
                }
            }
        }
    }
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

void fSend() {
    std::string strRecNum, strAET, strFileExt, strHostName, strDateTime, strFullPath, strCmd, strSendType, strPrimalID, strReturn;
    std::string strLogMessage, strCMD, strID, strPUID, strServerName, strDestNum, strDest, strOrg, strStartSend, strEndSend, strImages, strError, strRetry, strComplete;
    std::string strQuery, strQuery2, strQuery3, strQuery4, strLocation, strSendPort, strSendHIP, strSendAEC, strSendAET, strStatus;
    std::string strSendOrder, strSendPass, strSendRetry, strSendCompression, strSendTimeOut, strSendOrg, strSendName, strRecId;
    std::string strSendActive, strSendUser, strMPID, strAccn, strQuery5, strMPAccn, strNewAccn, strTime;

    int intNumRows, intStartSec, intNowSec, intSend=0;

    mysql_library_init(0, NULL, NULL);
    ReadDBConfFile();

    MYSQL_ROW row;
    MYSQL_ROW row2;
    MYSQL_ROW row3;
    MYSQL_ROW row5;

    MYSQL_RES *result;
    MYSQL_RES *result2;
    MYSQL_RES *result3;
    MYSQL_RES *result5;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage="MySQL Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="MySQL connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL 2nd Initilization failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), "mirth_primal", mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="MySQL 2nd connection failed.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        return;
    }

    strRecNum = "1";
    strLogMessage = "Starting prim_server version " + strVersionNum + ".";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    while (1) {
        strQuery = "SELECT send.id, send.puid, send.sservername, send.tdestnum, send.tdest, send.org, send.tstartsend, send.complete, study.AccessionNum FROM send LEFT JOIN study ON send.puid = study.puid WHERE send.complete = 5;";
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage="SQL Error: ";
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
                    strOrg = row[5];
                    strStartSend = row[6];
                    strComplete = row[7];
                    strAccn = row[8];
                    if(strAccn.size() > 3) {
                        strNewAccn = strAccn.substr(0, strAccn.size()-3);
                    }
                    strCMD = "date +\%s -d \"" + strStartSend + "\"";
                    intStartSec = stoi(exec(strCMD.c_str()));
                    strCMD = "date +\%s";
                    intNowSec = stoi(exec(strCMD.c_str()));
                    strTime = fSecToTime(intNowSec - intStartSec);
                    strLogMessage = strPUID + " Found " + strAccn + " truncated to " + strNewAccn + ", has been waiting to send for " + strTime + ".";
                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                    strQuery5="SELECT * FROM rec WHERE accn = '" + strNewAccn + "' AND send_status=0 ORDER BY rec_date DESC;";
                    mysql_query(mconnect2, strQuery5.c_str());
                    if(*mysql_error(mconnect2)) {
                        strLogMessage="SQL Error: ";
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
                                strLogMessage = strPUID + " Found " + strMPAccn + " HL7 message.  Sending...";
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
                                strLogMessage = strPUID + " " + strAccn + " has been waiting to send for more than 5 days.  Let's send";
                                fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                intSend = 2;
                            }
                        }
                        if(intSend > 0) {
                            strQuery2="SELECT * FROM conf_send WHERE conf_send_id = " + strDestNum + " limit 1;";
                            mysql_query(mconnect, strQuery2.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SQL Error: ";
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
                                    }
                                    mysql_free_result(result2);
                                }
                            }
                            strQuery3 = "SELECT DISTINCT ilocation FROM image WHERE puid = '" + strPUID + "';";
                            mysql_query(mconnect, strQuery3.c_str());
                            if(*mysql_error(mconnect)) {
                                strLogMessage="SQL Error: ";
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
                                    //Now we have all the info we need to send.  Let's build the command.  We need to do this for each ilocation.
                                    strLogMessage = strPUID + " Sending " + strAccn + " to " + strSendHIP + ".";
                                    fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                    strCMD = "dcmsend -ll debug -aet " + strSendAET + " -aec " + strSendAEC + " " + strSendHIP + " " + strSendPort + " " + strLocation + "/*.dcm 2>&1";
                                    //fWriteLog(strCMD, "/var/log/primal/primal.log");
                                    strStatus = exec(strCMD.c_str());
                                    strQuery4 = "UPDATE send SET complete = 1, tendsend = NOW() WHERE id = '" + strID + "';";
                                    mysql_query(mconnect, strQuery4.c_str());
                                    if(*mysql_error(mconnect)) {
                                        strLogMessage="SQL Error: ";
                                        strLogMessage+=mysql_error(mconnect);
                                        strLogMessage+="strQuery3 = " + strQuery4 + ".";
                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                    }
                                    strQuery4 = "DELETE FROM send WHERE puid = \"" + strPUID + "\" AND tdest = \"0\" limit 1;";
                                    mysql_query(mconnect, strQuery4.c_str());
                                    if(*mysql_error(mconnect)) {
                                        strLogMessage="SQL Error: ";
                                        strLogMessage+=mysql_error(mconnect);
                                        strLogMessage+="strQuery3 = " + strQuery4 + ".";
                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                    }
                                    //fWriteLog(strQuery4, "/var/log/primal/primal.log");
                                    strQuery4 = "UPDATE rec SET send_status = 1 WHERE id = '" + strMPID + "';";
                                    mysql_query(mconnect2, strQuery4.c_str());
                                    if(*mysql_error(mconnect2)) {
                                        strLogMessage="SQL Error: ";
                                        strLogMessage+=mysql_error(mconnect2);
                                        strLogMessage+="strQuery4 = " + strQuery4 + ".";
                                        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                                    }
                                    //fWriteLog(strQuery4, "/var/log/primal/primal.log");
                                }
                            }
                        }
                    }
                }
            }
        }
        intSend=0;
        strLogMessage = "Sleeping for 5 minutes.";
        fWriteLog(strLogMessage, "/var/log/primal/primal.log");
        std::this_thread::sleep_for (std::chrono::seconds(300));
    }

    mysql_library_end();
    return;
}

int main() {

    std::signal(SIGHUP, signal_handler);

    //fStartReceivers();
    //DCMTK will get the DICOM, just need to start working on it when it's done.
    std::thread receive(fEndReceive);
    std::thread process(fProcess);
    std::thread send(fSend);

    receive.join();
    process.join();
    send.join();
    
    return 0;
}