
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <openssl/md5.h>

using namespace std;
namespace fs = std::filesystem;
MYSQL *mconnect;
MYSQL *mconnect2;
MYSQL *mconnect3;

//#include "prim_functions.h"

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

int main(int argc, char** argv) {
    //time_t t2 = time(0);   // get time now
    //struct tm * now2 = localtime( & t2 );
    std::string strLine, strClientID, strGWIP, strQuery, strCaseID, strACCN, strSIUID, strQuery2, strLogMessage, strThisIP, strThisName;
    std::string strPath, strFilename, strCmd, strQuery3, strQuery4, strReturn;
    std::size_t intPos, intResults, intLC, intDicomCasesID, intNumRows, intReturn;
    std::time_t tmStartTime;
    std::stringstream sstream("1");

    MYSQL_ROW row4;
    MYSQL_RES *result4;

    if(argc != 3) {
        std::cout << "Wrong number of arguments: " << to_string(argc) << std::endl;
    }

    std::vector<std::vector<std::string> > vecDCB { 
        { "dicom_bridge_01", "10.85.50.80" },
        { "dicom_bridge_02", "10.85.51.80" },
        { "dicom_bridge_03", "10.85.50.81" },
        { "dicom_bridge_04", "10.85.51.81" },
        { "dicom_bridge_05", "10.85.50.82" },
        { "dicom_bridge_06", "10.85.51.82" },
        { "dicom_bridge_07", "10.85.50.83" }
    };
    std::vector<std::vector<std::string> >::iterator itvecDCB;
    std::vector<std::string>::iterator itcol;

    std::cout << "prim_aidoc version 2.01.02" << std::endl;
    strClientID = argv[1];
    strGWIP = argv[2];
    mconnect=mysql_init(NULL);
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    mysql_options(mconnect,MYSQL_OPT_READ_TIMEOUT,"10");
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    mysql_options(mconnect2,MYSQL_OPT_READ_TIMEOUT,"10");
    while (!mconnect) {
        cout << "MySQL Initilization failed.  Sleeping for 5 seconds before trying again.";
        std::this_thread::sleep_for (std::chrono::seconds(5));
        mconnect=mysql_init(NULL);
    }
    while (!mconnect2) {
        cout << "MySQL Initilization failed for 2nd connector.  Sleepign for 5 seconds before trying again.";
        std::this_thread::sleep_for (std::chrono::seconds(5));
        mconnect2=mysql_init(NULL);
    }
    mconnect=mysql_real_connect(mconnect, strGWIP.c_str(), "primal" , "ThisGuy1!", "rConnect", 3306,NULL,0);
    while (!mconnect) {
        std::cout << "connection failed for " << strGWIP << " Sleeping for 5 seconds before trying again." << std::endl;
        std::this_thread::sleep_for (std::chrono::seconds(5));
        mysql_close(mconnect);
        mconnect=mysql_real_connect(mconnect, strGWIP.c_str(), "primal" , "ThisGuy1!", "rConnect", 3306,NULL,0);
    }
    mconnect2=mysql_real_connect(mconnect2, "localhost", "primal" , "primal", "aidoc", 3306,NULL,0);
    while (!mconnect2) {
        std::cout << "connection failed for aidoc on localhost.  Sleepign for 5 seconds before trying again." << std::endl;
        std::this_thread::sleep_for (std::chrono::seconds(5));
        mysql_close(mconnect2);
        mconnect2=mysql_real_connect(mconnect2, "localhost", "primal" , "primal", "aidoc", 3306,NULL,0);
    }
    while(1) {
        tmStartTime = std::time(nullptr);
        std::cout << "Checking " << strGWIP << " for client ID " << strClientID << "." << std::endl;
        strQuery = "select cases_id, client_configuration.id, series_level.modality, study_level.accession_number, study_level.study_instance_uid, dicom_cases.dicom_cases_id from dicom_cases ";
        strQuery += "left join local_ae on dicom_cases.local_title = local_ae.local_title ";
        strQuery += "left join application_entity on application_entity.local_ae_id = local_ae.local_ae_id ";
        strQuery += "left join client_configuration on application_entity.client_configuration_id = client_configuration.client_configuration_id ";
        strQuery += "left join series_level on dicom_cases.dicom_cases_id = series_level.dicom_cases_id ";
        strQuery += "left join study_level on dicom_cases.dicom_cases_id = study_level.dicom_cases_id ";
        strQuery += "where date_receive_started >= DATE_SUB(NOW(), INTERVAL 1 HOUR) ";
        strQuery += "and dicom_cases.local_title NOT IN ( SELECT VALUE FROM system_configuration WHERE entry = \"dicom;prefetch;filter;node\" ) ";
        strQuery += "and dicom_cases.local_title != \"STORE_CHECK\" ";
        strQuery += "and series_level.modality = \"CT\" ";
        strQuery += "and client_configuration.id = \"" + strClientID + "\" group by dicom_cases.dicom_cases_id;";
        //strQuery += "group by study_level.study_instance_uid;";
        //std::cout << "strQuery = " << strQuery << std::endl;
        if(mysql_query(mconnect, strQuery.c_str())) {
            std::cerr << mysql_error(mconnect) << std::endl;
            mysql_close(mconnect);
            return 1;
        }
        MYSQL_RES *result = mysql_store_result(mconnect);
        if(result == NULL) {
            std::cerr << mysql_error(mconnect) << std::endl;
            mysql_close(mconnect);
            return 1;
        }
        MYSQL_ROW row;
        while( (row = mysql_fetch_row(result)) != NULL ) {
            if(row[0] != NULL) {
                strCaseID = row[0];
            } else {
                strCaseID = "0";
            }
            if(row[3] != NULL) {
                strACCN = row[3];
            } else {
                strACCN = "0";
            }
            if(row[4] != NULL) {
                strSIUID = row[4];
            } else {
                std::cout << "SIUID is blank.  Skipping..." << std::endl;
                continue;
            }
            if(row[5] != NULL) {
                sstream.clear();
                sstream.str(row[5]);
                sstream >> intDicomCasesID;
            }
            std::cout << "Found CaseID: " << strCaseID << " ACCN: " << strACCN << " SIUID: " << strSIUID << std::endl;
            strQuery2 = "select count(*) from studies where SIUID = '" + strSIUID + "' and dicom_cases_id = " + to_string(intDicomCasesID) + ";";
            if(mysql_query(mconnect2, strQuery2.c_str())) {
                std::cerr << mysql_error(mconnect2) << std::endl;
                mysql_close(mconnect2);
                return 1;
            }
            MYSQL_RES *result2 = mysql_store_result(mconnect2);
            if(result2 == NULL) {
                std::cerr << mysql_error(mconnect2) << std::endl;
                mysql_close(mconnect2);
                return 1;
            }
            MYSQL_ROW row2;
            if( (row2 = mysql_fetch_row(result2)) != NULL ) {
                intResults = atoi(row2[0]);
                mysql_free_result(result2);
                if(intResults < 1) {
                    //We have not seen this SIUID before.  Better send it.
                    std::cout << "Searching DCB for SIUID: " << strSIUID << std::endl;
                    for (itvecDCB = vecDCB.begin(); itvecDCB != vecDCB.end(); ++itvecDCB) {
                        intLC=0;
                        for (itcol = itvecDCB->begin(); itcol != itvecDCB->end(); itcol++) {
                            if(intLC == 0) {
                                strThisName = *itcol;
                            } else if (intLC == 1) {
                                strThisIP = *itcol;
                            }
                            intLC++;
                        }
                        //std::cout << "strThisIP = " << strThisIP << " strThisName = " << strThisName << std::endl;
                        strQuery3 = "select count(*) from imports where study_uids='[\"" + strSIUID + "\"]'";
                        mconnect3=mysql_init(NULL);
                        mysql_options(mconnect3,MYSQL_OPT_RECONNECT,"1");
                        mysql_options(mconnect3,MYSQL_OPT_READ_TIMEOUT,"10");
                        //mconnect3=mysql_real_connect(mconnect3, "radisphere-core-db.candescenthealth.com", "app_readonly", "1Wg.O;xS3jZ;6ul6yc*<", strThisName.c_str(), 3306,NULL,0);
                        mconnect3=mysql_real_connect(mconnect3, "rad-core-prod-cluster.cluster-ro-cziyzud3nvxx.us-east-1.rds.amazonaws.com", "app_readonly", "1Wg.O;xS3jZ;6ul6yc*<", strThisName.c_str(), 3306,NULL,0);
                        if (!mconnect3) {
                            //std::cout << "connection failed for DB " << strThisName << " on " << "radisphere-core-db.candescenthealth.com.  Sleeping for 5 seconds before trying again." << std::endl;
                            std::cout << "connection failed for DB " << strThisName << " on " << "rad-core-prod-cluster.cluster-ro-cziyzud3nvxx.us-east-1.rds.amazonaws.com.  Exiting..." << std::endl;
                            std::cerr << mysql_error(mconnect3) << std::endl;
                            mysql_close(mconnect3);
                            return 1;
                        }
                        if(mysql_query(mconnect3, strQuery3.c_str())) {
                            std::cerr << mysql_error(mconnect3) << std::endl;
                            mysql_close(mconnect3);
                            return 1;
                        }
                        MYSQL_RES *result3 = mysql_store_result(mconnect3);
                        if(result3 == NULL) {
                            std::cerr << mysql_error(mconnect3) << std::endl;
                            mysql_close(mconnect3);
                            return 1;
                        }
                        MYSQL_ROW row3;
                        if((row3 = mysql_fetch_row(result3)) != NULL) {
                            intResults = atoi(row3[0]);
                            mysql_free_result(result3);
                            if(intResults > 0) {
                                std::cout << "Found the study in " << strThisName << " DB." << std::endl;
                                strQuery3 = "select source from imports where study_uids='[\"" + strSIUID + "\"]' order by date_created limit 50";
                                mysql_query(mconnect3, strQuery3.c_str());
                                result3 = mysql_store_result(mconnect3);
                                //MYSQL_ROW row3;
                                while((row3 = mysql_fetch_row(result3))) {
                                    strPath = row3[0];
                                    std::cout << "row3 = " << strPath << std::endl;
                                    intPos = strPath.find_last_of("/");
                                    if(intPos != std::string::npos) {
                                        strFilename = strPath.substr(intPos + 1);
                                    } else {
                                        strFilename = strPath;
                                    }
                                    //See if we have pulled this file before
                                    strQuery4 = "select count(*) from dcb where caseid = '" + strCaseID + "' and source = '" + strFilename + "';";
                                    mysql_query(mconnect2, strQuery4.c_str());
                                    if(*mysql_error(mconnect2)) {
                                        strLogMessage="SQL Error: ";
                                        strLogMessage+=mysql_error(mconnect2);
                                        strLogMessage+="strQuery = " + strQuery4 + ".";
                                        std::cout << strLogMessage << std::endl;
                                    }
                                    result4 = mysql_store_result(mconnect2);
                                    if(result4) {
                                        intNumRows = mysql_num_rows(result4);
                                        if(intNumRows > 0) {
                                            row4 = mysql_fetch_row(result4);
                                            strReturn=row4[0];
                                            //std::cout << "strReturn: " << strReturn << std::endl;
                                            mysql_free_result(result4);
                                            sstream.clear();
                                            sstream.str(strReturn);
                                            sstream >> intReturn;
                                        }
                                        if(intReturn > 0) {
                                            std::cout << "Have pulled " << strFilename << " for case ID " << strCaseID << " before.  Skipping..." << std::endl;
                                        } else {
                                            strCmd = "scp " + strThisIP + ":" + strPath + " /tmp/";
                                            std::cout << "Executing: " << strCmd << std::endl;
                                            system(strCmd.c_str());
                                            if(fs::exists("/tmp/" + strFilename)) {
                                                fs::rename("/tmp/" + strFilename, "/home/dicom/inbound2/" + strFilename + ".tar");
                                                strQuery2 = "insert into studies (event_date, source_gateway, clientID, SIUID, DCBserver, AccessionNum, caseid, Complete, dicom_cases_id) ";
                                                strQuery2 += "VALUES(\"" + GetDate() + "\", \"" + strGWIP + "\", \"" + strClientID + "\", \"" + strSIUID + "\", \"";
                                                strQuery2 += strThisName + "\", \"" + strACCN + "\", \"" + strCaseID + "\", 1, " + to_string(intDicomCasesID) + ");";
                                                if(mysql_query(mconnect2, strQuery2.c_str())) {
                                                    std::cerr << mysql_error(mconnect2) << std::endl;
                                                    mysql_close(mconnect2);
                                                    return 1;
                                                }
                                                strQuery2 = "insert into dcb (caseid, source) ";
                                                strQuery2 += "VALUES('" + strCaseID + "', '" + strFilename + "');";
                                                mysql_query(mconnect2, strQuery2.c_str());
                                                if(*mysql_error(mconnect2)) {
                                                    strLogMessage="SQL Error: ";
                                                    strLogMessage+=mysql_error(mconnect2);
                                                    strLogMessage+="strQuery = " + strQuery2 + ".";
                                                    std::cout << strLogMessage << std::endl;
                                                }
                                            }
                                        }
                                    }
                                }
                                mysql_free_result(result3);
                                if(mconnect3) {
                                    mysql_close(mconnect3);
                                }
                                break;
                            }
                        }
                        if(mconnect3) {
                            mysql_close(mconnect3);
                        }
                    }
                } else {
                    std::cout << "Skipping duplicated SIUID: " << strSIUID << " with a duplicate DicomID: " << to_string(intDicomCasesID) << std::endl;
                }
            }
        }
        mysql_free_result(result);
        std::cout << "Ran for " << std::difftime(std::time(nullptr), tmStartTime) << " second(s), sleeping for 10 seconds..." << std::endl;
        std::this_thread::sleep_for (std::chrono::seconds(10));
    }
if (mconnect) {
        mysql_close(mconnect);
    }
    if (mconnect2) {
        mysql_close(mconnect2);
    }
    std::cout << "Exiting prim_aidoc." << std::endl;
    return 0;
}