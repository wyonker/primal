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

const std::string strProcChainType = "PRIMRCQR";

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

std::string fGetClientID(std::string strAEC) {
    std::string strLine, strClientID, strReturn, strClientName, strClientAET;
    std::size_t intFound, intFound2;

    strReturn="NULL";
    std::ifstream fpAEMap("/var/spool/primal_ae_map.txt");
    while (std::getline(fpAEMap, strLine)) {
        intFound=strLine.find(",");
        intFound2=strLine.find(",", intFound + 1);
        strClientID = strLine.substr(0, intFound);
        strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
        strClientName = strLine.substr(intFound2 + 1);
        intFound=strAEC.find(strClientID);
        if(intFound != std::string::npos) {
            strReturn = strLine;
        }
    }
    fpAEMap.close();
    return strReturn;
}

std::string fSendResponse(std::string strJson, std::string strPrimalID){
    std::string strCMD, strReturn, strQuery, strReadLine, strLogMessage, strRecNum, strClientID;
    std::size_t intPos;

    MYSQL *mconnect2;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "1";
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="MySQL connection failed in fSendResponse.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
        return "1";
    }
    MYSQL_ROW row;
    MYSQL_RES *result = mysql_store_result(mconnect2);

    intPos = strPrimalID.find("_");
    if(intPos != std::string::npos) {
        strRecNum=strPrimalID.substr(0,intPos);
    } else {
        strRecNum = "1";
    }
    strJson.erase(std::remove(strJson.begin(), strJson.end(), '\\'), strJson.end());
    intPos=strJson.find("dcm_raw");
    if(intPos != std::string::npos) {
        strJson.erase(0, intPos + 10);
    }
    intPos=strJson.find("[]}\"}");
    if(intPos != std::string::npos) {
        strJson.replace(intPos, 5, "[]}");
    }
    strQuery = "select sCaseID from study where puid = '" + strPrimalID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    }
    result = mysql_store_result(mconnect2);
    strClientID = "NULL";
    if(result) {
        row = mysql_fetch_row(result);
        strClientID=row[0];
    }
    mysql_free_result(result);

    strCMD = "curl -s --location --request POST 'https://sheridan.candescenthealth.com/radconnect/priorsListUpdate.db.php' ";
    strCMD += "--form 'challenge=94dfcb464afd2725297f2bbcc384f57a' ";
    strCMD += "--form 'cases_id=" + strClientID + "' ";
    strCMD += "--form 'priors=" + strJson + "'";
    //redi::ipstream proc(strCMD, redi::pstreams::pstdout);
    //std::getline(proc, strReturn);
    //while (std::getline(proc, strReadLine))
    //    strReturn.append(strReadLine + "\n");
    strReturn = exec(strCMD.c_str());
    strLogMessage = strCMD;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);

    //strQuery = "update study set sCaseID = '" + strReturn + "' where PUID='" + strPrimalID + "';";
    //mysql_query(mconnect2, strQuery.c_str());
    strLogMessage = strPrimalID + " QR Got response " + strReturn + " for C-FIND ";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
    //std::cout << strLogMessage << std::endl;
    mysql_close(mconnect2);
    return strReturn;
}

std::size_t fAddCfindtoDB(std::string strFullPath, std::size_t intMsgType, std::string strQRAET, std::string strQRAEC, std::string strQRport, std::string strQRIP, std::size_t intPrefetchPendingID) {
    (void) intMsgType;
    std::string strLine, strStudyDate, strStudyTime, strAccn, strMod, strStudyDesc, strPName, strPID, strPDOB, strSIUID;
    std::string strPrimalID, strResultSet, strQuery, strRecNum, strPSex, strBodyPart, strLogMessage, strDBREturn, strNumImg;
    std::string strFullPathCfind;
    std::size_t intInResult, intNumImg, intPos, intPos2, intFound;
    std::map<std::string, std::string>::iterator iprimConf;
    (void) intNumImg;
    (void) strFullPath;

    MYSQL *mconnect2;
    //MYSQL_ROW row;

    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return 1;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
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

    intInResult = 0;
    ifstream fpCfind;
    strFullPathCfind = strFullPath + "/cfind.xml";
    fpCfind.open(strFullPathCfind.c_str());
    intInResult=0;
    while (std::getline(fpCfind, strLine)) {
        intPos = strLine.find("<data-set");
        if(intPos != std::string::npos) {
            intInResult++;
        }
        intPos = strLine.find("</data-set>");
        if(intPos != std::string::npos) {
            intInResult++;
        }
        if(intInResult == 1) {
            intPos = strLine.find("0008,0020");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strStudyDate = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0008,0030");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strStudyTime = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0008,0050");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strAccn = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0008,0061");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strMod = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0008,1030");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strStudyDesc = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0010,0010");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strPName = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0010,0020");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strPID = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0010,0030");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strPDOB = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0010,0040");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strPSex = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0018,0015");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strBodyPart = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0020,000d");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strSIUID = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            intPos = strLine.find("0020,1208");
            if(intPos != std::string::npos) {
                intPos = strLine.find(">");
                intPos2 = strLine.find_last_of("<");
                strNumImg = strLine.substr(intPos + 1,intPos2 - (intPos + 1));
            }
            //intNumImg=stoi(fGetTagValue("0020,1208", strResultSet, 0));
        } else if (intInResult == 2) {
            intNumImg=0;
            strResultSet = "";
            strQuery = "insert into QR (puid, prefetch_pending_id, SIUID, qraet, qraec, qrport, qrip, pname, pid, psex, pbday, study_date, study_time, ";
            strQuery += "Accession, study_description, body_part_examined, modality, NumImages, qrstatus) values (\"";
            strQuery += strPrimalID + "\", \"" + to_string(intPrefetchPendingID) + "\", \"" + strSIUID + "\", \"" + strQRAET + "\", \"" + strQRAEC + "\", \"" + strQRport + "\", \"" + strQRIP + "\", \"";
            strQuery += strPName + "\", \"" + strPID + "\", \"" + strPSex + "\", \"" + strPDOB + "\", \"" + strStudyDate + "\", \"" + strStudyTime + "\", \"";
            strQuery += strAccn + "\", \"" + strStudyDesc + "\", \"" + strBodyPart + "\", \"" + strMod + "\", \"" + to_string(intNumImg) + "\", \"New\");";
            intInResult = 0;
            strLogMessage = "strQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
            //std::cout << strLogMessage << std::endl;
            mysql_query(mconnect2, strQuery.c_str());
            if(*mysql_error(mconnect2)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect2);
                strLogMessage+="\n strQuery = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
            }
        }
    }
    strLogMessage=strPrimalID + " QR  Finished adding found priors to DB.  Returning...";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    fpCfind.close();
    mysql_close(mconnect2);
    return 0;
}

