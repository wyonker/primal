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

const std::string strVersionNum = "4.00.00";
const std::string strVersionDate = "2025-01-14";

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

int fProcess() {
    std::string strQuery, strQuery2, strQuery3, strLogMessage, strPUID, strID, strPservername, strTstartproc, strRecID, strConf_proc_id, strConf_rec_id, strProc_name, strProc_type;
    std::string strProc_tag, strProc_operator, strProc_cond, strProc_action, strProc_order, strProc_dest, strProc_active;
    int intProc_type, intNumRows, intConf_proc_id, intReturn;

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

int main() {
    std::string strRecNum, strAET, strFileExt, strHostName, strDateTime, strFullPath, strCmd, strSendType, strPrimalID, strReturn;
    std::string strLogMessage, strCMD, strID, strPUID, strServerName, strDestNum, strDest, strOrg, strStartSend, strEndSend, strImages, strError, strRetry, strComplete;
    std::string strQuery, strQuery2, strQuery3, strQuery4, strLocation, strSendPort, strSendHIP, strSendAEC, strSendAET, strStatus;
    std::string strSendOrder, strSendPass, strSendRetry, strSendCompression, strSendTimeOut, strSendOrg, strSendName, strRecId;
    std::string strSendActive, strSendUser;

    std::size_t intNumRows;

    std::signal(SIGHUP, signal_handler);

    mysql_library_init(0, NULL, NULL);
    ReadDBConfFile();

    MYSQL_ROW row;
    MYSQL_ROW row2;
    MYSQL_ROW row3;

    MYSQL_RES *result;
    MYSQL_RES *result2;
    MYSQL_RES *result3;

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

    strRecNum = "1";
    strLogMessage = "Starting prim_server version " + strVersionNum + ".";
    fWriteLog(strLogMessage, "/var/log/primal/primal.log");

    while (1) {
        strQuery = "SELECT * FROM semd WHERE complete = 3;";
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
            if(intNumRows < 1) {
                while ((row = mysql_fetch_row(result))) {
                    strID = row[0];
                    strPUID = row[1];
                    strServerName = row[2];
                    strDestNum = row[3];
                    strDest = row[4];
                    strOrg = row[5];
                    strStartSend = row[6];
                    strEndSend = row[7];
                    strImages = row[8];
                    strError = row[9];
                    strRetry = row[10];
                    strComplete = row[11];
                }
                mysql_free_result(result);
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
                    if(intNumRows < 1) {
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
                strQuery3 = "SELECT DISTINCT ilocaiton FROM image WHERE puid = '" + strPUID + "';";
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
                    if(intNumRows < 1) {
                        while ((row3 = mysql_fetch_row(result3))) {
                            strLocation = row3[0];
                        }
                        //Now we have all the info we need to send.  Let's build the command.  We need to do this for each ilocation.
                        strCMD = "echo \"dcmsend -ll debug -aet " + strSendAET + " -aec " + strSendAEC + " " + strSendHIP + " " + strSendPort + " " + strLocation + "/*.dcm\" >> /var/log/primal/primal.log 2>&1";
                        strStatus = exec(strCMD.c_str());
                        strQuery4 = "UPDATE send SET complete = 1 WHERE id = '" + strID + "';";
                        mysql_query(mconnect, strQuery4.c_str());
                        if(*mysql_error(mconnect)) {
                            strLogMessage="SQL Error: ";
                            strLogMessage+=mysql_error(mconnect);
                            strLogMessage+="strQuery3 = " + strQuery4 + ".";
                            fWriteLog(strLogMessage, "/var/log/primal/primal.log");
                        }
                    }
                }
                std::this_thread::sleep_for (std::chrono::seconds(10));
            }
        }
    }

    mysql_library_end();
    return 0;
}