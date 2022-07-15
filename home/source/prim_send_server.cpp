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
//#include "libssh2_config.h"
#include <libssh.h>
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

const std::string strProcChainType = "PRIMRCSEND";

#include "prim_functions.h"

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

std::string fPassThrough(std::string strPrimID, std::string strRecNum) {
    std::string strAET, strQuery, strLogMessage;
    std::map<std::string, std::string>::iterator iprimConf;
    MYSQL *mconnect_local;
    MYSQL_ROW row;
    MYSQL_RES *result;

    mconnect_local=mysql_init(NULL);
    mysql_options(mconnect_local,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect_local) {
        strLogMessage="MySQL Initilization failed in fSendStudy.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return "-1";
    }
    mconnect_local=mysql_real_connect(mconnect_local, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect_local) {
        strLogMessage="MySQL connection failed in fSendStudy";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return "-1";
    }
    //Are we going to impersinate the machine that sent this study?
    if (conf1.primConf[strRecNum + "_PRIPASSTU"] == "1") {
        strQuery="select senderAET from receive where PUID = '" + strPrimID + "' limit 1;";
        mtx.lock();
        mysql_query(mconnect_local, strQuery.c_str());
        result = mysql_store_result(mconnect_local);
        while ((row = mysql_fetch_row(result))) {
            strAET=row[0];
        }
        mysql_free_result(result);
        mtx.unlock();
        strLogMessage="We are passing through the AET of " + strAET;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    } else {
        iprimConf = conf1.primConf.find(strRecNum + "_PRIAET");
        if(iprimConf != conf1.primConf.end()) {
            strAET=conf1.primConf.find(strRecNum + "_PRIAET")->second;
        } else {
            strAET="PRI_DEFAULT";
        }
    }
    mysql_close(mconnect_local);
    return strAET;
}

std::string Get_Date_Time() {
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	std::string strTime;

	strTime=std::to_string(now->tm_year + 1900);
    strTime.append("-");
    strTime+=std::to_string(now->tm_mon + 1);
    strTime.append("-");
    strTime+=std::to_string(now->tm_mday);
    strTime.append(" ");
    strTime+=std::to_string(now->tm_hour);
    strTime.append(":");
    strTime+=std::to_string(now->tm_min);
    strTime.append(":");
    strTime+=std::to_string(now->tm_sec);
    return(strTime);
}

int verify_knownhost(ssh_session session) {
  int state, hlen;
  unsigned char *hash = NULL;
  char *hexa;
  char buf[10];

  state = ssh_is_server_known(session);
  hlen = ssh_get_pubkey_hash(session, &hash);
  if (hlen < 0)
    return -1;

  switch (state) {
    case SSH_SERVER_KNOWN_OK:
      break; /* ok */

    case SSH_SERVER_KNOWN_CHANGED:
      fprintf(stderr, "Host key for server changed: it is now:\n");
      ssh_print_hexa("Public key hash", hash, hlen);
      fprintf(stderr, "For security reasons, connection will be stopped\n");
      free(hash);
      return -1;

    case SSH_SERVER_FOUND_OTHER:
      fprintf(stderr, "The host key for this server was not found but an other"
        "type of key exists.\n");
      fprintf(stderr, "An attacker might change the default server key to"
        "confuse your client into thinking the key does not exist\n");
      free(hash);
      return -1;

    case SSH_SERVER_FILE_NOT_FOUND:
      fprintf(stderr, "Could not find known host file.\n");
      fprintf(stderr, "If you accept the host key here, the file will be"
       "automatically created.\n");
      /* fallback to SSH_SERVER_NOT_KNOWN behavior */

    case SSH_SERVER_NOT_KNOWN:
      hexa = ssh_get_hexa(hash, hlen);
      fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
      fprintf(stderr, "Public key hash: %s\n", hexa);
      free(hexa);
      if (fgets(buf, sizeof(buf), stdin) == NULL) {
        free(hash);
        return -1;
      }
      if (strncasecmp(buf, "yes", 3) != 0) {
        free(hash);
        return -1;
      }
      if (ssh_write_knownhost(session) < 0) {
        fprintf(stderr, "Error %s\n", strerror(errno));
        free(hash);
        return -1;
      }
      break;

    case SSH_SERVER_ERROR:
      fprintf(stderr, "Error %s", ssh_get_error(session));
      free(hash);
      return -1;
  }

  free(hash);
  return 0;
}

/*
static int waitsocket(int socket_fd, LIBSSH2_SESSION *session) {
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;
 
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
 
    FD_ZERO(&fd);
 
    FD_SET(socket_fd, &fd);
 
    // now make sure we wait in the correct direction
    dir = libssh2_session_block_directions(session);

 
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;
 
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;
 
    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
 
    return rc;
}
*/