std::size_t fCFind(std::string strFullPath, std::size_t intMsgType) {
    std::string strPrimalID, strRecNum, strLogMessage, strTemp2, strFilename, strDCMdump, strPID, strPname, strCfind, strCmd, strReadLine;
    std::string strTagVal, strQRAET, strQRAEC, strQRport, strQRIP, strQuery, strDBREturn, strFindString, strNumResults, strSIUID;
    std::string strTemp3, strCMD, strReturn, strMRN;
    std::size_t intFound, intPos, intDone, intDone2, intLC, intLC2, intReturn, intPrefetchPendingID, intFindResults, intError = 0;
    std::size_t intLC3;
    std::map<std::string, std::string>::iterator iprimConf;


    MYSQL *mconnect2;
    MYSQL_ROW row;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return 1;
    }
    //mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed in fCfind.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return 1;
    }
    MYSQL_RES *result = mysql_store_result(mconnect2);

    intFound = strFullPath.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strFullPath.substr(intFound + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);

    strLogMessage = strPrimalID + "QR  starting fCfind for message type " + to_string(intMsgType) + ".";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    strQuery = "select DicomCasesID from study where puid = '" + strPrimalID + "' limit 1;";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    }
    result = mysql_store_result(mconnect2);
    if(result == NULL) {
        intPrefetchPendingID=0;
    } else {
        row = mysql_fetch_row(result);
        strDBREturn=row[0];
        intPrefetchPendingID=stoi(strDBREturn);
    }
    mysql_free_result(result);

    intDone=0;
    intLC=0;
    while (intDone == 0) {
        iprimConf = conf1.primConf.find(strRecNum + "_PRIQRHIP" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIQRTAG" + to_string(intLC) + ":0");
            if(iprimConf == conf1.primConf.end()) {
                intError = 1;
                strLogMessage = "ERROR:  Could not find " + strRecNum + "_PRIQRTAG" + to_string(intLC) + ":0 for PRIQRHIP" + to_string(intLC);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                intDone = 1;
            } else {
                strQRIP = conf1.primConf[strRecNum + "_PRIQRHIP" + to_string(intLC)];
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIQRPORT" + to_string(intLC));
            if(iprimConf == conf1.primConf.end()) {
                intError = 1;
                strLogMessage = "ERROR:  Could not find PRIQRPORT for PRIQRHIP" + to_string(intLC);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                intDone = 1;
            } else {
                strQRport = conf1.primConf[strRecNum + "_PRIQRPORT" + to_string(intLC)];
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIQRAET" + to_string(intLC));
            if(iprimConf == conf1.primConf.end()) {
                intError = 1;
                strLogMessage = "ERROR:  Could not find PRIQRAET for PRIQRHIP" + to_string(intLC);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                intDone = 1;
            } else {
                strQRAET = conf1.primConf[strRecNum + "_PRIQRAET" + to_string(intLC)];
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIQRAEC" + to_string(intLC));
            if(iprimConf == conf1.primConf.end()) {
                intError = 1;
                strLogMessage = "ERROR:  Could not find PRIQRAEC for PRIQRHIP" + to_string(intLC);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                intDone = 1;
            } else {
                strQRAEC = conf1.primConf[strRecNum + "_PRIQRAEC" + to_string(intLC)];
            }

            if(intError == 0) {
                intLC3=0;
                for (const auto & entry : fs::directory_iterator(strFullPath)) {
                    strTemp2=entry.path().string();
                    intPos = strTemp2.find_last_of("/");
                    strFilename=strTemp2.substr(intPos+1);
                    //strLogMessage = "Filename: " + strFilename;
                    //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                    //std::cout << strLogMessage << std::endl;
                    intPos=strFilename.find_last_of(".");
                    if(intPos != std::string::npos) {
                        strTemp2=strFilename.substr(intPos);
                        if(strTemp2 == ".dcm" && intLC3 == 0) {
                            strLogMessage = "C-Find started for " + entry.path().string();
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                            strDCMdump=fDcmDump(entry.path().string());
                            strSIUID=fGetTagValue("0020,000d", strDCMdump, 0);
                            //strLogMessage="strDCMdump = " + strDCMdump;
                            //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                            //std::cout << strLogMessage << std::endl;
                            strCmd = "findscu -S -k 0008,0052=\"STUDY\"";
                            intLC2 = 0;
                            intDone2 = 0;
                            while(intDone2 == 0) {
                                iprimConf = conf1.primConf.find(strRecNum + "_PRIQRTAG" + to_string(intLC) + ":" + to_string(intLC2));
                                if(iprimConf != conf1.primConf.end()) {
                                    strTagVal=fGetTagValue(conf1.primConf[strRecNum + "_PRIQRTAG" + to_string(intLC) + ":" + to_string(intLC2)], strDCMdump, 1);
                                    strTemp3=conf1.primConf[strRecNum + "_PRIQRTAG" + to_string(intLC) + ":" + to_string(intLC2)];
                                    if(strTemp3.compare("0010,0020")==0 && intMsgType==2) {
                                        if(fs::exists("/usr/local/scripts/mrn_swap2.bash")) {
                                            strCMD = "/usr/local/scripts/mrn_swap2.bash \"" + strPrimalID + "\"" + " \"" + strTagVal + "\"";
                                            strReturn = exec(strCMD.c_str());
                                            if(strReturn.compare("") != 0 && strReturn.compare(" ") != 0) {
                                                strLogMessage = strPrimalID + " QR  mrn_swap2.bash returned " + strReturn;
                                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                                                strTagVal = strReturn;
                                                strTagVal.erase(std::remove(strTagVal.begin(), strTagVal.end(), '\n'), strTagVal.end() );
                                            }
                                        }
                                        strMRN=strTagVal;
                                    }else if(strTemp3.compare("0010,0020")==0 && intMsgType==1) {
                                        strMRN=strTagVal;
                                    }
                                    if(strTagVal != "") {
                                        strCmd += " -k " + conf1.primConf[strRecNum + "_PRIQRTAG" + to_string(intLC) + ":" + to_string(intLC2)];
                                        strCmd += "=\"" + strTagVal + "\"";
                                    }
                                } else {
                                    intDone2 = 1;
                                }
                                intLC2 ++;
                                if(intLC2 > 20) {
                                    intDone2 = 1;
                                }
                            }
                            strCmd += " -aet " + conf1.primConf[strRecNum + "_PRIQRAET" + to_string(intLC)];
                            strCmd += " -aec " + conf1.primConf[strRecNum + "_PRIQRAEC" + to_string(intLC)];
                            strCmd += " " + conf1.primConf[strRecNum + "_PRIQRHIP" + to_string(intLC)];
                            strCmd += " " + conf1.primConf[strRecNum + "_PRIQRPORT" + to_string(intLC)];
                            strCmd += " -k 0008,0020";
                            strCmd += " -k 0008,0030";
                            strCmd += " -k 0008,0050";
                            strCmd += " -k 0008,0061";
                            strCmd += " -k 0008,1030";
                            strCmd += " -k 0010,0010";
                            strCmd += " -k 0010,0040";
                            strCmd += " -k 0018,0015";
                            strCmd += " -k 0020,000D";
                            strCmd += " -Xs " + strFullPath + "/cfind.xml";
                            strCmd += " 2>&1";
                            strLogMessage = "strCmd = " + strCmd;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                            //std::cout << strLogMessage << std::endl;
                            //strCfind = exec(strCmd.c_str());
                            system(strCmd.c_str());
                            strCmd = "grep -c \"<data-set\" " + strFullPath + "/cfind.xml";
                            strNumResults = exec(strCmd.c_str());
                            strNumResults.erase(remove_if(strNumResults.begin(), strNumResults.end(), [](char c) { return !std::isdigit(c); }), strNumResults.end());
                            if(strNumResults.length() < 1) {
                                intFindResults = 0;
                            } else {
                                intFindResults = stoi(strNumResults);
                            }
                            strLogMessage = strPrimalID + " QR  C-Find returned " + to_string(intFindResults);
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                            //std::cout << "strCfind right after exec: " << strCfind << "."  << std::endl;
                            //std::cout << "intFindResults = " << to_string(intFindResults) << "." << std::endl;
                            if(intFindResults == 0) {
                                strLogMessage = "QR WARN:  There were no C-FIND results for " + strPrimalID + " using MRN: " + strMRN + ".";
                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                                //std::cout << strLogMessage << std::endl;
                                intPrefetchPendingID = 0;
                            } else {
                                intReturn = fAddCfindtoDB(strFullPath, intMsgType, strQRAET, strQRAEC, strQRport, strQRIP, intPrefetchPendingID);
                                if (intReturn != 0) {
                                    strLogMessage = " QR ERROR:  fAddCfindtoDB returned a non-zero value";
                                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                                }
                            }
                            intLC3++;
                        }
                    }
                }
            }
        } else {
            intDone = 1;
        }
        intLC++;
    }
    strLogMessage = strPrimalID + " QR  fCfind finished.  Returning ID: " + to_string(intPrefetchPendingID);
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    mysql_close(mconnect2);
    return intPrefetchPendingID;
}

std::string fCreateJson(std::string strPrimalID, std::size_t intPrefetchPendingID, std::size_t intLC) {
    std::size_t intImgNum, intNumRows, intPrefetchResultsId, intLC2;
    std::string strICaseID, strJson, strTemp2, strQuery, strRow, strFilename, strRecNum, strLogMessage, strDBREturn, strSIUID, strQRaet;
    std::string strQRaec, strQRport, strQRip, strPname, strPID, strPsex, strDOB, strStudyDate, strStudyTime, strStudyDateTime;
    std::string strACCN, strStudyDesc, strBodyPartExamined, strMOD, strMRN;
    std::map<std::string, std::string>::iterator iprimConf;
    (void) strPrimalID;

    MYSQL *mconnect2;

    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fCreateJson.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return "ERR";
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed in fCreateJson";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return "ERR";
    }
    MYSQL_ROW row2;
    MYSQL_RES *result2 = mysql_store_result(mconnect2);

    strLogMessage = strPrimalID + " QR creating C-FIND JSON for " + conf1.primConf[strRecNum + "_PRIQRHIP" + to_string(intLC)];
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    //std::cout << strLogMessage << std::endl;

    strQuery="select SIUID from study where puid = '" + strPrimalID + "' limit 1;";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage=mysql_error(mconnect2);
        strLogMessage+="strQuery = " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        exit(-1);
    }
    result2 = mysql_store_result(mconnect2);
    if(result2) {
        intNumRows = mysql_num_rows(result2);
        if(intNumRows > 0) {
            row2 = mysql_fetch_row(result2);
            strSIUID = row2[0];
        }
        mysql_free_result(result2);
    }
    strQuery="select pid from patient where puid = '" + strPrimalID + "' limit 1;";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage=mysql_error(mconnect2);
        strLogMessage+="strQuery = " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        exit(-1);
    }
    strMRN="0";
    result2 = mysql_store_result(mconnect2);
    if(result2) {
        intNumRows = mysql_num_rows(result2);
        if(intNumRows > 0) {
            row2 = mysql_fetch_row(result2);
            strMRN = row2[0];
            strLogMessage+=" QR " + strPrimalID + " Using MRN:" + strMRN + " as master MRN.";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        }
        mysql_free_result(result2);
    }


    //Find the location of the file from the database
    strQuery="select * from QR where prefetch_pending_id = '" + to_string(intPrefetchPendingID) + "' and SIUID <> '" + strSIUID + "';";
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage=mysql_error(mconnect2);
        strLogMessage+="\nstrQuery = " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        exit(-1);
    }
    result2 = mysql_store_result(mconnect2);
    if(result2) {
        intNumRows = mysql_num_rows(result2);
        strLogMessage=to_string(intNumRows) + " results found.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    }
    strJson = "[";
    intLC2=0;
    while ((row2 = mysql_fetch_row(result2))) {
        strRow = row2[0];
        intPrefetchResultsId=stoi(strRow);
        strSIUID = row2[2];
        strQRaet = row2[4];
        strQRaec = row2[5];
        strQRport = row2[6];
        strQRip = row2[7];
        strPname = row2[9];
        if(strMRN.compare("0")!=0) {
            strPID = strMRN;
        } else {
            strPID = row2[10];
        }
        strPsex = row2[11];
        strDOB = row2[12];
        strStudyDate = row2[13];
        strStudyTime = row2[14];
        strStudyDateTime = strStudyDate + " " + strStudyTime;
        strACCN = row2[15];
        strStudyDesc = row2[16];
        strBodyPartExamined = row2[17];
        strMOD = row2[18];
        strRow = row2[21];
        intImgNum = stoi(strRow);
        if(intLC2 > 0) {
            strJson += ",";
        }
        strJson += "{";
        strJson += "\"prefetch_results_id\":\"" + to_string(intPrefetchResultsId) + "\",";
        strJson += "\"prefetch_pending_id\":\"" + to_string(intPrefetchPendingID) + "\",";
        strJson += "\"dicom_cases_id\":\"0\",";
        strJson += "\"application_entity_id\":\"0\",";
        strJson += "\"move_destination_ae\":\"null\",";
        strJson += "\"status\":\"Hold\",";
        strJson += "\"failure_reason\":\"null\",";
        strJson += "\"send_host\":\"" + strHostname + "\",";
        strJson += "\"send_network\":\"" + strHostname + "\",";
        strJson += "\"priority\":\"null\",";
        strJson += "\"date_created\":\"" + GetDate() + "\",";
        strJson += "\"date_modified\":\"" + GetDate() + "\",";
        strJson += "\"patient_name\":\"" + strPname + "\",";
        strJson += "\"patient_id\":\"" + strPID + "\",";
        strJson += "\"patient_sex\":\"" + strPsex + "\",";
        strJson += "\"patient_birth_date\":\"" + strDOB + "\",";
        strJson += "\"study_date\":\"" + strStudyDate + "\",";
        strJson += "\"study_time\":\"" + strStudyTime + "\",";
        strJson += "\"accession_number\":\"" + strACCN + "\",";
        strJson += "\"study_instance_uid\":\"" + strSIUID + "\",";
        strJson += "\"study_description\":\"" + strStudyDesc + "\",";
        strJson += "\"body_part_examined\":\"" + strBodyPartExamined + "\",";
        strJson += "\"modalities_in_study\":\"" + strMOD + "\",";
        strJson += "\"num_study_imgs\":\"" + to_string(intImgNum) + "\",";
        strJson += "\"num_images\":\"0\",";
        strJson += "\"instance_availability\":\"null\",";
        strJson += "\"file_set_id\":\"null\",";
        strJson += "\"registry_id\":\"0\",";
        strJson += "\"name\":\"PF: 1101\",";
        strJson += "\"remote_title\":\"RASP_LB\"";
        strJson += "}";
        intLC2++;
    }
    strJson += "]";
    mysql_free_result(result2);
    //Need to send.
    return strJson;
}

