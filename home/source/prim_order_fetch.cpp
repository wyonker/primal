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
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 

using namespace std;
namespace fs = std::filesystem;
MYSQL *mconnect;

const std::string strProcChainType = "PRIMRCPROC";
std::vector<std::string> vecFileSystem;
std::vector<std::string > vecRCcon1;
std::vector<std::string > vecRCopt1;
std::vector<std::string > vecRCcon2;
std::vector<std::string > vecRCact1;

#include "prim_functions.h"

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

std::size_t fCheckPacs(std::string strSIUID) {
    std::string strReturn, strCmd;
    std::size_t intPos;
    int sock = 0; 
    struct sockaddr_in serv_addr; 

    char buffer[4096] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        std::cout << " Socket creation error " << std::endl; 
        return -1; 
    } 

    strCmd = " -s " + strSIUID;
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(9980);
    if(inet_pton(AF_INET, "10.85.52.200", &serv_addr.sin_addr)<=0) {
        if(inet_pton(AF_INET, "10.85.51.201", &serv_addr.sin_addr)<=0) {
            std::cout << "Invalid address/Address not supported" << std::endl; 
            return -1;
        }
    } 
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        std::cout << "Connection Failed" << std::endl; 
        return -1; 
    } 
    send(sock , strCmd.c_str() , strCmd.length() , 0 ); 
    //valread = read( sock , buffer, 4096); 
    read( sock , buffer, 4096); 
    strReturn += buffer;

    intPos = strReturn.find(strSIUID);
    if(intPos == std::string::npos) {
        std::cout << "SIUID: " << strSIUID << " not found in the PACS.  Pulling..." << std::endl;
        return 1;
    } else {
        std::cout << "SIUID: " << strSIUID << " found in the PACS.  Skipping..." << std::endl;
        return 0;
    }
    //Should never get here...
    return -1; 
}

std::size_t fPullPackage(std::string strStudyID) {
    std::string strQuery, strTemp, strCmd, strThisDate, strFileName, strLogMessage, strDirFullPath2, strSOPIUID;
    std::size_t intDBEntries, intRand;
    std::map<std::string, std::string> mSOPIUID;
    std::vector<std::string> vecPaths;
    std::vector<std::string>::iterator iPaths;
    time_t now = time(0);

    tm *ltm = localtime(&now);
    strThisDate = to_string(1900 + ltm->tm_year);
    strThisDate += to_string(1 + ltm->tm_mon);
    strThisDate += to_string(ltm->tm_mday);
    
    MYSQL *mprimal_index;
    MYSQL_ROW row_Index;
    mprimal_index=mysql_init(NULL);
    mysql_options(mprimal_index,MYSQL_OPT_RECONNECT,"1");
    if (!mprimal_index) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    mprimal_index=mysql_real_connect(mprimal_index, "10.85.55.127", "primal", "ThisGuy1!", "primal_index", 0,NULL,0);
    if (!mprimal_index) {
        cout<<"connection failed to primal_order\n";
        return 1;
    }
    MYSQL_RES *result_Index = mysql_store_result(mprimal_index);

    std::cout << "Pulling packages for StudyID: " << strStudyID << "." << std::endl;
    strQuery = "select dirfullpath, SOPIUID from instance where parentid = '" + strStudyID + "' order by dirfullpath DESC;";
    intRand = rand() % 10000 + 1;
    strFileName = "0_TEMP" + strThisDate + "_" + to_string(intRand) + ".tar.gz";
    mysql_query(mprimal_index, strQuery.c_str());
    result_Index = mysql_store_result(mprimal_index);
    if(result_Index) {
        intDBEntries=mysql_num_rows(result_Index);
        if(intDBEntries > 0) {
            while ((row_Index = mysql_fetch_row(result_Index))) {
                std::pair<std::map<std::string,std::string>::iterator,bool> ret;
                strDirFullPath2=row_Index[0];
                strSOPIUID=row_Index[1];
                ret = mSOPIUID.insert ( std::pair<std::string,std::string>(strSOPIUID,strDirFullPath2) );
                if (ret.second==false) {
                    std::cout << strSOPIUID << " is already on the list.  Skipping..." << std::endl;
                } else {
                    std::cout << "Adding " << strSOPIUID << " to list."  << std::endl;
                    if( std::find(vecPaths.begin(), vecPaths.end(), strDirFullPath2) == vecPaths.end() ) {
                        vecPaths.push_back (strDirFullPath2);
                        std::cout << "Adding file " << strDirFullPath2 << " to pull list..." << std::endl;
                    }
                }
                //mSOPIUID.insert(row_Index[1],row_Index[0]);
            }
            for (iPaths = vecPaths.begin(); iPaths != vecPaths.end(); ++iPaths) {
                strTemp = *iPaths;
                strTemp += "_DICOM.tar.gz";
                strCmd = "aws s3 cp --no-progress s3://" + strTemp + " /tmp/";
                std::cout << "Trying: " << strCmd << std::endl;
                system(strCmd.c_str());
                strTemp = *iPaths;
                strTemp += "package.conf";
                strCmd = "aws s3 cp --no-progress s3://" + strTemp + " /tmp/";
                std::cout << "Trying: " << strCmd << std::endl;
                system(strCmd.c_str());
                strCmd = "(cd /tmp && tar -cf " + strFileName + " _DICOM.tar.gz package.conf)";
                strLogMessage = " ORDER Tar Command: " + strCmd;
                //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFPROC"]);
                system(strCmd.c_str());
                try {
                    fs::copy("/tmp/" + strFileName, "/home/dicom/inbound2/" + strFileName);
                } catch (fs::filesystem_error& err) {
                    std::cout << "ERROR:  Couldn't copy " << strFileName << " to /home/dicom/inbound2/" << strFileName << std::endl;
                    return 1;
                }
                try {
                    fs::remove_all("/tmp/_DICOM.tar.gz");
                    fs::remove_all("/tmp/" + strFileName);
                } catch (fs::filesystem_error& err) {
                    std::cout << "ERROR:  Could not delete /tmp/_DICOM.tar.gz" << std::endl;
                    return 1;
                }
            }
        }
    } else {
        intDBEntries = 0;
    }
    return 0;
}