void fSendStudy(std::string strPrimID, std::string strRecNum, std::size_t intLC, std::string strAET, std::string strPackFileName) {
    std::string strDateTime, strHost, strQuery, strHostName, strFullPath, strTemp, strSendType, strCmd, strReturn, strClientID;
    std::string strLine, strDate, strAEC, strFileName, strOldFileName, strReadLine, strPasswd, strDestHIP, strDestPort, strDestUser;
    std::string strDestPath, strClientAET, strClientName, strLogMessage, strCmd2, strLine2;
    std::size_t intNumFiles = 0, intLC2, intFound, intFound2, intPos, intImgRec, nread, intLC3, intError=0, intAETMapped=0;
    int sock, rc, rc2, auth_pw = 1, intPort, verbosity = SSH_LOG_PROTOCOL;
    std::map<std::string, std::string>::iterator iprimConf;
    char cHostName[1024], mem[1024], *ptr;
    ssh_scp scp;
    MYSQL *mconnect_local;
    MYSQL_ROW row;

    //const char *fingerprint;
    struct sockaddr_in sin;
    struct stat fileinfo;
    FILE *local;
    
    mconnect_local=mysql_init(NULL);
    mysql_options(mconnect_local,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect_local) {
        strLogMessage=strPrimID + " SEND  MySQL Initilization failed in fSendStudy.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return;
    }
    mconnect_local=mysql_real_connect(mconnect_local, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    intLC3=0;
    while (!mconnect_local && intLC3 < 60) {
        strLogMessage=strPrimID + " SEND  MySQL connection failed in fSendStudy";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        std::this_thread::sleep_for (std::chrono::seconds(1));
        intLC3++;
    }
    if(intLC3 >= 60) {
        return;
    }
    strDateTime = Get_Date_Time();
    gethostname(cHostName, 1024);
    strHostName = cHostName;
    strHost = conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)];
    strLogMessage=strPrimID + " SEND  hostname " + strHostName + " destination " + strHost + ".";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    //Count the number of files in the directory
    iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
    if(iprimConf != conf1.primConf.end()) {
        strFullPath = conf1.primConf.find(strRecNum + "_PRIOUT")->second;
        strFullPath.append("/" + strPrimID + "/");
    } else {
        //I don't even know what to do here.
        strReturn+= "ERROR:  Couldn't figure out what the outbound directory is...  Exiting";
    }

    if(!fs::exists(strFullPath)) {
        strLogMessage = " SEND " + strPrimID + " WARn:  Directory" + strFullPath + " does not exist.  Skipping...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        mysql_close(mconnect_local);
        return;
    }
    auto iFullPath = std::filesystem::directory_iterator(strFullPath);
    for (auto& entry : iFullPath) {
        if (entry.is_regular_file()) {
            ++intNumFiles;
        }
    }

    if(strHost.find("0.0.0.0") != std::string::npos) {
        //We use 0.0.0.0 to be a dummy destination
        strQuery="insert into send (puid, sservername, tdest, tstartsend, tendsend, timages, serror, complete) values ('";
        strQuery.append(strPrimID + "', '" + strHostName + "', '" + to_string(intLC) + "', '" + strDateTime + "', '" + strDateTime);
        strQuery.append("','" + to_string(intNumFiles) + "', 0, 1);");
        mysql_query(mconnect_local, strQuery.c_str());
        if(*mysql_error(mconnect_local)) {
            strLogMessage="SQL Error: ";
            strLogMessage+=mysql_error(mconnect_local);
            strLogMessage+="\nstrQuery = " + strQuery;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        }
        strLogMessage=strPrimID + " SEND  Skipping this send because dummy host " + strHost + " found.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    } else {
        //We really need to send to this destination.
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTTYPE" + to_string(intLC));
        if(iprimConf != conf1.primConf.end()) {
            strTemp = conf1.primConf[strRecNum + "_PRIDESTTYPE" + to_string(intLC)];
        } else {
            strReturn += "ERROR:  Could not determine destination type.  Using default:  DICOM\n";
            strTemp = "DICOM";
        }
        for (intLC2=0; intLC2<strTemp.length(); intLC2++) {
            strSendType += std::toupper(strTemp[intLC2]);
        }
        strLogMessage=strPrimID + " SEND  Send type is " + strSendType;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        if (strSendType == "SCP") {
            strLogMessage=strPrimID + " SEND  Starting SCP.";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            strPackFileName="none";
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPACK" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                if(conf1.primConf[strRecNum + "_PRIDESTPACK" + to_string(intLC)] == "tar.gz") {
                    strTemp=conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/";
                    for (const auto & entry : fs::directory_iterator(strTemp)) {
                        strFileName=entry.path().string();
                        intPos=strFileName.find(".tar");
                        if(intPos != std::string::npos) {
                            intPos=strFileName.find_last_of("/");
                            if(intPos != std::string::npos) {
                                strPackFileName=strFileName.substr(intPos + 1);
                            } else {
                                strPackFileName=strFileName;
                            }
                            intFound = 1;
                            intNumFiles=1;
                        }
                    }
                }
            }
            if(intFound != 1) {
                strLogMessage=" SEND " + strPrimID + " WARN:  SCP selected but no .tar found.  This is not currently handeled.  Skipping...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                return;
            }
            if(strPackFileName == "none") {
                iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTCDCR" + to_string(intLC));
                if(iprimConf != conf1.primConf.end()) {
                    if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "2") {
                        strPackFileName="*.ucr";
                    } else if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "1") {
                        strPackFileName="*.j2k";
                    } else {
                        strPackFileName="*.dcm";
                    }
                } else {
                    strPackFileName="*.dcm";
                }
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strDestHIP=conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)];
            } else {
                strReturn += "ERROR:  Could not determine the hostname or IP for this destination.  Exiting...\n";
                intError=1;
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPORT" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strDestPort=conf1.primConf[strRecNum + "_PRIDESTPORT" + to_string(intLC)];
            } else {
                strReturn += "ERROR:  Could not determine the port for this destination.  Using 22\n";
                strDestPort="22";
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTUSER" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strDestUser=conf1.primConf[strRecNum + "_PRIDESTUSER" + to_string(intLC)];
            } else {
                strReturn += "ERROR:  Could not determine the hostname or IP for this destination.  Exiting...\n";
                intError=1;
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPASS" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strPasswd=conf1.primConf[strRecNum + "_PRIDESTPASS" + to_string(intLC)];
            } else {
                strReturn += "ERROR:  Could not determine the password for this destination.  Exiting...\n";
                intError=1;
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPATH" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strDestPath=conf1.primConf[strRecNum + "_PRIDESTPATH" + to_string(intLC)];
            } else {
                strDestPath="";
            }
            strLogMessage=strPrimID + " SEND Start SCP send to " + strDestHIP + " on port " + strDestPort;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            strDestPath+=strPackFileName + ".part";
            //std::cout << "SCP Putting file in " << strDestPath << std::endl;
            strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
            strQuery+=", tdest='SCP" + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "'";
            strQuery+=", tstartsend='" + GetDate() + "', tdestnum=" + to_string(intLC) + ";";
            mysql_query(mconnect_local, strQuery.c_str());
            if(*mysql_error(mconnect_local)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect_local);
                strLogMessage+="\nstrQuery = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            }

            ssh_session my_ssh_session;
            my_ssh_session = ssh_new();
            if(my_ssh_session == NULL) {
                strLogMessage=" SEND " + strPrimID + " libssh initialization failed " + to_string(rc);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
            sin.sin_port = htons(stoi(strDestPort));
            intPort = stoi(strDestPort);
            ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, strDestHIP.c_str());
            ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &intPort);
            ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
            rc = ssh_connect(my_ssh_session);
            if(rc != SSH_OK) {
                strLogMessage=" SEND " + strPrimID + " Connection to host failed: " + to_string(rc);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
            if (verify_knownhost(my_ssh_session) < 0) {
                ssh_disconnect(my_ssh_session);
                ssh_free(my_ssh_session);
                strLogMessage=" SEND " + strPrimID + " SSH host key not known: " + to_string(rc);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
            strFullPath+=strPackFileName;
            strLogMessage=" SEND " + strPrimID + " SCP Sending file " + strFullPath + " to " + strDestHIP + " on " + strDestPort;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            local = fopen(strFullPath.c_str(), "rb");
            if(!local) {
                ssh_disconnect(my_ssh_session);
                ssh_free(my_ssh_session);
                strLogMessage=" SEND " + strPrimID + " SSH Can't open local file ";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
            rc = ssh_userauth_password(my_ssh_session, strDestUser.c_str(), strPasswd.c_str());
            if (rc != SSH_AUTH_SUCCESS) {
                ssh_disconnect(my_ssh_session);
                ssh_free(my_ssh_session);
                strLogMessage=" SEND " + strPrimID + " SSH Failed to authenticate:  Bad username or password ";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
            scp = ssh_scp_new (my_ssh_session, SSH_SCP_WRITE, strDestPath.c_str());
            if (scp == NULL) {
                ssh_disconnect(my_ssh_session);
                ssh_free(my_ssh_session);
                strLogMessage=" SEND " + strPrimID + " Error allocating scp session " +  ssh_get_error(my_ssh_session);
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                intError=1;
            }
            stat(strFullPath.c_str(), &fileinfo);
            do {
                nread = fread(mem, 1, sizeof(mem), local);
                if(nread <= 0) {
                    //std::cout << "Reached end of file for " << strFullPath << std::endl;
                    // end of file
                    break;
                }
                ptr = mem;

                do {
                    // write the same data over and over, until error or completion
                    rc2 = ssh_scp_push_file(scp, ptr, nread, S_IRUSR |  S_IWUSR);

                    if(rc2 < 0) {
                        fprintf(stderr, "ERROR %d\n", rc2);
                        break;
                    } else {
                        // rc indicates how many bytes were written this time
                        ptr += rc2;
                        nread -= rc2;
                    }
                } while(nread);
            } while(1);
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            if(intError == 0) {
                iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPATH" + to_string(intLC));
                if(iprimConf != conf1.primConf.end()) {
                    strCmd="mv " + conf1.primConf[strRecNum + "_PRIDESTPATH"] + "/" + strDestPath + " " + conf1.primConf[strRecNum + "_PRIDESTPATH"] + "/";
                } else {
                    strCmd="mv /pis/radconnect/extract/incoming/" + strDestPath + " /pis/radconnect/extract/incoming/";
                }
                intPos = strDestPath.find(".part");
                strDestPath.erase(intPos);
                strCmd+=strDestPath;
                strLogMessage = strPrimID + " " + strProcChainType + " Executing " + strCmd;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                if(rc != 0) {
                    strLogMessage=" SEND " + strPrimID + " Error??????";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                    intError=1;
                }
            }
            close(sock);
            if(local)
                fclose(local);
            //fprintf(stderr, "all done\n");
            strLogMessage=strPrimID + " SEND Finish SCP send to " + strDestHIP + " on port " + strDestPort;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            strDate = GetDate();
            strQuery="update send set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles) + ", serror=";
            strQuery+=to_string(intError) + " where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC) + ";";
            mysql_query(mconnect_local, strQuery.c_str());
            if(*mysql_error(mconnect_local)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect_local);
                strLogMessage+="\nstrQuery = " + strQuery;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            }
            strLogMessage=strPrimID + " SEND  Finished SCP.";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        } else if (strSendType == "S3") {
            strPackFileName="none";
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPACK" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                if(conf1.primConf[strRecNum + "_PRIDESTPACK" + to_string(intLC)] == "tar.gz") {
                    strPackFileName="*.tar";
                }
            }
            if(strPackFileName == "none") {
                iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTCDCR" + to_string(intLC));
                if(iprimConf != conf1.primConf.end()) {
                    if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "2") {
                        strPackFileName="*.ucr";
                    } else if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "1") {
                        strPackFileName="*.j2k";
                    } else {
                        strPackFileName="*.dcm";
                    }
                } else {
                    strPackFileName="*.dcm";
                }
            }
            //std::cout << "Building S3 send command." << std::endl;
            strCmd="aws s3 cp ";
            iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRIOUT")->second);
            } else {
                strReturn += "ERROR:  Could not determine the outbound directory.  Exiting..\n";
                intError=1;
            }
            strCmd+="/" + strPrimID + "/" + strPackFileName + " \"s3://";
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)]);
            } else {
                strReturn += "ERROR:  Could not determine the hostname or IP for this destination.  Exiting...\n";
                intError=1;
            }
            strCmd+="/";
            time_t t = time(0);
            struct tm * now = localtime( & t );
            strDate = std::to_string(now->tm_year + 1900);
            if ((now->tm_mon + 1) < 10) {
                strDate += "0";
                strDate += to_string(now->tm_mon + 1);
            } else {
                strDate += to_string(now->tm_mon + 1);
            }
            if ((now->tm_mday) < 10) {
                strDate += "0";
                strDate += to_string(now->tm_mday);
            } else {
                strDate += to_string(now->tm_mday);
            }
            strCmd+=strDate + "/" + strPrimID + "/" + strPackFileName + "\" --no-progress";
            strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
            strQuery+=", tdest='S3:" + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "'";
            strQuery+=", tstartsend='" + GetDate() + "', tdestnum=" + to_string(intLC) + ";";
            mysql_query(mconnect_local, strQuery.c_str());
            if(intError == 0) {
                strLogMessage=" SEND " + strPrimID + " Executing " + strCmd + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                system(strCmd.c_str());
            } else {
                strReturn += "There was an error.  Exiting...\n";
            }
            strCmd2="aws s3 ls " + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "/" + strDate ;
            strCmd2+="/" + strPrimID + "/|wc -l";
            //redi::ipstream proc(strCmd, redi::pstreams::pstderr);
            redi::ipstream proc(strCmd2);
    		while (std::getline(proc, strReadLine)) {
	    		strReturn.append(strReadLine);
		    }
            intNumFiles=stoi(strReturn);
            if(intNumFiles < 1) {
                intError=1;
                strLogMessage = strPrimID + " " + strProcChainType + " s3 failed with command " + strCmd;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            }
            strDate=GetDate();
            strQuery="update send set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles) + ", serror=";
            strQuery+=to_string(intError) + " where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC) + ";";
            mysql_query(mconnect_local, strQuery.c_str());
        } else {
            //This should be DICOM...
            strSendType == "DICOM";
            std::cout << "Building dicom send command." << std::endl;
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTCDCR" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "2") {
                    strPackFileName="*.ucr";
                } else if(conf1.primConf[strRecNum + "_PRIDESTCDCR" + to_string(intLC)] == "1") {
                    strPackFileName="*.j2k";
                } else {
                    strPackFileName="*.dcm";
                }
            } else {
                strPackFileName="*.dcm";
            }
            strCmd = "ls -l " + conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/*" + strPackFileName + "|wc -l";
            strReturn = exec(strCmd.c_str());
            if(stoi(strReturn) < 1) {
                std::cout << "strCmd = " << strCmd << "." << std::endl;
                std::cout << "strReturn = " << strReturn << "." << std::endl;
                strLogMessage = strPrimID + " SEND WARN:  There are no " + strPackFileName + " files to send.  Skipping...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                std::cout << strLogMessage << std::endl;
                return;
            }
            //strCmd = "storescu -ll ";
            strCmd = "dcmsend -ll ";
            iprimConf = conf1.primConf.find(strRecNum + "_PRILL");
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRILL")->second);
            } else {
                strCmd.append("debug");
            }
            //strCmd.append(" -xf /home/dicom/bin/storescu.cfg Default");
            strCmd.append(" -dn -nh +sd -nuc ");
            //Hack for IV
            strQuery="select senderAET from receive where puid='" + strPrimID + "';";
            mysql_query(mconnect_local, strQuery.c_str());
            MYSQL_RES *result = mysql_store_result(mconnect_local);
            row = mysql_fetch_row(result);
            strAEC=row[0];
            if(strAEC == "" || strAEC == " ") {
                strAEC=strAET;
            }
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTAEMAP" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strLogMessage = strPrimID + " SEND Searching map table for client ID of " + strAEC + "...";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                std::cout << strLogMessage << std::endl;
                if(conf1.primConf[strRecNum + "_PRIDESTAEMAP" + to_string(intLC)] == "1") {
                    std::ifstream fpAEMap("/var/spool/primal_ae_map.txt");
                    while (std::getline(fpAEMap, strLine)) {
                        intFound=strLine.find(",");
                        intFound2=strLine.find(",", intFound + 1);
                        strClientID = strLine.substr(0, intFound);
                        strClientAET = strLine.substr(intFound + 1, intFound2 - intFound - 1);
                        strClientName = strLine.substr(intFound2 + 1);
                        intFound=strAEC.find(strClientID);

                        //std::cout << "ClientID = " << strClientID << " strClientAET = " << strClientAET  << " strClientName = " << strClientName << std::endl;
                        intFound2=strAEC.find(strClientID);
                        if(intFound2 != std::string::npos) {
                            strAEC = strClientAET;
                            std::cout << "Found " << strClientID << " destiantion AEC is now " << strAEC << std::endl;
                            strCmd.append(" -aet " + strAEC + " -aec " + strAET + " ");
                            intAETMapped=1;
                        }
                    }
                    fpAEMap.close();
                    if(intAETMapped != 1) {
                        strCmd.append(" -aet " + strAEC + " -aec ");
                    }
                } else if(conf1.primConf[strRecNum + "_PRIDESTAEMAP" + to_string(intLC)] == "2") {
                    strLogMessage = strPrimID + " SEND Checking for package.conf file.";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                    std::cout << strLogMessage << std::endl;
                    if(fs::exists(conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/package.conf")) {
                        fstream fpPackage;
                        fpPackage.open(conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/package.conf", ios::in);
                        if(fpPackage.is_open()) {
                            while(getline(fpPackage, strLine)) {
                                intPos = strLine.find("CALLED_AE");
                                if(intPos != std::string::npos) {
                                    strLine2=strLine;
                                    intPos = strLine2.find_last_of(" ");
                                    if(intPos != std::string::npos) {
                                        strAEC = strLine2.substr(intPos +1);
                                        strLogMessage = strPrimID + "SEND Found " + strLine2 + " destiantion AEC is now " + strAEC;
                                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                                        std::cout << strLogMessage << std::endl;
                                        strCmd.append(" -aec " + strAEC + " -aet " + strAET + " ");
                                        intAETMapped=1;
                                    }
                                    break;
                                }
                            }
                        }
                        fpPackage.close();
                    } else {
                        strLogMessage = strPrimID + " SEND Could not find " + conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/package.conf.  Using APEX_1101.";
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                        std::cout << strLogMessage << std::endl;
                        strCmd.append(" -aec APEX_1101 -aet " + strAET + " ");
                    }
                }
            } else {
                strCmd.append(" -aet " + strAET + " -aec ");
            }
            if (intAETMapped != 1) {
                iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTAEC" + to_string(intLC));
                if(iprimConf != conf1.primConf.end()) {
                    strCmd.append(conf1.primConf[strRecNum + "_PRIDESTAEC" + to_string(intLC)]);
                } else {
                    strCmd.append(" DEFAULT ");
                }
            }
            strCmd += " +crf " + conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/" + strPrimID + ".info ";
            //strCmd.append(" +rn ");
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC))->second);
            } else {
                strReturn += "ERROR:  Could not determine the destination host or IP.  Exiting...\n";
                intError=1;
            }
            strCmd.append(" ");
            iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTPORT" + to_string(intLC));
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRIDESTPORT" + to_string(intLC))->second);
            } else {
                strCmd.append("104");
            }
            strCmd.append(" ");
            iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRIOUT")->second);
            } else {
                strReturn += "ERROR:  Could not determine the outbound directory.  Exiting..\n";
                intError=1;
            }
            strCmd.append("/");
            strCmd.append(strPrimID + "/" + strPackFileName + " >> ");
            iprimConf = conf1.primConf.find(strRecNum + "_PRILOGDIR");
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRILOGDIR")->second);
            } else {
                strReturn += "ERROR:  Could not determine the log directory.  Using /var/log/primal/\n";
                strCmd.append("/var/log/primal");
            }
            strCmd.append("/");
            iprimConf = conf1.primConf.find(strRecNum + "_PRILFOUT");
            if(iprimConf != conf1.primConf.end()) {
                strCmd.append(conf1.primConf.find(strRecNum + "_PRILFOUT")->second);
            } else {
                strReturn += "ERROR:  Could not determine the log file.  Using out.log\n";
                strCmd.append("out.log");
            }
            strCmd.append(" 2>&1");
            strDate=GetDate();
            strQuery="insert into send set puid='" + strPrimID + "', sservername='" + strHostName + "'";
            strQuery+=", tdest='DCM:" + conf1.primConf[strRecNum + "_PRIDESTHIP" + to_string(intLC)] + "'";
            strQuery+=", tstartsend='" + strDate + "', tdestnum=" + to_string(intLC) + ";";
            mysql_query(mconnect_local, strQuery.c_str());
            std::cout << "strCmd = " << strCmd << std::endl;
            if(intError == 0) {
                std::cout << "Executing " << strCmd << "." << std::endl;
                intPos=system(strCmd.c_str());
                //std::cout << "Return = " << intPos << std::endl;
            } else {
                std::cout << "There was an error.  Exiting..." << std::endl;
            }
            strQuery="select rec_images from receive where puid='" + strPrimID + "';";
            mysql_query(mconnect_local, strQuery.c_str());
            result = mysql_store_result(mconnect_local);
            row = mysql_fetch_row(result);
            strReturn=row[0];
            intImgRec = stoi(strReturn);
            //Count the number of good sends
            intNumFiles=0;
            intError=0;
            strTemp=conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimID + "/" + strPrimID + ".info";
            /*
            for (const auto & entry : fs::directory_iterator(strTemp)) {
                strFileName=entry.path().string();
                intPos=strFileName.find(".done");
                if(intPos != std::string::npos) {
                    intNumFiles++;
                    strOldFileName=strFileName;
                    strFileName.erase(intPos);
                    fs::rename(strOldFileName, strFileName);
                }
                intPos=strFileName.find(".bad");
                if(intPos != std::string::npos) {
                    intError=1;
                    strOldFileName=strFileName;
                    strFileName.erase(intPos);
                    fs::rename(strOldFileName, strFileName);
                }
            }
            */
            //std::cout << "Trying to read " << strTemp << std::endl;
            std::ifstream fpSendInfo(strTemp);
            while (std::getline(fpSendInfo, strLine)) {
                intPos=strLine.find("with status SUCCESS");
                if(intPos != std::string::npos) {
                    //std::cout << "Found status line" << std::endl;
                    intPos = strLine.find_last_of(" ");
                    if(intPos != std::string::npos) {
                        strTemp = strLine.substr(intPos + 1);
                        intNumFiles=stoi(strTemp);
                        //std::cout << "Num files: " << intNumFiles << std::endl;
                    } else {
                        intNumFiles=0;
                    }
                } else {
                    intNumFiles=0;
                }
            }
            fpSendInfo.close();
            strDate = GetDate();
            if(intNumFiles < intImgRec) {
                intError=1;
            }
            strQuery="update send set tendsend='" + strDate + "', complete='1', timages= " + to_string(intNumFiles) + ", serror=";
            strQuery+=to_string(intError) + " where puid='" + strPrimID + "' and tdestnum=" + to_string(intLC) + ";";
            mysql_query(mconnect_local, strQuery.c_str());
        }
    }
    strLogMessage=strPrimID + " SEND  Finished send...";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    mysql_close(mconnect_local);
    return;
}