std::size_t fSelectPriors(std::string strPrimalID, std::size_t intPrefetchPendingID) {
    std::string strQuery, strMOD, strDBREturn, strRecNum, strQRaec, strQRaet, strQRip, strQRport, strSIUID, strCmd, strLogMessage;
    std::string strPrimalSIUID, strStudyDesc, strBodyPart, strSDesc, strSBodyPart;
    std::size_t intPos, intDBEntries, intDone, intReturn;
    std::map<std::string, std::string>::iterator iprimConf;
    MYSQL *mconnect2;
    MYSQL_ROW row;
    MYSQL_ROW row2;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fSelectPriors";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return -1;
    }
    //mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed in fSelectPriors";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        return -1;
    }
    MYSQL_RES *result = mysql_store_result(mconnect2);
    MYSQL_RES *result2 = mysql_store_result(mconnect2);

    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);

    strQuery = "select StudyDesc from study where puid = '" + strPrimalID + "' limit 1;";
    //std::cout << "strQuery = " << strQuery << "." << std::endl;
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage=mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        row = mysql_fetch_row(result);
        strStudyDesc=row[0];
    } else {
        strStudyDesc="UNK";
    }
    mysql_free_result(result);

    strQuery="select SIUID from study where puid = '" + strPrimalID + "' limit 1";
    mysql_query(mconnect2, strQuery.c_str());
    result = mysql_store_result(mconnect2);
    if(result) {
        strPrimalSIUID = row[0];
    } else {
        strPrimalSIUID = "0";
    }
    strLogMessage = strPrimalID + " QR " + " SIUID query = " + strQuery;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    //std::cout << strLogMessage << std::endl;
    mysql_free_result(result);

    strQuery="select body_part from procedure_matching where study_description = '" + strStudyDesc + "' limit 1";
    strLogMessage = strPrimalID + " QR " + " Body part query = " + strQuery;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    //std::cout << strLogMessage << std::endl;
    mysql_query(mconnect2, strQuery.c_str());
    if(*mysql_error(mconnect2)) {
        strLogMessage = strPrimalID + " QR " + " QR query = " + strQuery;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        //std::cout << strLogMessage << std::endl;
        strLogMessage = strPrimalID + " QR " + " SQL Error: " + mysql_error(mconnect2);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        //std::cout << strLogMessage << std::endl;
    }
    result = mysql_store_result(mconnect2);
    if(result) {
        row = mysql_fetch_row(result);
        strBodyPart = row[0];
    } else {
        strBodyPart = "UNK";
    }
    mysql_free_result(result);
    strLogMessage = strPrimalID + " QR " + " Body part for current study is " + strBodyPart + ".";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    //std::cout << strLogMessage << std::endl;

    //Get results
    intDone=0;
    strQuery="select * from QR where prefetch_pending_id = '" + to_string(intPrefetchPendingID);
    strQuery += "' and SIUID != '" + strPrimalSIUID + "' order by study_date DESC;";
    strLogMessage = strPrimalID + " QR " + " QR query = " + strQuery;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
    //std::cout << strLogMessage << std::endl;
    mysql_query(mconnect2, strQuery.c_str());
    result = mysql_store_result(mconnect2);
    if(result) {
        intDBEntries=mysql_num_rows(result);
        if(intDBEntries > 0) {
            intDone = 1;
        }
    } else {
        intDBEntries = 0;
    }
    /*
    if(intDone == 0) {
        mysql_free_result(result);
        strQuery="select * from QR where prefetch_pending_id = '" + to_string(intPrefetchPendingID);
        strQuery += "' and study_description like '%" + strMOD + " %' and SIUID != '" + strPrimalSIUID + "';";
        mysql_query(mconnect2, strQuery.c_str());
        result = mysql_store_result(mconnect2);
        if(result) {
            intDBEntries=mysql_num_rows(result);
            if(intDBEntries > 0) {
                intDone = 1;
            }
        } else {
            intDBEntries = 0;
        }
    }
    */
    if(intDone == 0) {
        mysql_free_result(result);
    } else {
        intReturn = 0;
        while ((row = mysql_fetch_row(result))) {
            strSIUID = row[2];
            strQRaet = row[4];
            strQRaec = row[5];
            strQRport = row[6];
            strQRip = row[7];
            strSDesc = row[16];
            strMOD = row[18];
            iprimConf = conf1.primConf.find(strRecNum + "_PRIQRMOVE0");
            if(iprimConf != conf1.primConf.end()) {
                strQRaec = conf1.primConf[strRecNum + "_PRIQRMOVE0"];
            }
            strQuery = "select body_part from procedure_matching where study_description = '" + strSDesc + "' limit 1";
            mysql_query(mconnect2, strQuery.c_str());
            if(*mysql_error(mconnect2)) {
                strLogMessage = strPrimalID + " QR " + " QR query = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                //std::cout << strLogMessage << std::endl;
                strLogMessage = strPrimalID + " QR " + " SQL Error: " + mysql_error(mconnect2);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                //std::cout << strLogMessage << std::endl;
            }
            result2 = mysql_store_result(mconnect2);
            if(result2) {
                intDBEntries=mysql_num_rows(result2);
                if(intDBEntries > 0 && intReturn < 5) {
                    row2 = mysql_fetch_row(result2);
                    strSBodyPart = row2[0];
                    if(strBodyPart == strSBodyPart){
                        strLogMessage = strPrimalID + " QR " + " Body part " + strSBodyPart + " matched for SIUID " + strSIUID;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                        //std::cout << strLogMessage << std::endl;
                        strCmd = "movescu -d -S -k 0008,0052=STUDY -aec " + strQRaec + " -aet " + strQRaet + " " + strQRip + " " + strQRport;
                        strCmd += " -k 0020,000D=" + strSIUID + " &";
                        strLogMessage = strPrimalID + " QR " + " Directory " + strCmd;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                        //std::cout << strLogMessage << std::endl;
                        system(strCmd.c_str());
                        intReturn++;
                    } else {
                        strLogMessage = strPrimalID + " QR " + " Body part " + strSBodyPart + " did not match for SIUID " + strSIUID;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                        //std::cout << strLogMessage << std::endl;
                    }
                }
            } else {
                intDBEntries = 0;
            }
            mysql_free_result(result2);
        }
        mysql_free_result(result);
    }
    mysql_close(mconnect2);
    return intReturn;
}