int main(int argc, char** argv) {
    std::string strQuery, strQuery_PO, strMRN, strDOB, strOrder, strReqDateTime, strDateReceived, strClientID, strDBREturn;
    std::string strStudyDescrition, strACCN, strQuery_Index, strOrderList, strMOD, strStudyID, strSIUID;
    std::size_t intDBEntries, intNumRows, intError, intDone, intInPacs, intReturn;
    std::vector<std::string > vecSIUID;

    if (argc <= 1) {
        std::cout << "ERROR:  Please supply client ID."  << std::endl;
        return 1;
    }
    strClientID = argv[1];
    MYSQL *mconnect2;
    MYSQL_ROW row;
    mconnect2=mysql_init(NULL);
    mysql_options(mconnect2,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect2) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    //mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
    mconnect2=mysql_real_connect(mconnect2, "radisphere-core-db.candescenthealth.com", "app_readonly", "1Wg.O;xS3jZ;6ul6yc*<", "pis", 3306,NULL,0);
    if (!mconnect2) {
        cout<<"connection failed\n";
        return 1;
    }
    MYSQL_RES *result = mysql_store_result(mconnect2);
    MYSQL_RES *result2 = mysql_store_result(mconnect2);

    MYSQL *mprimal_order;
    MYSQL_ROW row_PO;
    mprimal_order=mysql_init(NULL);
    mysql_options(mprimal_order,MYSQL_OPT_RECONNECT,"1");
    if (!mprimal_order) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    mprimal_order=mysql_real_connect(mprimal_order, "localhost", "primal_order", "ThisGuy1!", "primal_order", 0,NULL,0);
    if (!mprimal_order) {
        cout<<"connection failed to primal_order\n";
        return 1;
    }
    MYSQL_RES *result_PO = mysql_store_result(mprimal_order);

    MYSQL *mprimal_index;
    //MYSQL_ROW row_Index;
    mprimal_index=mysql_init(NULL);
    mysql_options(mprimal_index,MYSQL_OPT_RECONNECT,"1");
    if (!mprimal_index) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    mprimal_index=mysql_real_connect(mprimal_index, "10.85.55.127", "primal_order", "ThisGuy1!", "primal_index", 0,NULL,0);
    if (!mprimal_index) {
        cout<<"connection failed to primal_index\n";
        return 1;
    }
    MYSQL_RES *result_Index = mysql_store_result(mprimal_index);

    intDone = 0;
    while(intDone == 0) {
        std::cout << "Checking Connect for new orders..." << std::endl;
        //Query Connect for new orders
        strQuery = "SELECT pid_3_patient_id, pid_7_date_of_birth, obr_3_filler_order_number, obr_6_requested_datetime, ";
        strQuery += "date_received, obr_4_universal_service_identifier, obr_3_filler_order_number FROM hl7_client_orders WHERE clients_id = '" + strClientID + "' AND orc_5_order_status = 'NW' ";
        strQuery += "and date_received >= NOW() - INTERVAL 10 MINUTE order by date_received;";
        mysql_query(mconnect2, strQuery.c_str());
        result = mysql_store_result(mconnect2);
        if(result) {
            while ((row = mysql_fetch_row(result))) {
                intError=0;
                strMRN=row[0];
                strDOB=row[1];
                strOrder=row[2];
                strReqDateTime=row[3];
                strDateReceived=row[4];
                strStudyDescrition=row[5];
                strACCN=row[6];
                std::cout << "Processing MRN: " << strMRN << " DOB: " << strDOB << " Order #: " << strOrder << " Requested Date: " << strReqDateTime << " Received Date: " << strDateReceived << std::endl;
                
                //First check and see if we have seen this order before
                strQuery_PO = "select count(*) from orders where order_number = '" + strOrder + "' and client_id = '" + strClientID + "';";
                mysql_query(mprimal_order, strQuery_PO.c_str());
                if(*mysql_error(mprimal_order)) {
                    std::cout << "SQL Error: " << mysql_error(mprimal_order) << std::endl;
                    std::cout << "strQuery_PO = " << strQuery_PO << "." << std::endl;
                }
                result_PO = mysql_store_result(mprimal_order);
                if(result_PO) {
                    row = mysql_fetch_row(result_PO);
                    strDBREturn=row[0];
                    intDBEntries=stoi(strDBREturn);
                } else {
                    intDBEntries=0;
                }
                mysql_free_result(result_PO);
                if(intDBEntries < 1) {
                    //Let's see if the accession number is processing already.
                    strQuery = "select count(*) from cases where accession_number = '" + strACCN + "';";
                    mysql_query(mconnect2, strQuery.c_str());
                    if(*mysql_error(mconnect2)) {
                        std::cout << "SQL Error: " << mysql_error(mconnect2) << std::endl;
                    }
                    result2 = mysql_store_result(mconnect2);
                    if(result2) {
                        row = mysql_fetch_row(result2);
                        strDBREturn=row[0];
                        intDBEntries=stoi(strDBREturn);
                    } else {
                        intDBEntries=0;
                    }
                    mysql_free_result(result2);
                    if(intDBEntries != 0) {
                        std::cout << "INFO:  Accession: " << strACCN << " has a CaseID.  Skipping..." << std::endl;
                        strQuery_PO = "insert into orders (order_number, order_date, received_date, procss_date, client_id, complete)";
                        strQuery_PO += " values ('" + strOrder + "', '" + strReqDateTime + "', '" + strDateReceived;
                        strQuery_PO += "', '" + GetDate() + "', '" + strClientID + "', '1');";
                        mysql_query(mprimal_order, strQuery_PO.c_str());
                        if(*mysql_error(mprimal_order)) {
                            std::cout << "SQL Error: " << mysql_error(mprimal_order) << std::endl;
                        }
                    } else {
                        //Okay, it's a new order.  Let's see if we have the study description.
                        std::cout << "Order #: " << strOrder << " is new.  Processing..." << std::endl;
                        strQuery_PO = "select modality from study_description where study_description = '" + strStudyDescrition;
                        strQuery_PO += "' and client_id = '" + strClientID + "' limit 1;";
                        //std::cout << "strQuery_PO : " << strQuery_PO << "." << std::endl;
                        mysql_query(mprimal_order, strQuery_PO.c_str());
                        if(*mysql_error(mprimal_order)) {
                            std::cout << "SQL Error: " << mysql_error(mprimal_order) << std::endl;
                        }
                        result_PO = mysql_store_result(mprimal_order);
                        if(result_PO) {
                            intNumRows = mysql_num_rows(result_PO);
                        } else {
                            intNumRows = 0;
                        }
                        if(intNumRows > 0) {
                            std::cout << "Getting modality..." << std::endl;
                            //We have the study description.
                            row_PO = mysql_fetch_row(result_PO);
                            strMOD = row_PO[0];
                            std::cout << "Study description: " << strStudyDescrition << " matches modality: " << strMOD << "."  << std::endl;
                        } else {
                            //Okay, need to get the modality info from Connect
                            std::cout << "We have not seen this study description before.  Need to check with Connect..." << std::endl;
                            strQuery = "SELECT modality FROM case_orders WHERE clients_id = '" + strClientID + "' AND study = '";
                            strQuery += strStudyDescrition + "' group by modality limit 1;";
                            std::cout << "strQuery = " << strQuery << "." << std::endl;
                            mysql_query(mconnect2, strQuery.c_str());
                            if(*mysql_error(mconnect2)) {
                                std::cout << "SQL Error: " << mysql_error(mconnect2) << std::endl;
                            }
                            result2 = mysql_store_result(mconnect2);
                            if(result2) {
                                intNumRows = mysql_num_rows(result2);
                            } else {
                                intNumRows = 0;
                            }
                            if(intNumRows > 0) {
                                std::cout << "Adding study description: " << strStudyDescrition << " to Primal database."  << std::endl;
                                row = mysql_fetch_row(result2);
                                strMOD = row[0];
                                mysql_free_result(result2);
                                strQuery_PO = "insert into study_description (study_description, modality, client_id) values ";
                                strQuery_PO += "('" + strStudyDescrition + "', '" + strMOD + "', '" + strClientID + "');";
                                mysql_query(mprimal_order, strQuery_PO.c_str());
                                if(*mysql_error(mprimal_order)) {
                                    std::cout << "SQL Error: " << mysql_error(mprimal_order) << std::endl;
                                    std::cout << "strQuery_PO = " << strQuery_PO << "." << std::endl;
                                }
                            } else {
                                std::cout << "ERROR:  Could not query Connect for the modality type.  Skipping study...";
                                intError = 1;
                            }
                        }
                        mysql_free_result(result_PO);
                        if(intError == 0) {
                            vecSIUID.clear();
                            strQuery_Index = "select study_id, SIUID from study where pid = '" + strMRN + "' and dob = '" + strDOB;
                            strQuery_Index += "' and clientid = '" + strClientID + "' and StudyModType = '" + strMOD;
                            strQuery_Index += "' order by StudyDate limit 5;";
                            std::cout << "strQuery_Index = " << strQuery_Index << std::endl;
                            mysql_query(mprimal_index, strQuery_Index.c_str());
                            if(*mysql_error(mprimal_index)) {
                                std::cout << "SQL Error: " << mysql_error(mprimal_index) << std::endl;
                                std::cout << "strQuery_Index = " << strQuery_Index << "." << std::endl;
                            }
                            result_Index = mysql_store_result(mprimal_index);
                            if(result_Index) {
                                intNumRows = mysql_num_rows(result_Index);
                                std::cout << "Found " << to_string(intNumRows) << " matching studies in S3." << std::endl;
                                while ((row = mysql_fetch_row(result_Index))) {
                                    strStudyID = row[0];
                                    strSIUID = row[1];
                                    if (std::find(vecSIUID.begin(), vecSIUID.end(), strStudyID) == vecSIUID.end()) {
                                        intInPacs = fCheckPacs(strSIUID);
                                        if(intInPacs == 1) {
                                            intReturn = fPullPackage(strStudyID);
                                            if(intReturn != 0) {
                                                std::cout << "ERROR:  fPullPackage returned a non-zero value." << std::endl;
                                            }
                                        }
                                        vecSIUID.push_back(strStudyID);
                                    } else {
                                        std::cout << "SIUID " << strStudyID << " has been pulled already.  Skipping..." << std::endl;
                                    }
                                }
                            } else {
                                intNumRows = 0;
                                std::cout << "INFO:  There were no matching studies in primal_index for MRN: " << strMRN << "' DOB = '" << strDOB << std::endl;
                                std::cout << "strQuery_Index: " << strQuery_Index << "." << std::endl;
                            }
                            mysql_free_result(result_Index);
                        } else {
                            std::cout << "There was an error.  Not processing study..." << std::endl;
                        }
                        //Let's add this study to primal_orders.
                        std::cout << "Adding order " << strOrder << " to Primal orders database." << std::endl;
                        strQuery_PO = "insert into orders (order_number, order_date, received_date, procss_date, client_id, complete)";
                        strQuery_PO += " values ('" + strOrder + "', '" + strReqDateTime + "', '" + strDateReceived;
                        strQuery_PO += "', '" + GetDate() + "', '" + strClientID + "', '1');";
                        mysql_query(mprimal_order, strQuery_PO.c_str());
                        if(*mysql_error(mprimal_order)) {
                            std::cout << "SQL Error: " << mysql_error(mprimal_order) << std::endl;
                        }
                    }
                } else {
                    std::cout << "Order #: " << strOrder << " has been processed before.  Skipping..." << std::endl;
                }
                //strQuery_Index = "select study_id from study where pid = '" + strMRN + "' and dob = '" + strDOB + "' StudyModType from study";
                std::this_thread::sleep_for (std::chrono::seconds(1));
            }
            mysql_free_result(result);
        }
        std::cout << "Sleeping for 60 seconds before checking for new orders..." << std::endl;
        std::this_thread::sleep_for (std::chrono::seconds(60));
    }
}
