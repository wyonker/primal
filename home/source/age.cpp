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
#include <stdexcept>

using namespace std;
namespace fs = std::filesystem;
std::string strLogFile = "/var/log/primal/primal_age.log";

struct DataBase {
    std::string DBTYPE, DBNAME, DBUSER, DBPASS, DBHOST;
    int intDBPORT;
} mainDB;

std::string GetDate() {
    time_t t = time(0);
    struct tm * now = localtime( & t );
    std::string strDate;

    strDate = std::to_string(now->tm_year + 1900);
    strDate += "-";
    if ((now->tm_mon + 1) < 10) {
        strDate += "0";
        strDate += std::to_string(now->tm_mon + 1);
    } else {
        strDate += std::to_string(now->tm_mon + 1);
    }
    strDate += "-";
    if ((now->tm_mday) < 10) {
        strDate += "0";
        strDate += std::to_string(now->tm_mday);
    } else {
        strDate += std::to_string(now->tm_mday);
    }
    strDate += " ";
    if (now->tm_hour < 10) {
        strDate += "0";
        strDate += std::to_string(now->tm_hour);
    } else {
        strDate += std::to_string(now->tm_hour);
    }
    strDate += ":";
    if (now->tm_min < 10) {
        strDate += "0";
        strDate += std::to_string(now->tm_min);
    } else {
        strDate += std::to_string(now->tm_min);
    }
    strDate += ":";
    if (now->tm_sec < 10) {
        strDate += "0";
        strDate += std::to_string(now->tm_sec);
    } else {
        strDate += std::to_string(now->tm_sec);
    }
    return strDate;
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

bool isIntegerWithStoi(const std::string& s) {
    std::size_t pos;
    try {
        std::stoi(s, &pos);
    } catch (const std::invalid_argument& e) {
        return false; // Not a valid number
    } catch (const std::out_of_range& e) {
        return false; // Number is too large or too small for int
    }
    return pos == s.length(); // Entire string was converted
}

int main () {
    std::string strQuery, strLogMessage, strConfName, strRetPeriod, strSentDir;
    int intNumRows;

    MYSQL *mconnect;
    MYSQL_ROW row;
    MYSQL_RES *result;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage="SEND  MySQL Initilization failed.";
        fWriteLog(strLogMessage, strLogFile);
        return -1;
    }

    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="SEND  MySQL connection failed.  'Out of retries!";
        fWriteLog(strLogMessage, strLogFile);
        return -1;
    }

    //Create log directory if it does not exist
    fs::path pathLogFile(strLogFile);
    if (!fs::exists(pathLogFile.parent_path())) {
        fs::create_directories(pathLogFile.parent_path());
    }

    strLogMessage="Starting age process to clean up sent directories based on retention periods.";
    fWriteLog(strLogMessage, strLogFile);

    strQuery = "SELECT conf_name, ret_period, sent_dir FROM conf_rec WHERE conf_name != '!Global!;";
    mysql_query(mconnect, strQuery.c_str());
    if(*mysql_error(mconnect)) {
        strLogMessage="SQL Error: ";
        strLogMessage+=mysql_error(mconnect);
        strLogMessage+="\nQuery: " + strQuery + "\n";
        fWriteLog(strLogMessage, strLogFile);
    }
    result = mysql_store_result(mconnect);
    if(result) {
        intNumRows=mysql_num_rows(result);
        if(intNumRows > 0) {
            while((row = mysql_fetch_row(result))) {
                strConfName = row[0];
                strRetPeriod = row[1];
                strSentDir = row[2];
                if (!isIntegerWithStoi(strRetPeriod)) {
                    strLogMessage = "Retention period for config " + strConfName + " is not a valid integer: " + strRetPeriod + ". Skipping.";
                    fWriteLog(strLogMessage, strLogFile);
                    continue;
                }
                // Now we have a sent dir, lets clean up files older than X days

                fs::path pathSentDir(strSentDir);
                if (fs::exists(pathSentDir) && fs::is_directory(pathSentDir)) {
                    auto now = std::chrono::system_clock::now();
                    for (const auto& entry : fs::directory_iterator(pathSentDir)){
                        if (fs::is_regular_file(entry.status())) {
                            auto ftime = fs::last_write_time(entry);
                            auto age2 = now;
                            auto age = age2 - ftime;
                            auto age_in_minutes = std::chrono::duration_cast<std::chrono::minutes>(age).count();
                            if (age_in_minutes > std::stoi(strRetPeriod)) {
                                fs::remove(entry.path());
                                strLogMessage = "Deleted file: " + entry.path().string() + " from sent directory as it is older than " + strRetPeriod + " minutes.";
                                fWriteLog(strLogMessage, strLogFile);
                            }
                        }
                    }
                } else {
                    strLogMessage = "Sent directory " + strSentDir + " for config " + strConfName + " does not exist or is not a directory. Skipping.";
                    fWriteLog(strLogMessage, strLogFile);
                }
            }
        }
        mysql_free_result(result);
    }

}