std::size_t fAckRequest(std::string strClientIDs, std::string strPrefetchID) {
    std::string strCMD, strReturn, strLogMessage;

    strCMD = "curl -s --location --request POST 'https://sheridan.candescenthealth.com/radconnect/getPriorsDownload.db.php' ";
    strCMD += "--form 'challenge=94dfcb464afd2725297f2bbcc384f57a' ";
    strCMD += "--form 'clients_id=" + strClientIDs + "' ";
    strCMD += "--form 'calling_ae=RCONN' ";
    strCMD += "--form 'called_ae=APEX_" + strClientIDs + "' ";
    strCMD += "--form 'acknowledements=[\"" + strPrefetchID + "\"]' ";

    strLogMessage="Sending acknowlegement\n";
    strLogMessage+=strCMD + "\n";
    strReturn = exec(strCMD.c_str());
    strLogMessage+="Polling strReturn\n";
    strLogMessage+=strReturn;
    fWriteLog(strLogMessage, conf1.primConf["1_PRILOGDIR"] + "/qr_poll.log");
    return 0;
}

void fPollRIS() {
    std::string strClientID , strLogMessage, strCMD, strReturn, strLine, strPrefetchID, strQuery, strSIUID, strQRaet, strQRaec;
    std::string strQRport, strQRip, strSDesc, strMOD, strPrimalID, strRecNum, strReturn2, strCasePriorsID, strQRstatus;
    std::size_t intFound, intPos, intPos2, intPos3, intPos4, intDBEntries;
    std::vector<std::string> vecClientIDs;
    std::vector<std::string>::iterator itClientIDs;
    std::map<std::string, std::string>::iterator iprimConf;
    MYSQL *mconnect2;
    MYSQL_ROW row;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fSelectPriors";
        fWriteLog(strLogMessage, conf1.primConf["1_PRILOGDIR"] + "/" + conf1.primConf["1_PRILFQR"]);
        return;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed in fSelectPriors";
        fWriteLog(strLogMessage, conf1.primConf["1_PRILOGDIR"] + "/" + conf1.primConf["1_PRILFQR"]);
        return;
    }
    MYSQL_RES *result = mysql_store_result(mconnect2);

    strRecNum = "1";
    //Get a list of client IDs we need to poll for
    if(fs::exists("/etc/primal/prim_ae_map.conf")) {
        std::ifstream fpAEMap("/etc/primal/prim_ae_map.conf");
        while (std::getline(fpAEMap, strLine)) {
            intFound=strLine.find(",");
            strClientID = strLine.substr(0, intFound);
            if(std::find(vecClientIDs.begin(), vecClientIDs.end(), strClientID) == vecClientIDs.end()) {
                vecClientIDs.push_back(strClientID);
                strLogMessage = " POLLING QR Adding " + strClientID + " to list of client ID to poll";
                fWriteLog(strLogMessage, conf1.primConf["1_PRILOGDIR"] + "/" + conf1.primConf["1_PRILFQR"]);
                //std::cout << strLogMessage << std::endl;
            }
        }
        fpAEMap.close();
    }
    while(1) {
        for (itClientIDs = vecClientIDs.begin(); itClientIDs != vecClientIDs.end(); ++itClientIDs) {
            strCMD = "curl -s --location --request POST 'https://sheridan.candescenthealth.com/radconnect/getPriorsDownload.db.php' ";
            strCMD += "--form 'challenge=94dfcb464afd2725297f2bbcc384f57a' ";
            strCMD += "--form 'clients_id=" + *itClientIDs + "' ";
            strCMD += "--form 'calling_ae=RCONN' ";
            strCMD += "--form 'called_ae=APEX_" + *itClientIDs + "' ";
            strReturn = exec(strCMD.c_str());
            strLogMessage="Polling strCMD\n";
            strLogMessage+=strCMD + "\n";
            strLogMessage+="Polling strReturn\n";
            strLogMessage+=strReturn + "\n";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        }
        while (strReturn.length() > 14) {
            intPos = strReturn.find("}");
            if(intPos != std::string::npos) {
                strLine = strReturn.substr(0, intPos);
            } else {
                strLine = strReturn;
            }
            intPos2 = strLine.find("case_priors_id");
            if(intPos2 != std::string::npos) {
                intPos3 = strLine.find_first_of(",");
                if(intPos3 == std::string::npos) {
                    intPos3 = strLine.find_first_of("]");
                }
                strCasePriorsID = strLine.substr(intPos2 + 17, intPos3 - intPos2 - 18);
            }
            intPos2 = strLine.find("prefetch_results_id");
            if(intPos2 != std::string::npos) {
                intPos3 = strLine.find_last_of("\"");
                //std::cout << "intPos2 = " << intPos2 << "intPos3 = " << intPos3 << std::endl;
                strPrefetchID = strLine.substr(intPos2 + 22, intPos3 - intPos2 - 22);
                strLogMessage="case_priors_id: " + strCasePriorsID + "   prefetch_results_id: " + strPrefetchID;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                strQuery = "select QR.puid, QR.SIUID, QR.qraet, QR.qraec, QR.qrport, QR.qrip, QR.study_description, QR.modality, QR.qrstatus, study.sClientID from QR left join study on QR.puid = study.puid where prefetch_results_id = '" + strPrefetchID + "';";
                mysql_query(mconnect2, strQuery.c_str());
                //strLogMessage = strQuery;
                //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                //std::cout << strQuery << std::endl;
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    strLogMessage+="\nstrQuery = " + strQuery + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                }
                result = mysql_store_result(mconnect2);
                if(result) {
                    intDBEntries=mysql_num_rows(result);
                    if(intDBEntries > 0) {
                        row = mysql_fetch_row(result);
                        strPrimalID = row[0];
                        strSIUID = row[1];
                        strQRaet = row[2];
                        strQRaec = row[3];
                        strQRport = row[4];
                        strQRip = row[5];
                        strSDesc = row[6];
                        strMOD = row[7];
                        strQRstatus = row[8];
                        strClientID = row[9];
                        intPos4 = strPrimalID.find("_");
                        if(intPos4 != std::string::npos) {
                            strRecNum = strPrimalID.substr(0,intPos4);
                        } else {
                            strRecNum = "1";
                            strLogMessage = strPrimalID + " QR " + " WARN  Unable to determine receiver number.  Setting to 1. ";
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                            //std::cout << strLogMessage << std::endl;
                        }
                        //std::cout << "strRecNum = " << strRecNum << std::endl;
                        iprimConf = conf1.primConf.find(strRecNum + "_PRIQRMOVE" + to_string(0));
                        if(iprimConf != conf1.primConf.end()) {
                           strQRaec = conf1.primConf[strRecNum + "_PRIQRMOVE" + to_string(0)];
                        }
                        if(strQRstatus == "Hold" || strQRstatus == "New") {
                            strCMD = "movescu -d -S -k 0008,0052=STUDY -aec " + strQRaec + " -aet " + strQRaet + " " + strQRip + " " + strQRport;
                            strCMD += " -k 0020,000D=" + strSIUID + " &";
                            strLogMessage = strPrimalID + " QR " + " Directory " + strCMD;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                            //std::cout << strLogMessage << std::endl;
                            strQuery="update QR set qrstatus='Requested' where prefetch_results_id = '" + strPrefetchID + "';";
                            mysql_query(mconnect2, strQuery.c_str());
                            if(*mysql_error(mconnect2)) {
                                strLogMessage=mysql_error(mconnect2);
                                strLogMessage+="\n" + strQuery;
                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                            }
                            system(strCMD.c_str());
                            fAckRequest(strClientID, strCasePriorsID);
                        } else {
                            strLogMessage = strPrimalID + " QR WARN:  Skipping Prefetch Results ID " + strPrefetchID + " because it is in status " + strQRstatus;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                        }
                        mysql_free_result(result);
                    }
                } else {
                    strLogMessage = "ERROR:  Couldn't get Primal ID";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                }
            }
            if(intPos != std::string::npos) {
                strReturn.erase(0, intPos + 1);
            } else {
                strReturn.erase(0,strReturn.length());
            }
        }
        std::this_thread::sleep_for (std::chrono::seconds(10));
    }
} 

