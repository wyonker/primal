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
MYSQL *prim_db;
MYSQL *gwdb;
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

std::size_t fPullFile(std::string strFullPath) {
    std::string strCmd, strFileName, strLocalFileName;
    std::size_t intPos;

    intPos = strFullPath.find_last_of("/");
    if(intPos != std::string::npos) {
        strFileName = strFullPath.substr(intPos + 1);
    } else {
        strFileName = strFullPath;
    }
    strLocalFileName = strFileName + ".tar.gz";
    strCmd = "scp 10.23.37.31:" + strFullPath + " /tmp/" + strLocalFileName;
    system(strCmd.c_str());
    if(fs::exists("/tmp/" + strLocalFileName)) {
        try {
            fs::copy("/tmp/" + strLocalFileName, "/home/dicom/inbound2/" + strLocalFileName);
        } catch (fs::filesystem_error &err) {
            std::cout << "ERROR: " << err.what() << std::endl;
            return 1;
        }
        try {
            fs::remove_all("/tmp/" + strLocalFileName);
        } catch (fs::filesystem_error &err) {
            std::cout << "ERROR: " << err.what() << std::endl;
            return 1;
        }
        return 0;
    } else {
        return 1;
    }
    //Not sure how you would get here...
    return 3;
}

std::size_t fQueryDB() {
    std::string strQuery, strInitialCasesId, strDBREturn, strPackagePath;
    std::size_t intDBEntries, intReturn;

    MYSQL_ROW row1;
    MYSQL_ROW row2;
    MYSQL_RES *result_gwdb = mysql_store_result(gwdb);

    MYSQL_RES *result_prim_db = mysql_store_result(prim_db);

    strQuery = "select dicom_cases.dicom_cases_id, exports.source from dicom_cases left join exports on ";
    strQuery += "dicom_cases.dicom_cases_id = exports.dicom_cases_id where initial_cases_id != 0 and ";
    strQuery += "exports.source like '/opt/rConnect%' and dicom_cases.date_created >= NOW() - INTERVAL 10 MINUTE;";
    mysql_query(gwdb, strQuery.c_str());
    result_gwdb = mysql_store_result(gwdb);
    while ((row1 = mysql_fetch_row(result_gwdb))) {
        strInitialCasesId = row1[0];
        strPackagePath = row1[1];
        std::cout << "Got CaseID: " << strInitialCasesId << std::endl;
        strQuery = "select count(*) from dicom_cases where initial_cases_id = '" + strInitialCasesId + "';";
        mysql_query(prim_db, strQuery.c_str());
        if(*mysql_error(prim_db)) {
            std::cout << "SQL Error: " << mysql_error(prim_db) << std::endl;
        }
        result_prim_db = mysql_store_result(prim_db);
        if(result_prim_db == NULL) {
            intDBEntries=0;
        } else {
            row2 = mysql_fetch_row(result_prim_db);
            strDBREturn=row2[0];
            intDBEntries=stoi(strDBREturn);
        }
        mysql_free_result(result_prim_db);
        if(intDBEntries == 0) {
            //We have not seen this dicom_cases ID before.  Let's pull the study
            intReturn = fPullFile(strPackagePath);
            if(intReturn == 0) {
                std::cout << "Adding " << strInitialCasesId << " to list if seen cases." << std::endl;
                strQuery = "insert into dicom_cases set initial_cases_id = '" + strInitialCasesId + "';";
                mysql_query(prim_db, strQuery.c_str());
                if(*mysql_error(prim_db)) {
                    std::cout << "SQL Error: " << mysql_error(prim_db) << std::endl;
                }
            }
        } else {
            std::cout << "Skipping " << strInitialCasesId << " because we have seen it before." << std::endl;
        }
    }
    mysql_free_result(result_gwdb);
    return 0;
}

int main() {
std::size_t intDone = 0;

    gwdb=mysql_init(NULL);
    mysql_options(gwdb,MYSQL_OPT_RECONNECT,"1");
    if (!gwdb) {
        cout << "MySQL Initilization failed for rConnect DB";
        return 1;
    }
    gwdb=mysql_real_connect(gwdb, "10.23.37.31", "primal", "ThisGuy1!", "rConnect", mainDB.intDBPORT,NULL,0);
    if (!gwdb) {
        cout<<"connection failed to McLaren GW database\n";
        return 1;
    }

    prim_db=mysql_init(NULL);
    mysql_options(prim_db,MYSQL_OPT_RECONNECT,"1");
    if (!prim_db) {
        cout << "MySQL Initilization failed for primalgw DB";
        return 1;
    }
    prim_db=mysql_real_connect(prim_db, "127.0.0.1", "primalgw", "ThisGuy1!", "primal_gw", mainDB.intDBPORT,NULL,0);
    if (!prim_db) {
        cout<<"connection failed to Primal GW database\n";
        return 1;
    }


    while(intDone != 1) {
        fQueryDB();
        std::cout << "Sleeping for 60 seconds" << std::endl;
        std::this_thread::sleep_for (std::chrono::seconds(60));
    }
    return 0;
}