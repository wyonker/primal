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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <openssl/md5.h>
#include <sys/stat.h>

using namespace std;
namespace fs = std::filesystem;
MYSQL *mconnect;
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

void fAddEntries(std::string strCurPath, std::string strParentDir, std::list<std::string> &listFileSystem) {
    std::string strCmd, strReturn, strLine, strLine2;
    std::size_t intPos;

    if(strCurPath == " " || strCurPath == "") {
        strCmd = "aws s3 ls s3://" + strParentDir;
    } else {
        strCmd = "aws s3 ls s3://" + strParentDir + strCurPath;
    }
    strReturn = exec(strCmd.c_str());
    std::istringstream istr(strReturn);
    while(std::getline(istr, strLine)) {
        intPos=strLine.find_last_of(" ");
        if(intPos != std::string::npos) {
            strLine2 = strLine.substr(intPos + 1);
        } else {
            strLine2 = strLine;
        }
        if(strLine2.back() != '/') {
            strLine2 += "/";
        }
        listFileSystem.push_back(strCurPath + strLine2);
        std::cout << "Added: " << listFileSystem.back() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    listFileSystem.erase(std::remove(listFileSystem.begin(), listFileSystem.end(), strCurPath), listFileSystem.end());

    return;
}

void fAddList(std::string strCurPath, std::string strParentDir, std::list<std::string> &listFileSystem) {
    std::string strCmd, strReturn, strLine, strLine2, strPath, strHandle;
    std::size_t intPos, intDone;
    (void) strParentDir;

    std::cout << "Getting new meessage from SQS." << std::endl;
    intDone = 0;
    while(intDone == 0) {
        /*
        strCmd = "aws sqs receive-message --queue-url https://sqs.us-east-1.amazonaws.com/313891674905/candh-dicom-queue ";
        strCmd += "--attribute-names All --message-attribute-names All --max-number-of-messages 1|grep '\"Body\"'|";
        strCmd += "tr \":\" \"\n\"|grep \"auto-expire\"|cut -d '\"' -f2";
        */
        strCmd = "aws sqs receive-message --queue-url https://sqs.us-east-1.amazonaws.com/313891674905/candh-dicom-queue ";
        strCmd += "--attribute-names All --message-attribute-names All --max-number-of-messages 1";

        strReturn = exec(strCmd.c_str());
        if(strReturn.length() > 10) {
            //std::cout << "Message: " << std::endl;
            //std::cout << strReturn << std::endl;
            intPos = strReturn.find("key\\\":\\\"");
            if(intPos != std::string::npos) {
                strLine2 = strReturn.substr(intPos + 8);
                intPos = strLine2.find("\\\",\\\"");
                if(intPos != std::string::npos) {
                    strPath = strLine2.substr(0, intPos);
                    intPos = strPath.find_last_of("/");
                    if(intPos != std::string::npos) {
                        strPath = strPath.substr(0, intPos);
                    }
                    if(strPath.back() != '/') {
                        strPath += "/";
                    }
                    //std::cout << "Path = " << strPath << "." << std::endl;
                } else {
                    std::cout << "ERROR:  Could not parse strReturn." << std::endl;
                    std::cout << strReturn << std::endl;
                    exit(-1);
                }
            }
            intPos = strReturn.find("\"ReceiptHandle\": \"");
            if(intPos != std::string::npos) {
                strLine2 = strReturn.substr(intPos + 18);
                intPos = strLine2.find("\", ");
                if(intPos != std::string::npos) {
                    strHandle = strLine2.substr(0, intPos);
                    //std::cout << "Handle = " << strHandle << "." << std::endl;
                } else {
                    std::cout << "ERROR:  Could not parse strReturn." << std::endl;
                    std::cout << strReturn << std::endl;
                    exit(-1);
                }
            } else {
                std::cout << "ERROR:  Could not parse strReturn." << std::endl;
                std::cout << strReturn << std::endl;
                exit(-1);
            }
            strCmd = "aws sqs delete-message --queue-url https://sqs.us-east-1.amazonaws.com/313891674905/candh-dicom-queue ";
            strCmd += "--receipt-handle " + strHandle;
            strReturn = exec(strCmd.c_str());
            //std::cout << "Return after delete: " << std::endl;
            //std::cout << strReturn << std::endl;
            listFileSystem.push_back(strCurPath + strPath);
            //std::cout << "Added: " << listFileSystem.back() << std::endl;
            //std::this_thread::sleep_for(std::chrono::milliseconds(1));
            listFileSystem.erase(std::remove(listFileSystem.begin(), listFileSystem.end(), strCurPath), listFileSystem.end());
            intDone=1;
        } else {
            std::cout << "No messages in the queue.  Sleeping for 10 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }

    return;
}

void fCrawlRoot(std::string &strParentDir, std::list<std::string> &listRootFileSystem) {
    std::string strCmd, strReturn, strLine, strLine2;
    std::size_t intPos;

    strCmd = "aws s3 ls s3://" + strParentDir;
    strReturn = exec(strCmd.c_str());
    std::istringstream istr(strReturn);
    while(std::getline(istr, strLine)) {
        intPos=strLine.find_last_of(" ");
        if(intPos != std::string::npos) {
            strLine2 = strLine.substr(intPos + 1);
        } else {
            strLine2 = strLine;
        }
        if(strLine2.back() != '/') {
            strLine2 += "/";
        }
        listRootFileSystem.push_back(strParentDir + strLine2);
        std::cout << "Added: " << listRootFileSystem.back() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return;
}

std::string fPullAndAdd(std::string strCurPath, std::string strLocalDir, std::string strParentDir) {
    std::string strTemp, strDirName, strLogMessage, strCmd, strReturn, strLine, strLine2;
    std::size_t intPos, intRand;

    strTemp = strCurPath;
    strTemp.pop_back();
    intPos=strTemp.find_last_of("/");
    if(intPos != std::string::npos) {
        strDirName = strTemp.substr(intPos + 1);
    } else {
        intRand = rand() % 10000 + 1;
        strDirName = "Temp" + to_string(intRand);
    }
    strLogMessage = " STOR Creating directory " + strLocalDir + strDirName;
    //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
    std::cout << strLogMessage << std::endl;
    try {
        fs::create_directory(strLocalDir + strDirName);
    }
    catch (fs::filesystem_error& err) {
        strLogMessage = " STOR ERROR: Could not make directory " + strLocalDir + strDirName + ".  Skipping message...";
        //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        std::cout << strLogMessage << std::endl;
        return "Error";
    }
    strCmd = "aws s3 ls s3://" + strParentDir + strCurPath;
    std::cout << "strCmd = " << strCmd << std::endl;
    strReturn = exec(strCmd.c_str());
    std::istringstream istr(strReturn);
    while(std::getline(istr, strLine)) {
        intPos=strLine.find_last_of(" ");
        if(intPos != std::string::npos) {
            strLine2 = strLine.substr(intPos + 1);
        } else {
            strLine2 = strLine;
        }
        //strLogMessage = " STOR Downloading " + strParentDir + strCurPath + strLine2 + " to /tmp/" + strDirName + "/" + strLine2;
        //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        //std::cout << strLogMessage << std::endl;
        strCmd = "aws s3 cp s3://" + strParentDir + strCurPath + strLine2 + " " + strLocalDir + strDirName + "/";
        //std::cout << strCmd << std::endl;
        system(strCmd.c_str());
        //strCmd = "ls -l /tmp/" + strDirName + "/*|wc -l";
        //strReturn=exec(strCmd.c_str());
        //strLogMessage = " STOR Downlaoded " + strReturn + " files.";
        //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFIN"]);
        //std::cout << strLogMessage << std::endl;
    }
    if(fs::exists(strLocalDir + strDirName + "/_DICOM.tar.gz")) {
        strCmd = "(cd " + strLocalDir + strDirName + " && tar -xf " + strLocalDir + strDirName + "/_DICOM.tar.gz)";
        //std::cout << strCmd << std::endl;
        system(strCmd.c_str());
        strCmd = "find " + strLocalDir + strDirName + " -iname \"*.dcm\" -exec mv {} " + strLocalDir + strDirName + "/ \\;";
        //std::cout << strCmd << std::endl;
        system(strCmd.c_str());
    }
    return strDirName;
}

std::string fReadPackage (std::string strFullPath) {
    std::string strLogMessage, strPrimID, strLine, strLine2, strReturn;
    std::size_t intPos;

    if(fs::exists(strFullPath + "/package.conf")) {
        fstream fpPackage;
        fpPackage.open(strFullPath + "/package.conf", ios::in);
        if(fpPackage.is_open()) {
            while(getline(fpPackage, strLine)) {
                intPos = strLine.find("ID =");
                if(intPos != std::string::npos) {
                    strLine2=strLine;
                    intPos = strLine2.find_last_of(" ");
                    if(intPos != std::string::npos) {
                        strReturn = strLine2.substr(intPos +1);
                        std::cout << "ClientID = " << strReturn << std::endl;
                        break;
                    }
                }
            }
        }
        fpPackage.close();
    }
    return strReturn;
}

std::string fCleanString(std::string strOriginal) {

    strOriginal.erase(std::remove(strOriginal.begin(), strOriginal.end(), '\0'), strOriginal.end());
    strOriginal.erase(std::remove(strOriginal.begin(), strOriginal.end(), '\''), strOriginal.end());
    strOriginal.erase(std::remove(strOriginal.begin(), strOriginal.end(), '"'), strOriginal.end());
    strOriginal.erase(std::remove(strOriginal.begin(), strOriginal.end(), '\\'), strOriginal.end());

    return strOriginal;
}

void fAddToDB (std::string strLocalPath, std::string strCurPath, std::string strParentDir) {
    std::string strClientID, strTemp2, strTemp3, strFilename, strRawDCMdump, strAccn, strMod, strPname, strMRN, strDOB, strSex;
    std::string strSerIUID, strSerDesc, strModality, strSopIUID, strSIUID, strStudyDate, strStudyTime, strStudyDateTime;
    std::string strACCN, strStudyDesc, strPatientComments, strQuery, strDBREturn, strTemp4, strTemp;
    std::size_t intImgNum, intTemp, intPos, intDBEntries, intStudyID, intFolderID, intLC, intDone2;

    MYSQL_ROW row;
    MYSQL_RES *result = mysql_store_result(mconnect);

    strClientID = fReadPackage(strLocalPath);
    intImgNum=0;
    for (const auto & entry : fs::directory_iterator(strLocalPath)) {
        strTemp2=entry.path().string();
        intTemp = strTemp2.find_last_of("/");
        strFilename=strTemp2.substr(intTemp+1);
        intPos=strFilename.find_last_of(".");
        if(intPos != std::string::npos) {
            strTemp3=strFilename.substr(intPos);
        }
        if(strTemp3 == ".dcm") {
            intImgNum++;
            //std::cout << "Performing dcmdump and get tag values for " << strTemp2 << std::endl;
            strRawDCMdump=fDcmDump(strTemp2);
            strAccn=fGetTagValue("0008,0050", strRawDCMdump, 0);
            strAccn=fCleanString(strAccn);
            strMod=fGetTagValue("0008,0060", strRawDCMdump, 0);
            strPname=fGetTagValue("0010,0010", strRawDCMdump, 0);
            strPname=fCleanString(strPname);
            strMRN=fGetTagValue("0010,0020", strRawDCMdump, 0);
            strMRN=fCleanString(strMRN);
            strDOB=fGetTagValue("0010,0030", strRawDCMdump, 0);
            strSex=fGetTagValue("0010,0040", strRawDCMdump, 0);
            strSex=fCleanString(strSex);
            strSerIUID=fGetTagValue("0020,000e", strRawDCMdump, 0);
            strSerDesc=fGetTagValue("0008,103e", strRawDCMdump, 0);
            strSerDesc=fCleanString(strSerDesc);
            strModality=fGetTagValue("0008,0060", strRawDCMdump, 0);
            strSopIUID=fGetTagValue("0008,0018", strRawDCMdump, 0);
            strSopIUID=fCleanString(strSopIUID);
            strSIUID=fGetTagValue("0020,000d", strRawDCMdump, 0);
            strSIUID=fCleanString(strSIUID);
            strStudyDate=fGetTagValue("0008,0020", strRawDCMdump, 0);
            strStudyTime=fGetTagValue("0008,0030", strRawDCMdump, 0);
            strStudyDateTime = strStudyDate + " " + strStudyTime;
            //strACCN=fGetTagValue("0008,0050", strRawDCMdump, 0);
            strStudyDesc=fGetTagValue("0008,1030", strRawDCMdump, 0);
            strStudyDesc=fCleanString(strStudyDesc);
            strPatientComments=fGetTagValue("0010,4000", strRawDCMdump, 0);
            strPatientComments=fCleanString(strPatientComments);
            std::cout << "Got all tag values for " << strTemp2 << std::endl;
            if(intImgNum == 1) {
                //Need to lob off the dicom-XX part of the path.
                strTemp = strCurPath;
                if(strTemp.back() == '/') {
                    strTemp.pop_back();
                }
                intPos = strTemp.find_last_of("/");
                if(intPos != std::string::npos) {
                    strTemp4 = strTemp.substr(0, intPos);
                } else {
                    strTemp4 = strTemp;
                }
                //First see if there is alredy an entry
                std::cout << "strMRN = '" << strMRN << "'." << std::endl;
                strQuery = "select count(*) from study where dirfullpath = '" + strParentDir + strTemp4 + "' and ";
                strQuery += "pid = '" + strMRN + "' and dob = '" + strDOB + "' limit 1;";
                std::cout << strQuery << std::endl;
                mysql_query(mconnect, strQuery.c_str());
                result = mysql_store_result(mconnect);
                if(result == NULL) {
                    intDBEntries=0;
                } else {
                    row = mysql_fetch_row(result);
                    strDBREturn=row[0];
                    intDBEntries=stoi(strDBREturn);
                }
                mysql_free_result(result);
                std::cout << "Got " << to_string(intDBEntries) << " entries." << std::endl;
                if(intDBEntries < 1) {
                    std::cout << "Adding to study Patient: " << strPname << " MRN: " << strMRN << " ACCN: " << strAccn << std::endl;
                    intLC=0;
                    intDone2=0;
                    while(intLC < 10 && intDone2 != 1) {
                        strQuery = "insert into study (pname, pid, dob, sex, clientid, SIUID, StudyDesc, AccessionNum, StudyDate, ";
                        strQuery += "StudyModType, CaseID, dirfullpath) values ('" + strPname + "', '" + strMRN + "', '" + strDOB;
                        strQuery += "', '" + strSex + "', '" + strClientID + "', '" + strSIUID + "', '" + strStudyDesc + "', '" + strAccn;
                        strQuery += "', '" + strStudyDate + "', '" + strModality + "', '" + "0" + "', '" + strParentDir + strTemp4 + "');";
                        std::cout << strQuery << std::endl;
                        mysql_query(mconnect, strQuery.c_str());
                        if(*mysql_error(mconnect)) {
                            std::cout << "SQL Error: " << mysql_error(mconnect) << std::endl;
                        }
                        strQuery = "select count(*) from study where dirfullpath = '" + strParentDir + strTemp4 + "' limit 1;";
                        std::cout << strQuery << std::endl;
                        mysql_query(mconnect, strQuery.c_str());
                        result = mysql_store_result(mconnect);
                        if(result == NULL) {
                            intDBEntries=0;
                        } else {
                            row = mysql_fetch_row(result);
                            strDBREturn=row[0];
                            intDBEntries=stoi(strDBREturn);
                        }
                        mysql_free_result(result);
                        if(intDBEntries < 1) {
                            std::this_thread::sleep_for (std::chrono::seconds(1));
                            intLC++;
                        } else {
                            intDone2 = 1;
                        }
                    }
                    if(intDone2 != 1) {
                        std::cout << "ERROR:  inserted " << strParentDir << strCurPath << " but couldn't query for it.  Exiting..." << std::endl;
                        exit (1);
                    } else {
                        strQuery = "select study_id from study where dirfullpath = '" + strParentDir + strTemp4 + "' limit 1;";
                        mysql_query(mconnect, strQuery.c_str());
                        result = mysql_store_result(mconnect);
                        row = mysql_fetch_row(result);
                        strDBREturn=row[0];
                        intStudyID=stoi(strDBREturn);
                        mysql_free_result(result);
                    }
                } else {
                    std::cout << "Getting the StudyID for Patient: " << strPname << " MRN: " << strMRN << " ACCN: " << strAccn << std::endl;
                    strQuery = "select count(*) from study where dirfullpath = '" + strParentDir + strTemp4 + "' limit 1;";
                    std::cout << strQuery << std::endl;
                    mysql_query(mconnect, strQuery.c_str());
                    result = mysql_store_result(mconnect);
                    if(result == NULL) {
                        intDBEntries=0;
                    } else {
                        row = mysql_fetch_row(result);
                        strDBREturn=row[0];
                        intDBEntries=stoi(strDBREturn);
                    }
                    mysql_free_result(result);
                    if(intDBEntries < 1) {
                        std::cout << "ERROR:  inserted " << strParentDir << strCurPath << " but couldn't query for it.  Exiting..." << std::endl;
                        exit (1);
                    } else {
                        strQuery = "select study_id from study where dirfullpath = '" + strParentDir + strTemp4;
                        strQuery += "' and pid='" + strMRN + "' and dob = '" + strDOB + "' limit 1;";
                        std::cout << strQuery << std::endl;
                        mysql_query(mconnect, strQuery.c_str());
                        result = mysql_store_result(mconnect);
                        row = mysql_fetch_row(result);
                        strDBREturn=row[0];
                        intStudyID=stoi(strDBREturn);
                        mysql_free_result(result);
                    }
                }
                std::cout << "Adding to folder Patient: " << strPname << " MRN: " << strMRN << " ACCN: " << strAccn << std::endl;
                strQuery = "insert into folder (parentid, dirfullpath, pname, pid, dob, sex, clientid, SIUID, StudyDesc, AccessionNum,";
                strQuery += "StudyDate, StudyModType) values ('" + to_string(intStudyID) + "', '" + strParentDir + strCurPath + "', '" + strPname;
                strQuery += "', '" + strMRN + "', '" + strDOB + "', '" + strSex + "', '" + strClientID  + "', '" + strSIUID;
                strQuery += "', '" + strStudyDesc + "', '" + strAccn + "', '" + strStudyDate + "', '" + strModality + "');";
                std::cout << strQuery << std::endl;
                mysql_query(mconnect, strQuery.c_str());
                if(*mysql_error(mconnect)) {
                    std::cout << "SQL Error: " << mysql_error(mconnect) << std::endl;
                }
                strQuery = "select folder_id from folder where dirfullpath = '" + strParentDir + strCurPath + "' limit 1;";
                std::cout << strQuery << std::endl;
                mysql_query(mconnect, strQuery.c_str());
                result = mysql_store_result(mconnect);
                if(result == NULL) {
                    intFolderID=0;
                } else {
                    row = mysql_fetch_row(result);
                    strDBREturn=row[0];
                    intFolderID=stoi(strDBREturn);
                }
                mysql_free_result(result);
            }
            strQuery = "select count(*) from instance where dirfullpath = '" + strParentDir + strCurPath;
            strQuery += "' and SOPIUID = '" + strSopIUID + "';";
            mysql_query(mconnect, strQuery.c_str());
            result = mysql_store_result(mconnect);
            if(result == NULL) {
                intDBEntries=0;
            } else {
                row = mysql_fetch_row(result);
                strDBREturn=row[0];
                intDBEntries=stoi(strDBREturn);
            }
            mysql_free_result(result);
            if(intDBEntries < 1) {
                std::cout << "Adding to instance SOPIUID: " << strSopIUID << " ACCN: " << strAccn << std::endl;
                strQuery = "insert into instance (parentid, folderid, SOPIUID, dirfullpath, image_mod) values ('" + to_string(intStudyID);
                strQuery += "', '" + to_string(intFolderID) + "', '" + strSopIUID + "', '" + strParentDir + strCurPath + "', '" + strModality + "');";
                mysql_query(mconnect, strQuery.c_str());
                if(*mysql_error(mconnect)) {
                    std::cout << "SQL Error: " << mysql_error(mconnect) << std::endl;
                }
                strQuery = "select count(*) from instance where dirfullpath = '" + strParentDir + strCurPath;
                strQuery += "' and SOPIUID = '" + strSopIUID + "';";
                mysql_query(mconnect, strQuery.c_str());
                if(*mysql_error(mconnect)) {
                    std::cout << "SQL Error: " << mysql_error(mconnect) << std::endl;
                }
                result = mysql_store_result(mconnect);
                row = mysql_fetch_row(result);
                strDBREturn=row[0];
                intDBEntries=stoi(strDBREturn);
                mysql_free_result(result);
                if(intDBEntries < 1) {
                    std::cout << "ERROR: Could not add SOPIUID: " << strSopIUID << " path: " << strParentDir << strCurPath << ".  Exiting...";
                    exit (1);
                }
            }
        }
    }
    return;
}

void fCrawlTree(std::string strParentDir, std::string strLocalDir) {
    std::string strCmd, strCurPath, strLine, strClientID, strLocalPath, strQuery, strDBREturn;
    std::size_t intDone, intPos, intDBEntries;
    std::list<std::string> listFileSystem;

    MYSQL_ROW row;
    MYSQL_RES *result = mysql_store_result(mconnect);
    
    std::cout << "strLocalDir = " << strLocalDir << "." << std::endl;
    if(!fs::exists(strLocalDir)) {
        std::cout << "Creating directory " << strLocalDir << std::endl;
        try {
            fs::create_directory(strLocalDir);
        } catch (fs::filesystem_error& err) {
            std::cout << "ERROR:  Could not create directory " << strLocalDir << ".  Exiting..." << std::endl;
            return;
        }
    } else {
        std::cout << "Removing existing directory " << strLocalDir << std::endl;
        try {
            fs::remove_all(strLocalDir);
            fs::create_directory(strLocalDir);
        } catch (fs::filesystem_error& err) {
            std::cout << "ERROR:  Could not create directory " << strLocalDir << ".  Exiting..." << std::endl;
            return;
        }
    }
    strCurPath="";
    if(strParentDir == "candh-dicom/") {
        fAddList(strCurPath, strParentDir, listFileSystem);
    } else {
        fAddEntries(strCurPath, strParentDir, listFileSystem);
    }
    intDone = 0;
    while (intDone == 0) {
        strCurPath = listFileSystem.back();
        intPos = strCurPath.find("dicom-");
        if(intPos != std::string::npos) {
            //Need to do process directories different here.
            std::cout << "Processing " << strCurPath << std::endl;
            intPos = strCurPath.find("candh-dicom/auto-expire/sheridan/dicom-bridge");
            if(intPos == std::string::npos) {
                strQuery = "select count(*) from folder where dirfullpath = '" + strParentDir + strCurPath + "';";
                mysql_query(mconnect, strQuery.c_str());
                if(*mysql_error(mconnect)) {
                    std::cout << "SQL Error: " << mysql_error(mconnect) << std::endl;
                    std::cout << "strQuery = " << strQuery << "." << std::endl;
                    exit(-1);
                }
                if(result) {
                    result = mysql_store_result(mconnect);
                    row = mysql_fetch_row(result);
                    strDBREturn=row[0];
                    intDBEntries=stoi(strDBREturn);
                    mysql_free_result(result);
                } else {
                    intDBEntries=0;
                }
                if(intDBEntries < 1) {
                    strLocalPath = strLocalDir + fPullAndAdd(strCurPath, strLocalDir, strParentDir);
                    fAddToDB(strLocalPath, strCurPath, strParentDir);
                    listFileSystem.erase(std::remove(listFileSystem.begin(), listFileSystem.end(), strCurPath), listFileSystem.end());
                    try {
                        fs::remove_all(strLocalPath);
                    } catch (fs::filesystem_error& err) {
                        std::cout << "ERROR:  Could not remove directory " << strLocalPath << ".  Exiting...";
                        return;
                    }
                } else {
                    std::cout << "WARN:  Directory " << strParentDir << strCurPath << " has been processed already.  Skipping..." << std::endl;
                    listFileSystem.erase(std::remove(listFileSystem.begin(), listFileSystem.end(), strCurPath), listFileSystem.end());
                }
            } else {
                std::cout << "WARN:  Directory is not production.  Skipping..." << strCurPath << std::endl;
            }
        } else {
            if(strParentDir == "candh-dicom/") {
                fAddList(strCurPath, strParentDir, listFileSystem);
            } else {
                fAddEntries(strCurPath, strParentDir, listFileSystem);
            }
        }
        std::cout << listFileSystem.size() << " elements left to process." << std::endl;
        if(listFileSystem.size() < 1) {
            std::cout << "No directories left to process for " << strParentDir << ".  Exiting...";
            return;
        }
    }
}

int main(int argc, char** argv) {
    std::string strTemp;
    std::string strParentDir="candh-dicom/auto-expire/sheridan/dicom-bridge/";
    std::string strLocalDir="/tmp/";
    std::size_t intDone;
    std::list<std::string> listRootFileSystem;
    std::list<std::string>::iterator itRootFileSystem;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    ReadDBConfFile();

    //mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
    mconnect=mysql_real_connect(mconnect, "10.85.55.127", "primalindex2", "ThisGuy1!", "primal_index", 3306,NULL,0);
    if (!mconnect) {
        cout<<"connection failed\n";
        return 1;
    }
    if(argc < 2) {
        strTemp = "000";
    }
    strTemp = argv[1];
    std::cout << "prim_index_server version 1.00.07" << std::endl;
    if(strTemp == "candh-dicom") {
        strParentDir = strTemp + "/";
        pid_t pid = getpid();
        strLocalDir += strTemp + to_string(pid) + "/";
        intDone=0;
        while(intDone != 1) {
            std::cout << "Getting 10 messages..." << strParentDir << std::endl;
            fCrawlTree(strParentDir, strLocalDir);
        }
    } else {
        strParentDir += strTemp + "/";
        strLocalDir += strTemp + "/";
        std::cout << "Starting in: " << strParentDir << std::endl;
        fCrawlTree(strParentDir, strLocalDir);
    }
    return 0;
}