void fQRmain(std::string strMessage) {
    std::string strFullPath, strPrimalID, strQuery, strRow, strTemp, strJson, strSerIUID, strSerTemp, strTemp2, strFilename, strOldVal;
    std::string strDate, strRecNum, strPrimalIDInt, strTemp3, strLogMessage, strSIUID, strAEC, strPackFileName, strCaseID, strNewPath;
    std::string strDBREturn, strCfindFile, strTempPath, strCfind, strList, strCmd, strPrefetchPendingID;
    std::size_t intPos, intFound, intMsgType, intPrefetchPendingID, intUseRis, intLC, intNumResults, intNumRows, intDone;
    std::map<std::string, std::string> mapSeriesJson;
    std::map<std::string, std::string>::iterator iprimConf;
    //struct stat sb;

    MYSQL *mconnect;
    mconnect=mysql_init(NULL);
    std::cout << "MySQL thread safe " << mysql_thread_safe() << std::endl;
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect) {
        cout << "MySQL Initilization failed";
        return;
    }
    ReadDBConfFile();
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        cout<<"connection failed\n";
        return;
    }
    MYSQL_ROW row;
    MYSQL_RES *result = mysql_store_result(mconnect);

    strFullPath=strMessage;
    //Parse message
    strLogMessage = "Got the following message: " + strMessage;
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
    //std::cout << strLogMessage << std::endl;
    intPos=strFullPath.find_last_of(" ");
    if(intPos != std::string::npos) {
        intMsgType=stoi(strFullPath.substr(intPos));
    } else {
        intMsgType = 1;
    }
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

    //Start doing the work
    if(intMsgType == 1) {
        intDone=0;
        intLC=0;
        while(intDone != 1) {
            iprimConf = conf1.primConf.find(strRecNum + "_PRIQRWAIT" + to_string(intLC));
            if(iprimConf == conf1.primConf.end() || conf1.primConf[strRecNum + "_PRIQRWAIT" + to_string(intLC)] == "0") {
                strCmd = "touch " + strFullPath + "/cfind.json";
                system(strCmd.c_str());
            }
            if(!fs::exists(strFullPath)) {
                strLogMessage = strPrimalID + " QR " + " Directory " + strFullPath + " does not exist.  Skipping...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                std::cout << strLogMessage << std::endl;
            } else {
                //C-Find section
                intPrefetchPendingID = fCFind(strFullPath, intMsgType);
                if(fs::exists("/usr/local/scripts/mrn_swap2.bash")) {
                    fCFind(strFullPath, 2);
                }
                strLogMessage="intPrefetchPendingID = " + to_string(intPrefetchPendingID) + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                if(intPrefetchPendingID != 0) {
                    intUseRis = 0;
                    intLC=0;
                    iprimConf = conf1.primConf.find(strRecNum + "_PRIQRLIST" + to_string(intLC));
                    if(iprimConf != conf1.primConf.end()) {
                        if(conf1.primConf[strRecNum + "_PRIQRLIST" + to_string(intLC)] == "1") {
                            intUseRis = 1;
                        }
                    }
                    if(intUseRis != 1) {
                        intNumResults = fSelectPriors(strPrimalID, intPrefetchPendingID);
                        strLogMessage = strPrimalID + " QR Processed " + to_string(intNumResults) + " results.";
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        std::cout << strLogMessage << std::endl;
                    } else {
                        strLogMessage = strPrimalID + " QR Using RIS priors workflow for " + conf1.primConf[strRecNum + "_PRIQRHIP" + to_string(intLC)];
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        strQuery = "select sCaseID from study where puid = '" + strPrimalID + "';";
                        mtx.lock();
                        mysql_query(mconnect, strQuery.c_str());
                        if(*mysql_error(mconnect)) {
                            strLogMessage=mysql_error(mconnect);
                            strLogMessage+="strQuery = " + strQuery;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        } else {
                            result = mysql_store_result(mconnect);
                            if(result) {
                                intNumRows = mysql_num_rows(result);
                                if(intNumRows > 0) {
                                    row = mysql_fetch_row(result);
                                    if(row[0] == NULL) {
                                        strCaseID = "0";
                                    } else {
                                        strCaseID = row[0];
                                    }
                                    if(strCaseID.compare("0") != 0) {
                                        strJson = fCreateJson(strPrimalID, intPrefetchPendingID, intLC);
                                        strLogMessage=strJson;
                                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                                        strTemp = fSendResponse(strJson, strPrimalID);
                                    } else {
                                        strLogMessage = strPrimalID + " QR caseID of " + strCaseID + " for " + strPrimalID + ".  Skipping QR process...";
                                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                                        //std::cout << strLogMessage << std::endl;
                                    }
                                } else {
                                    strLogMessage = strPrimalID + " QR No results found for " + strPrimalID + ".  Skipping QR process...";
                                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                                    //std::cout << strLogMessage << std::endl;
                                }
                                mysql_free_result(result);
                            } else {
                                strLogMessage = strPrimalID + " QR No results found for " + strPrimalID + ".  Skipping QR process...";
                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                                //std::cout << strLogMessage << std::endl;
                            }
                        }
                        mtx.unlock();
                        strQuery = "update QR set qrstatus = 'Hold' where puid = '" + strPrimalID = "';";
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        mysql_query(mconnect, strQuery.c_str());
                        if(*mysql_error(mconnect)) {
                            strLogMessage=mysql_error(mconnect);
                            strLogMessage+="strQuery = " + strQuery;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        }
                        strLogMessage = strPrimalID + " QR  Setting qrstatus to Hold for results.";
                        strCmd = "touch " + strFullPath + "/cfind.json";
                        system(strCmd.c_str());
                    }
                } else {
                    strCmd = "touch " + strFullPath + "/cfind.json";
                    system(strCmd.c_str());
                }
            }
            intLC++;
            iprimConf = conf1.primConf.find(strRecNum + "_PRIQRHIP" + to_string(intLC));
            if(iprimConf == conf1.primConf.end()) {
                intDone=1;
            }
        }
    } else if(intMsgType == 2) {
        strLogMessage = strPrimalID + " QR  Should not be getting here.  Have a message type of " + to_string(intMsgType);
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
        if(!fs::exists(strFullPath)) {
            strLogMessage = strPrimalID + " QR " + " Directory " + strFullPath + " does not exist.  Skipping...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
            //std::cout << strLogMessage << std::endl;
        } else {
            //C-Find section
            intPrefetchPendingID = fCFind(strFullPath, intMsgType);
            strLogMessage="Additional C-Find executed for PrefetchPendingID = " + to_string(intPrefetchPendingID) + ".\n";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
        }         
    } else if(intMsgType == 3) {
        strQuery = "select prefetch_pending_id from QR where puid = '" + strPrimalID + "';";
        mtx.lock();
        mysql_query(mconnect, strQuery.c_str());
        if(*mysql_error(mconnect)) {
            strLogMessage=mysql_error(mconnect);
            strLogMessage+="strQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
            return;
        }
        result = mysql_store_result(mconnect);
        if(result) {
            intNumRows = mysql_num_rows(result);
            if(intNumRows > 0) {
                while((row = mysql_fetch_row(result))) {
                    strPrefetchPendingID = row[0];
                    strPrefetchPendingID.erase(remove_if(strPrefetchPendingID.begin(), strPrefetchPendingID.end(), [](char c) { return !std::isdigit(c); }), strPrefetchPendingID.end());
                    if(strPrefetchPendingID.length() < 1) {
                        strLogMessage="intPrefetchPendingID = " + to_string(intPrefetchPendingID) + " which is bogus.  Continuing...";
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        continue;
                    } else {
                        intPrefetchPendingID = stoi(strPrefetchPendingID);
                    }
                    strLogMessage="intPrefetchPendingID = " + to_string(intPrefetchPendingID) + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                    intUseRis = 0;
                    intLC=0;
                    iprimConf = conf1.primConf.find(strRecNum + "_PRIQRLIST" + to_string(intLC));
                    if(iprimConf != conf1.primConf.end()) {
                        if(conf1.primConf[strRecNum + "_PRIQRLIST" + to_string(intLC)] == "1") {
                            intUseRis = 1;
                        }
                    }
                    if(intUseRis != 1) {
                        intNumResults = fSelectPriors(strPrimalID, intPrefetchPendingID);
                        strLogMessage = strPrimalID + " QR Processed " + to_string(intNumResults) + " results.";
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        std::cout << strLogMessage << std::endl;
                    } else {
                        strLogMessage = strPrimalID + " QR Using RIS priors workflow for " + conf1.primConf[strRecNum + "_PRIQRHIP" + to_string(intLC)];
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        strQuery = "select sCaseID from study where puid = '" + strPrimalID + "';";
                        mtx.lock();
                        mysql_query(mconnect, strQuery.c_str());
                        if(*mysql_error(mconnect)) {
                            strLogMessage=mysql_error(mconnect);
                            strLogMessage+="strQuery = " + strQuery;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        } else {
                            result = mysql_store_result(mconnect);
                            if(result) {
                                intNumRows = mysql_num_rows(result);
                                if(intNumRows > 0) {
                                    row = mysql_fetch_row(result);
                                    if(row[0] == NULL) {
                                        strCaseID = "0";
                                    } else {
                                        strCaseID = row[0];
                                    }
                                    if(strCaseID.compare("0") != 0) {
                                        strJson = fCreateJson(strPrimalID, intPrefetchPendingID, intLC);
                                        strLogMessage=strJson;
                                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                                        strTemp = fSendResponse(strJson, strPrimalID);
                                    } else {
                                        strLogMessage = strPrimalID + " QR caseID of " + strCaseID + " for " + strPrimalID + ".  Skipping QR process...";
                                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                                        //std::cout << strLogMessage << std::endl;
                                    }
                                } else {
                                    strLogMessage = strPrimalID + " QR No results found for " + strPrimalID + ".  Skipping QR process...";
                                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                                    //std::cout << strLogMessage << std::endl;
                                }
                                mysql_free_result(result);
                            } else {
                                strLogMessage = strPrimalID + " QR No results found for " + strPrimalID + ".  Skipping QR process...";
                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                                //std::cout << strLogMessage << std::endl;
                            }
                        }
                        strQuery = "update QR set qrstatus = 'Hold' where puid = '" + strPrimalID = "';";
                        mysql_query(mconnect, strQuery.c_str());
                        if(*mysql_error(mconnect)) {
                            strLogMessage=mysql_error(mconnect);
                            strLogMessage+="strQuery = " + strQuery;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        }
                        mtx.unlock();
                        strCmd = "touch " + strFullPath + "/cfind.json";
                        system(strCmd.c_str());
                    }
                }
            }
        }
    }
    mysql_close(mconnect); 
}

