#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <map>
#include <algorithm>
#include <vector>
#include <functional>
#include <unistd.h>
#include <sys/stat.h>
#include <my_global.h>
#include <mysql.h>

using namespace std;
MYSQL *mconnect;

class ConfFile
    {
        public:
        //This should be an unordered_map but Eclipse won't let me...
        //std::unordered_map<std::string,std::string> primConf;
        std::map<std::string,std::string> primConf;
        std::string strRec;
        std::vector <std::string> strSCP;
		std::vector <std::string> strPort;

        int ReadConfFile();
        int ValidateConf(void);
    } conf1;

    int ConfFile::ReadConfFile()
    {
        std::string strLine;
        std::size_t intPOS, intEndPOS, intLC1;
        std::ifstream infile("/etc/primal/primal.conf", std::ifstream::in);

		intLC1=0;
        while(!infile.eof())
        {
            getline(infile,strLine); // Saves the line in STRING.
			intPOS=strLine.find("#");
			if(intPOS!=std::string::npos) {
				//Need to remove # and everything after it
				strLine=strLine.substr(0,intPOS);
			}
			intPOS=strLine.find("<scp");
			intEndPOS=strLine.find(">");
			if(intPOS!=std::string::npos && intEndPOS!=std::string::npos) {
				strSCP.resize(intLC1+1);
				strPort.resize(intLC1+1);
				strSCP[intLC1]=strLine.substr((intPOS+4), (intEndPOS-(intPOS+4)));
				intLC1++;
			}
			intPOS=strLine.find("PRIPORT=");
			if(intPOS!=std::string::npos) {
				//Found the port I should be echoing.  Need to save that
				strPort[(intLC1-1)]=strLine.substr((intPOS+8));
				intEndPOS=strPort[(intLC1-1)].find(" ");
				if(intEndPOS!=std::string::npos) {
					strPort[(intLC1-1)]=strPort[(intLC1-1)].substr(0,intEndPOS);
				}
				intEndPOS=strPort[(intLC1-1)].find("\t");
				if(intEndPOS!=std::string::npos) {
					strPort[(intLC1-1)]=strPort[(intLC1-1)].substr(0,intEndPOS);
				}
				//cout << "Port " << strSCP[(intLC1-1)] << " = " << strPort[(intLC1-1)] << endl;
			}
        }
        infile.close();

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

struct DataBase {
    std::string DBTYPE, DBNAME, DBUSER, DBPASS, DBHOST;
    int intDBPORT;
} mainDB;


int ReadDBConfFile()
{
    std::string strLine, strKey, strValue;
    std::size_t intPOS, intStartPOS, intStartConf=0, intEndConf=0;
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
            } else if (strKey.compare("DBPORT") == 0) {
                mainDB.intDBPORT=atoi(strValue.c_str());
            }
        }
    }
    infile.close();

    return 0;
};

std::string Get_Date_Time() {
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	std::string strTime;

	strTime+=std::to_string(now->tm_year + 1900);
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

int main()
{
	std::size_t intPOS, intNumRows, intColSCP, intColStatus, intLC1=0;
	std::string strReturn, strQuery, strCommand, strDateTime, strStatus;
	FILE *fp;
	char buffer[256];
	std::ostringstream oss, oss2, oss3;

	mconnect=mysql_init(NULL);
    mysql_options(mconnect,MYSQL_OPT_RECONNECT,"1");
	if (!mconnect) {
        cout << "MySQL Initilization failed";
        return 1;
    }
    //mconnect=mysql_real_connect(mconnect, "localhost", "primal", "primal", "primal", 0,NULL,0);
	mconnect=mysql_real_connect(mconnect, mainDB.DBHOST.c_str(), mainDB.DBUSER.c_str(), mainDB.DBPASS.c_str(), mainDB.DBNAME.c_str(), mainDB.intDBPORT,NULL,0);
    if (!mconnect) {
        cout<<"connection failed\n";
        return 1;
    }
	conf1.ReadConfFile();
	while(intLC1 < conf1.strPort.size()) {
		strCommand="/home/dicom/bin/echoscu -to 10 -ta 10 -td 10 -ll debug localhost " + conf1.strPort[intLC1] + " 2>&1";
		dup2(1, 2);
		fp = popen(strCommand.c_str(), "r");
		strReturn.clear();
		while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
			strReturn.append(buffer);
			strReturn.append("\n");
		}
		//cout << strReturn << endl;
		intPOS=strReturn.find("I: Received Echo Response (Status: Success)");
		//oss << "select * from monitor where SCP = '" << conf1.strPort[intLC1] << "';";
		//strQuery=oss.str();
		strQuery="select * from monitor where SCP = '" + conf1.strSCP[intLC1] + "';";
		cout << strQuery << endl;
		mysql_ping(mconnect);
		if (mysql_query(mconnect, strQuery.c_str())) {
			fprintf(stderr, "%s\n", mysql_error(mconnect));
			mysql_close(mconnect);
			exit(1);
		}

		MYSQL_RES *result = mysql_store_result(mconnect);
		//The MySQL C API doesn't have a key->value map of the field names.  This is a workaround
		MYSQL_ROW row;
		MYSQL_FIELD *field;
		std::size_t num_fields = mysql_num_fields(result);
		char *headers[num_fields];
		for(std::size_t i = 0; (field = mysql_fetch_field(result)); i++) {
			headers[i] = field->name;
			if (strcmp("SCP", headers[i]) == 0) {
				intColSCP = i;
			}
			if (strcmp("status", headers[i]) == 0) {
				intColStatus = i;
			}
		}
		//Now we are sure which column to read and write (in case someone reorders the database)
		intNumRows=mysql_num_rows(result);
		strDateTime = Get_Date_Time();
		if(intPOS!=std::string::npos) {
			strStatus = "1";
			cout << "storescp on port " << conf1.strPort[intLC1] << " is up!" << endl;
		} else {
			strStatus = "0";
			cout << "storescp on port " << conf1.strPort[intLC1] << " is down!" << endl;
		}
		if(intNumRows > 1) {
            strQuery = "delete from monitor where SCP = '" + conf1.strPort[intLC1] + "';";
			cout << strQuery << endl;
            mysql_query(mconnect, strQuery.c_str());
		}
		if(intNumRows < 1 || intNumRows > 1) {
			strQuery = "insert into monitor (SCP, status, begin_state) values ('" + conf1.strSCP[intLC1] + "', '" + strStatus + "', '" + strDateTime + "');";
		} else if(intNumRows == 1) {
			while ((row = mysql_fetch_row(result))) {
				if(strcmp(row[intColStatus], strStatus.c_str()) == 0) {
					strQuery = "update monitor set status = '" + strStatus + "' where SCP = '" + conf1.strSCP[intLC1] + "';";
				} else {
					strQuery = "update monitor set status = '" + strStatus + "', begin_state = '" + strDateTime + "' where SCP = '" + conf1.strSCP[intLC1] + "';";
				}
			}
		}
		cout << strQuery << endl;
		mysql_query(mconnect, strQuery.c_str());
		intLC1++;
		pclose(fp);
		mysql_free_result(result);
	}
	mysql_close(mconnect);
	return 0;
}
