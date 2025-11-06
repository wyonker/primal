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

namespace fs = std::filesystem;
std::string strLogFile = "/var/log/primal/primal_age.log";

int main () {
    std::string strQuery, strLogMessage;

    MYSQL *mconnect;
    MYSQL_ROW row;
    MYSQL_RES *result;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect) {
        strLogMessage="SEND  MySQL Initilization failed.";
        fWriteLog(strLogMessage, strLogFile);
        return;
    }

    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        if(intLC <= 5) {
            strLogMessage="SEND  MySQL connection failed.  'Trying again...";
            fWriteLog(strLogMessage, strLogFile);
        } else {
            strLogMessage="SEND  MySQL connection failed.  'Out of retries!";
            fWriteLog(strLogMessage, strLogFile);
            return;
        }
        std::this_thread::sleep_for (std::chrono::seconds(3));
        mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    }

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
                // Now we have a sent dir, lets clean up files older than X days

                fs::path pathSentDir(strSentDir);
                if (fs::exists(pathSentDir) && fs::is_directory(pathSentDir))
                {
                    auto now = std::chrono::system_clock::now();
                    for (const auto& entry : fs::directory_iterator(pathSentDir))
                    {
                        if (fs::is_regular_file(entry.status()))
                        {
                            auto ftime = fs::last_write_time(entry);
                            auto age = now - ftime;
                            auto age_in_days = std::chrono::duration_cast<std::chrono::hours>(age).count() / 24;
                            if (age_in_days > std::stoi(strRetPeriod))
                            {
                                fs::remove(entry.path());
                                strLogMessage = "Deleted file: " + entry.path().string() + " from sent directory as it is older than " + strRetPeriod + " days.";
                                fWriteLog(strLogMessage, strLogFile);
                            }
                        }
                    }
                }
            }
        }
        mysql_free_result(result);
    }

}