void fCheckStartup() {
    std::size_t intNewPID, intPID, intNumRows;
    std::string strPID, strQuery, strLogMessage, strRecNum, strCmd;

    MYSQL *mconnect2;
    MYSQL_ROW row;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fSelectPriors";
        fWriteLog(strLogMessage, conf1.primConf["1_PRILOGDIR"] + "/" + conf1.primConf["1_PRILFQR"]);
        return;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed in fSelectPriors";
        fWriteLog(strLogMessage, conf1.primConf["1_PRILOGDIR"] + "/" + conf1.primConf["1_PRILFQR"]);
        return;
    }
    MYSQL_RES *result = mysql_store_result(mconnect2);

    strRecNum = "1";
    intNewPID = getpid();
    if(fs::exists("/var/run/prim_qr_server.pid")) {
        std::ifstream fpPID("/var/run/prim_qr_server.pid");
        std::getline(fpPID, strPID);
        strPID.erase(remove_if(strPID.begin(), strPID.end(), [](char c) { return !std::isdigit(c); }), strPID.end());
        if(strPID.length() < 1) {
            intPID = 0;
        } else {
            intPID = stoi(strPID);
        }
        fpPID.close();
        if(intPID != 0 && intPID != intNewPID) {
            std::ofstream fpPID2("/var/run/prim_qr_server.pid");
            fpPID2 << to_string(intNewPID);
            fpPID2.close();
        }
        strQuery = "select puid from QR where qrstatus = 'New' group by prefetch_pending_id";
        mtx.lock();
        mysql_query(mconnect2, strQuery.c_str());
        if(*mysql_error(mconnect2)) {
            strLogMessage=mysql_error(mconnect2);
            strLogMessage+="strQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
        } else {
            result = mysql_store_result(mconnect2);
            if(result) {
                intNumRows = mysql_num_rows(result);
                if(intNumRows > 0) {
                    while((row = mysql_fetch_row(result))) {
                        strCmd = "/home/dicom/processing/";
                        strCmd +=row[0];
                        strCmd +=" 3";
                        strLogMessage = " QR  Requeing message:  " + strCmd;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                        fWriteMessage(strCmd, "/prim_qr");
                    }
                }
                mysql_free_result(result);
            }
        }
        mtx.unlock();
    } else {
        std::ofstream fpPID2("/var/run/prim_qr_server.pid");
        fpPID2 << to_string(intNewPID);
        fpPID2.close();
    }
    mysql_close(mconnect2);
    return;
}

