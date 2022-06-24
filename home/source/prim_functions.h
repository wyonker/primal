
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
//Version 3
//2020-06-25

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

std::string fMakePUID() {
    time_t t = time(0);
    struct tm * now = localtime( & t );
    std::string strDate;
    struct timespec spec;
    long ms;

    clock_gettime(CLOCK_REALTIME, &spec);
    ms = round(spec.tv_nsec / 1.0e6);
    if(ms > 999) {
        ms = 0;
    }

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
    strDate += "_";
    if (now->tm_hour < 10) {
        strDate += "0";
        strDate += to_string(now->tm_hour);
    } else {
        strDate += to_string(now->tm_hour);
    }
    if (now->tm_min < 10) {
        strDate += "0";
        strDate += to_string(now->tm_min);
    } else {
        strDate += to_string(now->tm_min);
    }
    if (now->tm_sec < 10) {
        strDate += "0";
        strDate += to_string(now->tm_sec);
    } else {
        strDate += to_string(now->tm_sec);
    }
    strDate += to_string(ms);
    return strDate;
}

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

std::string strHostname;

class ConfFile
    {
        public:
        //This should be an unordered_map but Eclipse won't let me...
        //std::unordered_map<std::string,std::string> primConf;
        std::map<std::string,std::string> primConf;
        std::string strRec;

        int ReadConfFile();
        int ValidateConf(void);
        int PriError(std::string strErrCode, const char* FileName);
        int PriLog(std::string strErrCode);
        int StartReceive();
        int EndReceive(std::string strCaller);
        int WriteStudy(std::string strSIUID, std::string strStudyDesc);
        int WriteSeries(std::string strSERIUID, std::string strSIUID, std::string strSeriesDesc);
        int WriteImage(std::string strSOPIUID, std::string strSERIUID, std::string ifilename);
    } conf1;

    int ConfFile::ReadConfFile() {
        std::string strLine, strLine2, strKey, strValue, strTemp, strTemp1, strTemp2, strTemp3, strTemp4;
        std::size_t intPOS, intPOS2, intPOS3, intPOS4, intLength, intDone, intLC, intStartConf=0, intFound;
        std::ifstream infile2("/etc/primal/primal.conf", std::ifstream::in);
        std::map<std::string, std::string>::iterator iprimConf;

        //std::cout << "Reading configuraiton file..." << std::endl;
        std::ifstream infile("/etc/primal/primal.conf", std::ifstream::in);
        while(!infile.eof()) {
            getline(infile,strLine); // Saves the line in STRING.
            intFound=0;
            if(intStartConf == 0) {
                intPOS=strLine.find("<scp");
                intPOS2=strLine.find(">");
                if(intPOS != std::string::npos && intPOS2 != std::string::npos) {
                    intLength = intPOS2 - intPOS - 4;
                    strRec=strLine.substr(intPOS+4, intLength);
                    intStartConf=1;
                    //std::cout << "Reading configuration for " << strRec << "."  << std::endl;
                }
            } else {
                intPOS=strLine.find("</scp"+strRec);
                if(intPOS!=std::string::npos)
                {
                    intStartConf=0;
                } else {
                    strLine2=strLine;
                    intPOS=strLine.find("#");
                    if(intPOS!=std::string::npos)
                        strLine=strLine.substr(0, intPOS);
                    intPOS=strLine.find("=");
                    if(intPOS!=std::string::npos)
                    {
                        strKey = strLine.substr(0,intPOS);
                        while(strKey.substr(0,1) == " " || strKey.substr(0,1) == "\t") {
                            strKey.erase (0,1);
                        }
                        strKey=strRec + "_" + strKey;
                        intPOS2 = strKey.find("PRIQRTAG");
                        if(intPOS2 != std::string::npos) {
                            intLC=0;
                            intDone=0;
                            while(intDone == 0) {
                                strTemp = strKey + ":" + to_string(intLC);
                                iprimConf = conf1.primConf.find(strTemp);
                                if(iprimConf != conf1.primConf.end()) {
                                    intLC++;
                                    if(intLC >= 1000)
                                        intDone = 1;
                                } else {
                                    intDone = 1;
                                }
                            }
                            strKey=strTemp;
                            //std::cout << "Found PRIQRTAG " << strKey << std::endl;
                        }
                        intPOS2 = strKey.find(strProcChainType);
                        if(intPOS2 != std::string::npos) {
                            //std::cout << "Found config rule for " << strProcChainType << std::endl;
                            intPOS3=strLine2.find("=");
                            if(intPOS3 != std::string::npos) {
                                strValue=strLine2.substr(intPOS3+1);
                            }
                            intPOS3 = strValue.find("|:|");
                            if(intPOS3 != std::string::npos) {
                                strTemp1 = strValue.substr(1, intPOS3 - 2);
                                intFound=1;
                            } else {
                                //std::cout << "   Rule incomplete.  Missing first segment." << std::endl;
                                continue;
                            }
                            intPOS4 = strValue.find("|:|", intPOS3+1);
                            if(intPOS4 != std::string::npos) {
                                strTemp2 = strValue.substr(intPOS3+4, intPOS4 - (intPOS3 + 5));
                                intFound++;
                            } else {
                                //std::cout << "   Rule incomplete.  Missing second segment." << std::endl;
                                continue;
                            }
                            intPOS3 = strValue.find("|:|", intPOS4+1);
                            if(intPOS3 != std::string::npos) {
                                strTemp3 = strValue.substr(intPOS4+4, intPOS3 - (intPOS4 + 5));
                                strTemp4 = strValue.substr(intPOS3+4);
                                intFound = intFound+2;
                            } else {
                                //std::cout << "   Rule incomplete.  Missing third and fourth segments." << std::endl;
                                continue;
                            }
                            if(intFound == 4) {
                                strTemp1=strRec + "_" + strTemp1;
                                vecRCcon1.push_back(strTemp1);
                                vecRCopt1.push_back(strTemp2);
                                vecRCcon2.push_back(strTemp3);
                                vecRCact1.push_back(strTemp4);
                                //std::cout << "Found rule:" << strTemp1 << ":" << strTemp2 << ":" << strTemp3 << ":" << strTemp4 << ":" << std::endl;
                            }
                        } else {
                            strValue=strLine.substr(intPOS+1);
                        }
                        //Remove leading spaces and tabs
                        while(strValue.substr(0,1) == " " || strValue.substr(0,1) == "\t") {
                            strValue.erase (0,1);
                        }
                        //Remove trailing spaces or tabs
                        while(strValue.substr(strValue.length() - 1,1) == " " || strValue.substr(strValue.length() - 1,1) == "\t") {
                            strValue.erase (strValue.length() - 1,1);
                        }
                        //std::cout << "Adding key: " << strKey << " value: " << strValue << "." << std::endl;
                        primConf[strKey]=strValue;
                    }
                }
            }
        }
        infile.close();

        //Need the host name
        char hostname[1024];
        hostname[1023] = '\0';
        gethostname(hostname, 1023);
        strHostname=hostname;

        return 0;
    };
    int ConfFile::ValidateConf(void)
    {
        struct stat sb;

        //PRIPORT
        int intValue = atoi(this->primConf["PRIPORT"].c_str());
        if (intValue < 100 || intValue > 10000)
        {
            cout << "Error:  PRIPORT is : " << intValue << " and must be a number and must be between 100 and 10,000." << endl;
            return 1;
        }
        //PRIDESTPORT0
        intValue = atoi(this->primConf["PRIDESTPORT0"].c_str());
        if (intValue < 100 || intValue > 100000)
        {
            cout << "Error:  PRIDESTPORT0 is : " << intValue << " and must be a number and must be between 100 and 100,000." << endl;
            return 1;
        }
        //PRILL
        std::transform(this->primConf["PRILL"].begin(), this->primConf["PRILL"].end(), this->primConf["PRILL"].begin(), ::tolower);
        if (this->primConf["PRILL"] != "fatal" && this->primConf["PRILL"] != "error" && this->primConf["PRILL"] != "warn" && this->primConf["PRILL"] != "info" && this->primConf["PRILL"] != "debug" && this->primConf["PRILL"] != "trace")
        {
            cout << "Error:  PRILL is : " << this->primConf["PRILL"] << " and must be one of the following:  fatal, error, warn, info, debug or trace..." << endl;
            return 1;
        }
        //PRIIF - Location receiver will store incoming files
        if (stat(this->primConf["PRIIF"].c_str(), &sb) == -1)
        {
            cout << "Error:  PRIIF = " << this->primConf["PRIIF"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
            return 1;
        }
        //PRIPROC - Location to put series that are being processed.
        if (stat(this->primConf["PRIPROC"].c_str(), &sb) == -1)
        {
            cout << "Error:  PRIPROC = " << this->primConf["PRIPROC"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
            return 1;
        }
        //PRIOUT - Location to put series that are to be sent
        if (stat(this->primConf["PRIOUT"].c_str(), &sb) == -1)
        {
            cout << "Error:  PRIOUT = " << this->primConf["PRIOUT"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
            return 1;
        }
        //PRISENT - Location to put studies that have been sent successfully
        if (stat(this->primConf["PRISENT"].c_str(), &sb) == -1)
        {
            cout << "Error:  PRISENT = " << this->primConf["PRISENT"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
            return 1;
        }
        //PRILOGDIR - Location to put studies that have been sent successfully
        if (stat(this->primConf["PRILOGDIR"].c_str(), &sb) == -1)
        {
            cout << "Error:  PRILOGDIR = " << this->primConf["PRILOGDIR"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
            return 1;
        }
        //PRIERROR - Location to put studies that have been sent successfully
        if (stat(this->primConf["PRIERROR"].c_str(), &sb) == -1)
        {
            cout << "Error:  PRIERROR = " << this->primConf["PRIERROR"] << " but the directory does not exist or could not be accessed.  Exiting..." << endl;
            return 1;
        }
        //PRIRET - Days to keep series that were successfully sent.  0 = forever
        intValue = atoi(this->primConf["PRIRET"].c_str());
        if (intValue < 0 || intValue > 365)
        {
            cout << "Error:  PRIRET is : " << intValue << " and must be a number between 0 and 365." << endl;
            return 1;
        }
        //PRICL - Days to keep series that were successfully sent.  0 = forever
        intValue = atoi(this->primConf["PRICL"].c_str());
        if (intValue < 0 || intValue > 9)
        {
            cout << "Error:  PRICL is : " << intValue << " and must be a number between 0 and 9." << endl;
            return 1;
        }
        //PRIDESTHIP0
        if (this->primConf["PRIDESTHIP0"].empty())
        {
            cout << "Error:  PRIDESTHIP0 must be defined as the Hostname or IP of the first destination." << endl;
            return 1;
        }
        //Need to add checking for additional HIPx
        return 0;
    }

int ConfFile::PriError(std::string strErrCode, const char* FileName) {
    std::string strTemp = this->primConf["PRILOGDIR"] + "/" + this->primConf["PRILFOUT"];
    std::string strTemp2, strQuery;
    std::vector<string> vecTimeStamps;
    std::ofstream ErrFile, TSfile2;
    std::string strErrDate;
    std::ifstream TSFile;
    std::size_t intTemp, intTemp2, intFound, intLC1;
    std::map<std::string,std::string>::iterator itConf;
    std::ostringstream oss;
    struct stat sb;

    pData.strEndDate=GetDate();
    pData.intIsError=1;

    //Get total number of senders
    intLC1=0;
    strTemp="PRIDESTHIP"+std::to_string(intLC1);
    itConf=this->primConf.find(strTemp);
    while(itConf != primConf.end()) {
        intLC1++;
        strTemp="PRIDESTHIP"+std::to_string(intLC1);
        itConf=this->primConf.find(strTemp);
    }
    std::size_t intNumRec=intLC1;

    //Write that we are done sending to the info file.
    strTemp2 = "/tmp/" + pData.strDirName;
    TSfile2.open(strTemp2, std::ios::app);
    TSfile2 << "Error " << pData.strRec << endl;
    TSfile2.close();

    //Read info file and see how many senders are done.
    intFound=0;
    intLC1=0;
    TSFile.open(strTemp2, std::ios_base::in);
    if (TSFile.is_open()) {
        while (getline(TSFile, strTemp))
            vecTimeStamps.push_back (strTemp);
        TSFile.close();
        while(intLC1 < vecTimeStamps.size()) {
            intTemp=vecTimeStamps[intLC1].find("Done");
            intTemp2=vecTimeStamps[intLC1].find("Error");
            if(intTemp != std::string::npos || intTemp2 != std::string::npos)
                intFound++;
            intLC1++;
        }
    } else {
        cout << "Error:  Could not open /tmp/" << pData.strDirName << " for reading..." << endl;
        return 1;
    }

    //First we need to make sure the directory is still there.  Some sender might have errored and moved it.

    char *chrPath = (char*)pData.strPath.c_str();
    if (stat(chrPath, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        if (intFound == intNumRec) {
            //All senders are finished.  Time to move the directory to the error directory
            strTemp="mv " + pData.strPath + " " + this->primConf["PRIERROR"];
            system(strTemp.c_str());
            //strTemp="/tmp/" + pData.strDirName;
            //strcpy(chrPath, strTemp.c_str());
            //unlink(chrPath);
        } else if (intFound > intNumRec) {
            this->PriLog("Warning:  More senders reported complete than should exist. ");
            strTemp="mv " + pData.strPath + " " + this->primConf["PRIERROR"];
            system(strTemp.c_str());
            //strTemp="/tmp/" + pData.strDirName;
            //strcpy(chrPath, strTemp.c_str());
            //unlink(chrPath);
        }
    } else {
        cout << "Error:  Directory not found to move..." << endl;
    }

    strTemp=pData.strPath + "/" + "error.txt";
    ErrFile.open(strTemp, std::ios_base::app);
    strErrDate=GetDate();
    ErrFile << strErrDate << "   " << strErrCode << FileName << " for Patient: " << pData.strPName << "  MRN: " << pData.strMRN << " Accession#: " << pData.strACCN << " with " << pData.intNumFiles << " images. #Rec" << intNumRec << " : " << pData.strRec << endl;
    oss << "update send set tendsend='" << GetDate() << "', serror='1' where send.puid='" << pData.strDirName << "' and tdest='" << pData.strDest << "';";
    //strQuery=oss.str();
    //mysql_ping(mconnect);
    //mysql_query(mconnect, strQuery.c_str());
    ErrFile.close();
    this->PriLog(strErrCode);

    return 0;
}

int ConfFile::PriLog(std::string strLog) {
    std::string strTemp = this->primConf["PRILOGDIR"] + "/" + this->primConf["PRILFOUT"];
    std::ofstream LogFile;
    std::string strLogDate=GetDate();
    LogFile.open(strTemp, std::ios_base::app);
    LogFile << strLogDate << "   Destination: " << pData.strDest << "  "<< strLog << " for Patient: " << pData.strPName << "  MRN: " << pData.strMRN << " Accession#: " << pData.strACCN << " with " << pData.intNumFiles << " images." << endl;
    LogFile.close();
    return 0;
}

struct DataBase {
    std::string DBTYPE, DBNAME, DBUSER, DBPASS, DBHOST;
    int intDBPORT;
} mainDB;

int ReadDBConfFile()
{
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

void fWriteMessage(std::string strFullPath, std::string strMessageQueue) {
    mqd_t fdWMQueue;
    unsigned int intPrio = 0;
    std::size_t intCount;

    fdWMQueue = mq_open(strMessageQueue.c_str(), O_WRONLY);
    if (fdWMQueue == -1) {
        std::cerr << "ERROR:  Couuld not open " << strMessageQueue << " queue." << std::endl;
        return;
    }
    if (strFullPath.length() < 128) {
        intCount=128-strFullPath.length();
        strFullPath.append(intCount, ' ');
    }
    //std::cout << strFullPath << "." << std::endl;
    mq_send(fdWMQueue, strFullPath.c_str(), 128, intPrio);
    fflush(stdout);
    mq_close(fdWMQueue);
    return;
}

std::string fGetMessage(std::string strMsgQueue) {
    mqd_t fdMQueue;
    char msg_buffer[129];
    struct mq_attr attr;
    ssize_t num_bytes_received = -1;
    msg_buffer[10] = 0;
    std::string strReturn;
    unsigned int prio;

    attr.mq_maxmsg = 512;
    attr.mq_msgsize = 128;
    attr.mq_flags   = 0;

    /*
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 1024;
    attr.mq_curmsgs = 0;
    */

    fdMQueue = mq_open(strMsgQueue.c_str(), O_CREAT | O_RDONLY, 0666, &attr);
    if (fdMQueue == -1) {
        std::cerr << "ERROR:  Couuld not open " << strMsgQueue << " queue." << std::endl;
        exit(0);
    }
    while (num_bytes_received == -1)  {
        memset(msg_buffer, 0x00, sizeof(msg_buffer));
        num_bytes_received = mq_receive(fdMQueue, msg_buffer, 128, &prio);
        if(num_bytes_received == -1) {
            std::this_thread::sleep_for (std::chrono::milliseconds(100));
        }
    }
    //mq_close(fdMQueue);
    //mq_unlink("/prim_process");
    //std::cout << "msg_buffer = " << msg_buffer << std::endl;
    fflush(stdout);
    strReturn = msg_buffer;

    //Remove whitespace from the end
    strReturn.erase(std::find_if(strReturn.rbegin(), strReturn.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), strReturn.end());

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

std::size_t fProcessRule (std::string strFullPath, std::string strCond1, std::string strOpt1, std::string strCond2, std::string strAct1, std::size_t intDefLvl) {
    std::string strCMD, strReturn, strFileExtension, strFilename, strDCMdump, strPathandName, strTagVal;
    std::size_t intTemp, intPos, intReturn;
    std::vector<std::string>::iterator itRCcon1;
    (void) strAct1;
    //(void) intDefLvl;

    //std::cout << "Getting to the fProcessRule fuction..." << std::endl;
    for (const auto & entry : fs::directory_iterator(strFullPath)) {
        strPathandName=entry.path().string();
        intTemp = strPathandName.find_last_of("/");
        strFilename=strPathandName.substr(intTemp+1);
        intPos=strFilename.find_last_of(".");
        if(intPos != std::string::npos) {
            strFileExtension=strFilename.substr(intPos);
        } else {
            strFileExtension="N/A";
        }
        if(strFileExtension == ".dcm") {
            strDCMdump=fDcmDump(strPathandName);
            if(strOpt1 == "run" && strProcChainType == "PRIMRCPROC") {
                //Only process run, sed and mod operators in the processing program.
                strTagVal = fGetTagValue(strCond1, strDCMdump, 1);
                strCMD = "/usr/local/scripts/" + strCond2 + " \"" + strTagVal + "\"" + " \"" + strPathandName + "\"";
                //std::cout << "Run comand = " << strCMD << std::endl;
                strReturn = exec(strCMD.c_str());
                //Remove any carrage returns
                strReturn.erase( std::remove(strReturn.begin(), strReturn.end(), '\r'), strReturn.end() );
                strReturn.erase( std::remove(strReturn.begin(), strReturn.end(), '\n'), strReturn.end() );
                strReturn.erase( std::remove(strReturn.begin(), strReturn.end(), '"'), strReturn.end() );
                //std::cout << "strReturn = " << strReturn << std::endl;
                if(strReturn != "-1") {
                    if(intDefLvl == 0) {
                        std::cout << "Processing rule on the study level..." << std::endl;
                        if(strReturn != "" && strReturn != " " && strReturn.length() < 65) {
                            strCMD = "dcmodify -nb -ie -nrc -i \"" + strCond1 + "=" + strReturn + "\" " + strFullPath + "/*.dcm";
                            //std::cout << "dcmodify command = " << strCMD << std::endl;
                            system(strCMD.c_str());
                        }
                        intReturn = 0;
                        break;
                    } else {
                        //std::cout << "Processing rule on the series level..." << std::endl;
                        if(strReturn != "" && strReturn != " " && strReturn.length() < 65) {
                            strCMD = "dcmodify -nb -ie -nrc -i \"" + strCond1 + "=" + strReturn + "\" " + strPathandName;
                            //std::cout << "dcmodify command = " << strCMD << std::endl;
                            system(strCMD.c_str());
                        }
                    }
                } else {
                    intReturn = 1;
                }
            } else if(strOpt1 == "sed" && strProcChainType == "PRIMRCPROC") {
                //Only process run, sed and mod operators in the processing program.
                strCMD = "sed " + strCond2;
                strReturn = exec(strCMD.c_str());
                if(strReturn != "" && strReturn != " " && strReturn.length() < 65) {
                    strCMD = "dcmodify -nb -ie -nrc -i \"" + strCond1 + " = " + strReturn + "\"";
                    strCMD += " " + strFilename;
                    system(strCMD.c_str());
                }
            } else if(strOpt1 == "mod" && strProcChainType == "PRIMRCPROC") {
                strCMD = "dcmodify -nb -ie -nrc -i \"" + strCond1 + " = " + strCond2 + "\" " + strFilename;
                system(strCMD.c_str());
            }
        }
    }
    return intReturn;
}

std::size_t fRulesChain(std::string strFullPath) {
    std::size_t intLC, intReturn, intTemp, intPos, intRuleNum, intAction, intPrevAction = -1, intDefAct = 0, intDefLvl = 0;
    std::size_t intFound;
    std::string strCMD, strReturn, strTemp2, strTemp3, strFilename, strDCMdump, strTagVal, strPrimalID, strLogMessage, strRecNum;
    std::string strLogFile, strProcName, strTemp, strRCcon1;
    std::vector<std::string>::iterator itRCcon1;
    (void) intRuleNum;

    intFound = strFullPath.find_last_of("/");
    if(intFound != std::string::npos) {
        strPrimalID=strFullPath.substr(intFound + 1);
    } else {
        strPrimalID=strFullPath;
    }
    intPos = strPrimalID.find("_");
    strRecNum=strPrimalID.substr(0,intPos);
    if(strProcChainType == "PRIMRCSTOR") {
        strLogFile = "_PRILFIN";
        strProcName = "STOR";
    } else if(strProcChainType == "PRIMRCRECV") {
        strLogFile = "_PRILFIN";
        strProcName = "RECV";
    } else if(strProcChainType == "PRIMRCPROC") {
        strLogFile = "_PRILFPROC";
        strProcName = "PROC";
    } else if(strProcChainType == "PRIMRCSEND") {
        strLogFile = "_PRILFOUT";
        strProcName = "SEND";
    } else if(strProcChainType == "PRIMRCQR") {
        strLogFile = "_PRILFQR";
        strProcName = "QR";
    }
    strLogMessage = strPrimalID + " " + strProcName + " Started processing rules chain.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + strLogFile]);
    //std::cout << strLogMessage << std::endl;

    intLC=0;
    /* Default action values
     0 - Continue
     1 - Stop
    */
    fs::create_directory(strFullPath + "/ignore");
    for(itRCcon1 = vecRCopt1.begin(); itRCcon1 < vecRCopt1.end(); itRCcon1++) {
        if(*itRCcon1 == "default") {
            if(vecRCcon2[intLC] == "study") {
                intDefLvl = 0;
            } else if(vecRCcon2[intLC] == "series") {
                intDefLvl = 1;
            } else if(vecRCcon2[intLC] == "instance") {
                intDefLvl = 2;
            }
            if(vecRCact1[intLC] == "stop") {
                intDefAct = 1;
            }
        }
        intLC++;
    }
    intLC=0;
    /* Rule return values
      -1 - Mod rule.  No action
       0 - Not matched
       1 - Rule matched
       2 - skip
       3 - stop
       4 - continue
       5 - and matched
       6 - and not matched
       7 - or match
       8 - or not matched
       9 - nor matched
       10 - nor not matched
    */
    intRuleNum=0;
    std::vector<std::string>::iterator itRCopt1;
    for(itRCopt1 = vecRCopt1.begin(); itRCopt1 < vecRCopt1.end(); itRCopt1++) {
        intPos=vecRCcon1[intLC].find("_");
        if(intPos != std::string::npos) {
            strTemp = vecRCcon1[intLC].substr(0,intPos + 1);
            //std::cout << "strTemp = " << strTemp << "." << std::endl;
        } else {
            return 0;
        }
        if(strTemp.compare(strRecNum + "_") == 0) {
            strRCcon1 = vecRCcon1[intLC].substr(intPos + 1);
            if(intPrevAction != 2) {
                if(strRCcon1 != "default") {
                    intAction = fProcessRule (strFullPath, strRCcon1, vecRCopt1[intLC], vecRCcon2[intLC], vecRCact1[intLC], intDefLvl);
                } else {
                    std::cout << "Skipping rule because it's a default" << std::endl;
                }
                if(intDefAct == 0) {
                    if(intAction == 1 && intPrevAction < 5) {
                        intReturn = 3;
                    } else if(intAction == 1 && intPrevAction == 5) {
                        intReturn = 3;
                    } else if(intAction == 1 && intPrevAction == 8) {
                        intReturn = 3;
                    } else if(intAction == 0 && intPrevAction == 7) {
                        intReturn = 3;
                    } else if(intAction == 0 && intPrevAction == 10) {
                        intReturn = 3;
                    } else {
                        intReturn = 4;
                    }
                } else if(intDefAct == 1) {
                    if(intAction == 1 && intPrevAction < 5) {
                        intReturn = 4;
                    } else if(intAction == 4) {
                        intReturn = 4;
                    } else if(intAction == 1 && intPrevAction == 5) {
                        intReturn = 4;
                    } else if(intAction == 1 && intPrevAction == 8) {
                        intReturn = 4;
                    } else if(intAction == 0 && intPrevAction == 9) {
                        intReturn = 4;
                    } else if(intAction == 0 && intPrevAction == 10) {
                        intReturn = 4;
                    } else {
                        intReturn = 3;
                    }
                }
                if(intDefLvl != 0 && intReturn == 3) {
                    fs::rename(strFullPath + "/" + strFilename, strFullPath + "/ignore/" + strFilename);
                } 
            } else {
                intAction = -1;
            }
            intPrevAction = intAction;
            if(intDefAct == 1) {
                fs::rename(strFullPath + "/" + strFilename, strFullPath + "/ignore/" + strFilename);
            }
        }
        intLC++;
    }
    if(intReturn == 4) {
        //Default action is to stop the study and no rules matched.  So return stop.
        strLogMessage = strPrimalID + " " + strProcName + " Continue processing study.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + strLogFile]);
        std::cout << strLogMessage << std::endl;
        return 4;
    } else if(intReturn == 3) {
        //Default action is to continue the study and but had a rule match.  So return stop.
        strLogMessage = strPrimalID + " " + strProcName + " Stop processing study.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + strLogFile]);
        std::cout << strLogMessage << std::endl;
        return 3;
    } else {
        //Need to see if there are any files left to continue with
        for (const auto & entry : fs::directory_iterator(strFullPath)) {
            strTemp2=entry.path().string();
            intTemp = strTemp2.find_last_of("/");
            strFilename=strTemp2.substr(intTemp+1);
            intPos=strFilename.find_last_of(".");
            if(intPos != std::string::npos) {
                strTemp3=strFilename.substr(intPos);
            }
            if(strTemp3 == ".dcm") {
                strLogMessage = strPrimalID + " " + strProcName + " Continue processing study due to rule.";
                fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + strLogFile]);
                std::cout << strLogMessage << std::endl;
                return 4;
            }
        }
        //Default aciton is to continue and no rules matched but there are no DICOM files left.  So return stop.
        strLogMessage = strPrimalID + " " + strProcName + " Stop processing study due to no DICOM files left to send.";
        fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + strLogFile]);
        std::cout << strLogMessage << std::endl;
        return 3;
    }
    //Not sure how you would get here but...
    strLogMessage = strPrimalID + " " + strProcName + " Continue processing study due to faulty rules processing.";
    fWriteLog(strLogMessage, conf1.primConf[strRecNum + "_PRILOGDIR"] + "/" + conf1.primConf[strRecNum + strLogFile]);
    std::cout << strLogMessage << std::endl;
    return 0;
}