std::size_t fUpdateLocation(std::string strPrimalID, std::string strPriOut, std::string strRecNum) {
    std::string strFullPath, strQuery, strFileName, strResult, strDCMdump, strSopIUID, serSerIUID, strLogMessage, strFullFilePath;
    std::size_t intPos, intResult, intNumRows, intIsFound = 0;
    MYSQL *mconnect_local;
    MYSQL_ROW row;

    mconnect_local=mysql_init(NULL);
    mysql_options(mconnect_local,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect_local) {
        strLogMessage = "SEND MySQL initilization failed in fUpdateLocation function.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    mconnect_local=mysql_real_connect(mconnect_local, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect_local) {
        strLogMessage = "SEND MySQL connection failed in fUpdateLocation function.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    MYSQL_RES *result = mysql_store_result(mconnect_local);
    strLogMessage=strPrimalID + " SEND  Updating location of images in DB...";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    strFullPath=conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimalID;
    if(fs::exists(strFullPath)) {
        for (const auto & entry : fs::directory_iterator(strFullPath)) {
            strFullFilePath=entry.path().string();
            intPos=strFullFilePath.find_last_of("/");
            if(intPos != std::string::npos) {
                strFileName=strFullFilePath.substr(intPos + 1);
            } else {
                strFileName=strFullFilePath;
            }
            intPos=strFileName.find(".dcm");
            if(intPos != std::string::npos) {
                strQuery = "select count(*) from image where puid='" + strPrimalID + "' and ifilename='" + strFileName + "';";
                mtx.lock();
                mysql_query(mconnect_local, strQuery.c_str());
                if(*mysql_error(mconnect_local)) {
                    strLogMessage="SQL Error: ";
                    strLogMessage+=mysql_error(mconnect_local);
                    strLogMessage+="\nstrQuery = " + strQuery;
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                }
                result = mysql_store_result(mconnect_local);
                if(result) {
                    intNumRows = mysql_num_rows(result);
                    if(intNumRows > 0) {
                        row = mysql_fetch_row(result);
                        strResult=row[0];
                        intResult=stoi(strResult);
                        if (intResult > 0) {
                            strQuery = "update image set ilocation='" + strPriOut + "/" + strPrimalID + "' where puid = '";
                            strQuery += strPrimalID + "' and ifilename='" + strFileName + "';";
                            mysql_query(mconnect_local, strQuery.c_str());
                            intIsFound = 1;
                        } else {
                            intIsFound = 0;
                        }
                    }
                    mysql_free_result(result);
                }
                mtx.unlock();
                if(intIsFound == 0) {
                    strLogMessage=strPrimalID + " SEND  Adding image location for " + strFileName + ".";
                    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                    strDCMdump=fDcmDump(strFullFilePath);
                    strSopIUID=fGetTagValue("0008,0018", strDCMdump, 0, 0);
                    serSerIUID=fGetTagValue("0020,000e", strDCMdump, 0, 0);
                    GetDate();
                    strQuery = "insert into image set ilocation='" + strPriOut + "/" + strPrimalID + "', puid='" + strPrimalID;
                    strQuery += "', SOPIUID='" + strSopIUID + "', SERIUID='" + serSerIUID + "', iservername='" + strHostname;
                    strQuery += "', ifilename='" + strFileName + "', idate='" + GetDate() + "';";
                    mysql_query(mconnect_local, strQuery.c_str());
                    if(*mysql_error(mconnect_local)) {
                        strLogMessage="SQL Error: ";
                        strLogMessage+=mysql_error(mconnect_local);
                        strLogMessage+="\nstrQuery = " + strQuery;
                        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                    }
                }
            }
        }
    }
    strLogMessage=strPrimalID + " SEND  DB update complete.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    mysql_close(mconnect_local);
    return 0;
}

std::size_t fEndSend(std::string strPrimalID) {
    std::string strQuery, strRecNum, strTemp, strSource, strDest, strLogMessage;
    std::size_t intPos, intNumSent, intDirExists, intNumDest = 0;
    std::map<std::string, std::string>::iterator iprimConf;
    MYSQL *mconnect_local;
    MYSQL_ROW row;
    struct stat st;

    mconnect_local=mysql_init(NULL);
    mysql_options(mconnect_local,MYSQL_OPT_RECONNECT,"1");
    if (!mconnect_local) {
        strLogMessage = "SEND MySQL initilization failed in fEndSend function.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    mconnect_local=mysql_real_connect(mconnect_local, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect_local) {
        strLogMessage = "SEND MySQL connection failed in fEndSend function.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intNumDest));
    while(iprimConf != conf1.primConf.end()) {
        intNumDest++;
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intNumDest));
    }
    strSource = conf1.primConf[strRecNum + "_PRIOUT"] + "/" + strPrimalID;
    intDirExists = stat(strSource.c_str(),&st);
    if(intDirExists == 0 && intNumDest != 0) {
        strQuery="select count(*) from send where puid = '" + strPrimalID + "' and complete = 1 and serror = 0";
        mysql_query(mconnect_local, strQuery.c_str());
        MYSQL_RES *result = mysql_store_result(mconnect_local);
        row = mysql_fetch_row(result);
        strTemp=row[0];
        mysql_free_result(result);
        intNumSent = stoi(strTemp);
        if (intNumSent >= intNumDest) {
            strDest = conf1.primConf[strRecNum + "_PRISENT"] + "/" + strPrimalID;
            //fs::rename(strSource, strDest);
            strLogMessage = strPrimalID + " SEND " + "Copying from " + strSource + " to " + strDest + ".";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            fs::copy(strSource, strDest);
            fs::remove_all(strSource);
        } else {
            strDest = conf1.primConf[strRecNum + "_PRIERROR"] + "/" + strPrimalID;
            //fs::rename(strSource, strDest);
            fs::copy(strSource, strDest);
            fs::remove_all(strSource);
        }
    } else if(intDirExists == 0) {
        strDest = conf1.primConf[strRecNum + "_PRIERROR"] + "/" + strPrimalID;
            //fs::rename(strSource, strDest);
            fs::copy(strSource, strDest);
            fs::remove_all(strSource);
    } else {
        strLogMessage = strPrimalID + " SEND " + "Directory does not exist " + strSource + " for " + strPrimalID;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        mysql_close(mconnect_local);
        return 1;
    }
    intDirExists = stat(strDest.c_str(),&st);
    if(intDirExists == 0) {
        strLogMessage = strPrimalID + " SEND " + "Moved " + strPrimalID + " to " + strDest;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        strQuery = "update image set ilocation = '" + strDest + "' where puid = '" + strPrimalID + "';";
        mysql_query(mconnect_local, strQuery.c_str());
    } else {
        mysql_close(mconnect_local);
        return 2;
    }
    mysql_close(mconnect_local);
    return 0;
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
    conf1.primConf.erase(conf1.primConf.begin(), conf1.primConf.end());
    conf1.ReadConfFile();
    return;
}