int main() {
    std::string strClientID , strLogMessage, strCMD, strReturn, strLine, strPrefetchID, strQuery, strSIUID, strQRaet, strQRaec;
    std::string strQRport, strQRip, strSDesc, strMOD, strPrimalID, strRecNum, strReturn2, strCasePriorsID, strQRstatus;
    std::size_t intFound, intPos, intPos2, intPos3, intPos4, intDBEntries;
    std::vector<std::string> vecClientIDs;
    std::vector<std::string>::iterator itClientIDs;
    std::map<std::string, std::string>::iterator iprimConf;
    MYSQL *mconnect2;
    MYSQL_ROW row;


    ReadDBConfFile();
    conf1.ReadConfFile();
    strRecNum = "1";
    if(mysql_library_init(0, NULL, NULL)) {
        strLogMessage="ERROR:  Could not initalize MySQL client library";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
        exit(-1);
    }

    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect2) {
        strLogMessage="MySQL Initilization failed in fSelectPriors";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
        return -1;
    }
    mconnect2=mysql_real_connect(mconnect2, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect2) {
        strLogMessage="connection failed in fSelectPriors";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
        return -1;
    }
    MYSQL_RES *result = mysql_store_result(mconnect2);

    strRecNum = "1";
    strLogMessage = "Starting prim_qr_poll_server version 1.00.01";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
    //Get a list of client IDs we need to poll for
    if(fs::exists("/etc/primal/prim_ae_map.conf")) {
        std::ifstream fpAEMap("/etc/primal/prim_ae_map.conf");
        while (std::getline(fpAEMap, strLine)) {
            intFound=strLine.find(",");
            strClientID = strLine.substr(0, intFound);
            if(std::find(vecClientIDs.begin(), vecClientIDs.end(), strClientID) == vecClientIDs.end()) {
                vecClientIDs.push_back(strClientID);
                strLogMessage = " POLLING QR Adding " + strClientID + " to list of client ID to poll";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                //std::cout << strLogMessage << std::endl;
            }
        }
        fpAEMap.close();
    }
    while(1) {
        for (itClientIDs = vecClientIDs.begin(); itClientIDs != vecClientIDs.end(); ++itClientIDs) {
            strCMD = "curl -s --location --request POST 'https://sheridan.candescenthealth.com/radconnect/getPriorsDownload.db.php' ";
            strCMD += "--form 'challenge=94dfcb464afd2725297f2bbcc384f57a' ";
            strCMD += "--form 'clients_id=" + *itClientIDs + "' ";
            strCMD += "--form 'calling_ae=RCONN' ";
            strCMD += "--form 'called_ae=APEX_" + *itClientIDs + "' ";
            strReturn = exec(strCMD.c_str());
            strLogMessage="Polling strCMD\n";
            strLogMessage+=strCMD + "\n";
            strLogMessage+="Polling strReturn\n";
            strLogMessage+=strReturn + "\n";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
        }
        while (strReturn.length() > 14) {
            intPos = strReturn.find("}");
            if(intPos != std::string::npos) {
                strLine = strReturn.substr(0, intPos);
            } else {
                strLine = strReturn;
            }
            intPos2 = strLine.find("case_priors_id");
            if(intPos2 != std::string::npos) {
                intPos3 = strLine.find_first_of(",");
                if(intPos3 == std::string::npos) {
                    intPos3 = strLine.find_first_of("]");
                }
                strCasePriorsID = strLine.substr(intPos2 + 17, intPos3 - intPos2 - 18);
            }
            intPos2 = strLine.find("prefetch_results_id");
            if(intPos2 != std::string::npos) {
                intPos3 = strLine.find_last_of("\"");
                //std::cout << "intPos2 = " << intPos2 << "intPos3 = " << intPos3 << std::endl;
                strPrefetchID = strLine.substr(intPos2 + 22, intPos3 - intPos2 - 22);
                strLogMessage="case_priors_id: " + strCasePriorsID + "   prefetch_results_id: " + strPrefetchID;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                strQuery = "select QR.puid, QR.SIUID, QR.qraet, QR.qraec, QR.qrport, QR.qrip, QR.study_description, QR.modality, QR.qrstatus, study.sClientID from QR left join study on QR.puid = study.puid where prefetch_results_id = '" + strPrefetchID + "';";
                mysql_query(mconnect2, strQuery.c_str());
                //strLogMessage = strQuery;
                //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFQR"]);
                //std::cout << strQuery << std::endl;
                if(*mysql_error(mconnect2)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect2);
                    strLogMessage+="\nstrQuery = " + strQuery + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                }
                result = mysql_store_result(mconnect2);
                if(result) {
                    intDBEntries=mysql_num_rows(result);
                    if(intDBEntries > 0) {
                        row = mysql_fetch_row(result);
                        strPrimalID = row[0];
                        strSIUID = row[1];
                        strQRaet = row[2];
                        strQRaec = row[3];
                        strQRport = row[4];
                        strQRip = row[5];
                        strSDesc = row[6];
                        strMOD = row[7];
                        strQRstatus = row[8];
                        strClientID = row[9];
                        intPos4 = strPrimalID.find("_");
                        if(intPos4 != std::string::npos) {
                            strRecNum = strPrimalID.substr(0,intPos4);
                        } else {
                            strRecNum = "1";
                            strLogMessage = strPrimalID + " QR " + " WARN  Unable to determine receiver number.  Setting to 1. ";
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                            //std::cout << strLogMessage << std::endl;
                        }
                        //std::cout << "strRecNum = " << strRecNum << std::endl;
                        iprimConf = conf1.primConf.find(strRecNum + "_PRIQRMOVE" + to_string(0));
                        if(iprimConf != conf1.primConf.end()) {
                           strQRaec = conf1.primConf[strRecNum + "_PRIQRMOVE" + to_string(0)];
                        }
                        if(strQRstatus == "Hold" || strQRstatus == "New") {
                            strCMD = "movescu -d -S -k 0008,0052=STUDY -aec " + strQRaec + " -aet " + strQRaet + " " + strQRip + " " + strQRport;
                            strCMD += " -k 0020,000D=" + strSIUID + " &";
                            strLogMessage = strPrimalID + " QR " + " Directory " + strCMD;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                            //std::cout << strLogMessage << std::endl;
                            strQuery="update QR set qrstatus='Requested' where prefetch_results_id = '" + strPrefetchID + "';";
                            mysql_query(mconnect2, strQuery.c_str());
                            if(*mysql_error(mconnect2)) {
                                strLogMessage=mysql_error(mconnect2);
                                strLogMessage+="\n" + strQuery;
                                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                            }
                            system(strCMD.c_str());
                            fAckRequest(strClientID, strCasePriorsID);
                        } else {
                            strLogMessage = strPrimalID + " QR WARN:  Skipping Prefetch Results ID " + strPrefetchID + " because it is in status " + strQRstatus;
                            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                        }
                        mysql_free_result(result);
                    }
                } else {
                    strLogMessage = "ERROR:  Couldn't get Primal ID";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/qr_poll.log");
                }
            }
            if(intPos != std::string::npos) {
                strReturn.erase(0, intPos + 1);
            } else {
                strReturn.erase(0,strReturn.length());
            }
        }
        std::this_thread::sleep_for (std::chrono::seconds(10));
    }


    mysql_library_end();

    return 0;
}