std::size_t fLoopSend(std::string strFullPath) {
    std::string strLogMessage, strPrimalID, strRecNum, strAET, strPackFileName;
    std::size_t intReturn, intPos, intLC;
    std::map<std::string, std::string>::iterator iprimConf;

    mysql_thread_init();
    MYSQL *mconnect;
    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect) {
        strLogMessage="MySQL Initilization failed in fLoopSend.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="MySQL connection failed in fLoopSend.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return 1;
    }
    //Parse message
    //intPos=strFullPath.find_last_of(" ");
    //intMsgType=stoi(strFullPath.substr(intPos));
    //strFullPath.erase(intPos);
    if(strFullPath.back() == '/') {
        strFullPath.pop_back();
    }
    intPos = strFullPath.find_last_of("/");
    if(intPos != std::string::npos) {
        strPrimalID=strFullPath.substr(intPos + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);

    intLC=0;
    //First get the receiver number for config searches
    strAET = fPassThrough(strPrimalID, strRecNum);
    strAET.erase(std::remove(strAET.begin(), strAET.end(), '"'), strAET.end());
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
    if(iprimConf == conf1.primConf.end()) {
        strLogMessage=" SEND " + strPrimalID + "  " + strRecNum + "_PRIDESTHIP" + to_string(intLC) + ".  Not found...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    }
    strPackFileName="none";
    iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
    if(iprimConf != conf1.primConf.end()) {
        strLogMessage=strPrimalID + " SEND " + " Starting send for destination # " + to_string(intLC) + " primalID " + strPrimalID;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        fUpdateLocation(strPrimalID, conf1.primConf[strRecNum + "_PRIOUT"], strRecNum);
        fSendStudy(strPrimalID, strRecNum, intLC, strAET, strPackFileName);
        intLC++;
        iprimConf = conf1.primConf.find(strRecNum + "_PRIDESTHIP" + to_string(intLC));
    } else {
        strLogMessage=strPrimalID + " SEND  ERROR:  Could not find send destation.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    }
    intReturn = fEndSend(strPrimalID);
    if(intReturn == 2) {
        strLogMessage =strPrimalID + " SEND " + " ERROR:  Tried to move " + strPrimalID + " but failed";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    } else if (intReturn == 1) {
        strLogMessage =strPrimalID + " SEND " + " ERROR:  Failed to move " + strPrimalID;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
    }
    mysql_close(mconnect);
    mysql_thread_end();
    return intReturn;
}

std::string fPollSendDir(std::string strRecNum) {
    std::size_t intTemp, intRecNum, intMaxThreads, intReturn, intReturn2;
    std::string strTemp2, strCMD, strCmd, strLogMessage, strFilename, strPrimalID, strRawDCMdump, strPName, strMRN;
    std::string strDOB, strSerIUID, strSerDesc, strModality, strSopIUID, strSIUID, strStudyDate, strACCN, strStudyDesc;
    std::string strPatientComments, strTemp, strQuery, strDBReturn, strStartRec, strResult, strStudyTime, strStudyDateTime;
    std::string strFullPath, strFileExtension, strDirName, strWorkingDirectory, strRecNum2, strReturn;
    std::map<std::string, std::string>::iterator iprimConf;
    DIR *dp;
    struct dirent *dirp;
    struct stat attrib;
    struct FIs {
        std::string FullPath;
        std::size_t intTimeStamp;

        FIs(std::string FullPath, std::size_t intTimeStamp)
            : FullPath(std::move(FullPath))
            , intTimeStamp(intTimeStamp)
        {}
    };
    std::vector<FIs> vecSendList;
    std::vector<FIs>::iterator itSendList;
    std::stringstream sstream("1");

    MYSQL *mconnect;
    //MYSQL_ROW row;
    //MYSQL_RES *result;

    mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
    //mysqlpp::Connection conn(false);
    if (!mconnect) {
        strLogMessage="MySQL Initilization failed in fPollSendDir.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return "-1";
    }
    mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        strLogMessage="MySQL connection failed in fPollSendDir";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return "-1";
    }

    if(!fs::exists(conf1.primConf[strRecNum + "_PRIOUT"])) {
        strLogMessage = " SEND WARN:  Directory" + conf1.primConf[strRecNum + "_PRIOUT"] + " does not exist.  Skipping...";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return "-1";
    }
    iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
    if(iprimConf != conf1.primConf.end()) {
        strWorkingDirectory = conf1.primConf[strRecNum + "_PRIOUT"];
    } else {
        strLogMessage = " SEND ERROR:  Could not find outbound directory for recver # " + strRecNum;
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
        return "-1";
    }

    while(true) {
        intRecNum=1;
        strRecNum = to_string(intRecNum);
        iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
        while(iprimConf != conf1.primConf.end()) {
            strRecNum=to_string(intRecNum);
            strWorkingDirectory = conf1.primConf[strRecNum + "_PRIOUT"];
            if((dp  = opendir(strWorkingDirectory.c_str())) == NULL) {
                strLogMessage = " SEND ERROR:  (" + to_string(errno) + ") opening " + strWorkingDirectory;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                return "-1";
            }
            while ((dirp = readdir(dp)) != NULL) {
                stat(dirp->d_name, &attrib);
                strTemp = string(dirp->d_name);
                if(strTemp != "." && strTemp != "..") {
                    if((attrib.st_mode & S_IFMT) == S_IFDIR) {
                        if(strWorkingDirectory.back() == '/') {
                            strTemp = strWorkingDirectory + string(dirp->d_name);
                        } else {
                            strTemp = strWorkingDirectory + "/" + string(dirp->d_name);
                        }
                        auto p = std::find_if(vecSendList.begin(), vecSendList.end(), [strTemp] (const FIs& a ) { return a.FullPath == strTemp ; } );
                        if(p == vecSendList.end()) {
                            vecSendList.emplace_back(strTemp, attrib.st_ctime);
                        }
                        //files.push_back(string(dirp->d_name));
                    }
                }
            }
            closedir(dp);
            intRecNum++;
            strRecNum=to_string(intRecNum);
            iprimConf = conf1.primConf.find(strRecNum + "_PRIOUT");
        }
        for (itSendList = vecSendList.begin(); itSendList != vecSendList.end(); ) {
            intTemp = itSendList->FullPath.find_last_of("/");
            strDirName=itSendList->FullPath.substr(intTemp+1);
            intTemp = strDirName.find_first_of("_");
            strRecNum=strDirName.substr(0,intTemp);
/*
            strQuery="select tstartsend from send where puid = \"" + strDirName + "\" and serror=0;";
            mysql_query(mconnect, strQuery.c_str());
            if(*mysql_error(mconnect)) {
                strLogMessage="SQL Error: ";
                strLogMessage+=mysql_error(mconnect);
                strLogMessage+="strQuery = " + strQuery + ".";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            }
            result = mysql_store_result(mconnect);
            if(result) {
                intNumRows = mysql_num_rows(result);
                if(intNumRows < 1) {
*/
            intMaxThreads = 5;
            iprimConf = conf1.primConf.find(strRecNum + "_PRIMAXSEND");
            if(iprimConf != conf1.primConf.end()) {
                sstream.clear();
                sstream.str(conf1.primConf[strRecNum + "_PRIMAXSEND"]);
                sstream >> intMaxThreads;
            }
            //std::cout << "strRecNum = " << strRecNum << "  Max send in conf file: " << conf1.primConf[strRecNum + "_PRIMAXSEND"] << std::endl;
            strCMD = "ps -ef|grep \"prim_send_worker\"|grep -v grep|wc -l";
            strReturn = exec(strCMD.c_str());
            sstream.clear();
            sstream.str(strReturn);
            sstream >> intReturn;
            strCMD = "ps -ef|grep \"prim_send_worker " + itSendList->FullPath + "\"|grep -v grep|wc -l";
            strReturn = exec(strCMD.c_str());
            sstream.clear();
            sstream.str(strReturn);
            sstream >> intReturn2;
            if(intReturn2 > 0) {
                vecSendList.erase(itSendList);
            } else if(intReturn < intMaxThreads) {
                //mysql_free_result(result);
                //strLogMessage=strPrimalID + " SEND Got " + itSendList->FullPath + " message.  Processing...";
                //fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                strLogMessage = strPrimalID + "SEND Launching worker for " + itSendList->FullPath;
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
                strCMD = "/usr/local/bin/prim_send_worker " + itSendList->FullPath + " &";
                system(strCMD.c_str());
                std::this_thread::sleep_for (std::chrono::seconds(1));
                vecSendList.erase(itSendList);
            }
            if(vecSendList.size() < 1) {
                break;
            }
        }
        std::this_thread::sleep_for (std::chrono::seconds(3));
    }
    //Should never get here.
    return "-2";
}

int main() {
    //std::string strFullPath, strPrimalID, strQuery, strRow, strTemp, strJson, strSerIUID, strSerTemp, strTemp2, strFilename, strOldVal;
    //std::size_t intDone=0, intImgNum=0, intTemp;
    //std::map<std::string, std::string> mapSeriesJson;
    //MYSQL_ROW row;
    std::size_t intMaxThreads;
    std::string strRecNum, strAET, strFileExt, strHostName, strDateTime, strFullPath, strCmd, strSendType, strPrimalID, strReturn;
    std::string strLogMessage, strCMD;
    std::map<std::string, std::string>::iterator iprimConf;

    std::signal(SIGHUP, signal_handler);

    mysql_library_init(0, NULL, NULL);
    ReadDBConfFile();
    conf1.ReadConfFile();
    strRecNum = "1";
    intMaxThreads = 5;
    iprimConf = conf1.primConf.find(strRecNum + "_PRIMAXSEND");
    if(iprimConf != conf1.primConf.end()) {
        std::stringstream sstream(conf1.primConf[strRecNum + "_PRIMAXSEND"]);
        sstream >> intMaxThreads;
    }
    strLogMessage = "Starting prim_send_server version 1.02.10 with " + to_string(intMaxThreads) + " max sends.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);

    while(true) {
        //strCMD = "ps -ef|grep \"prim_send_worker\"|grep -v grep|wc -l";
        //strReturn = exec(strCMD.c_str());
        //std::stringstream sstream(strReturn);
        //sstream >> intReturn;
        //if(intReturn < intMaxThreads) {
            //strFullPath = fGetMessage("/prim_send");
            strFullPath = fPollSendDir(strRecNum);
            /*
            strLogMessage=strPrimalID + " SEND Got " + strFullPath + " message.  Processing...";
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            std::cout << strLogMessage << std::endl;
            strLogMessage = "SEND Launching worker for " + strFullPath;
            fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + "_PRILFOUT"]);
            std::cout << strLogMessage << std::endl;
            strCMD = "/usr/local/bin/prim_send_worker " + strFullPath + " &";
            system(strCMD.c_str());
            */
        //}
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    mysql_library_end();
    return 0;
}