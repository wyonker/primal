
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
#include <memory>
#include <cstdio>
#include <array>
//#include <pstreams/pstream.h>
//#include <mysql/my_global.h>
//#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <openssl/md5.h>

using namespace std;
namespace fs = std::filesystem;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
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

std::size_t fLaunchChild() {
    std::string strLine, strClientID, strGWIP, strCMD, strReturn;
    std::size_t intPos, intReturn;
    std::ifstream infile("/etc/primal/aidoc.conf", std::ifstream::in);

    if(infile.bad()) {
        std::cerr << "ERROR:  /etc/primal/aidoc.conf does not exists.  Exiting" << std::endl;
        return 1;
    }
    while(!infile.eof()) {
        getline(infile,strLine); // Saves the line in STRING.
        intPos=strLine.find_first_of(" ");
        if(intPos != std::string::npos) {
            strClientID = strLine.substr(0, intPos);
            strGWIP = strLine.substr(intPos+1);
            strCMD = "ps -ef|grep \"prim_aidoc_client " + strClientID + " " + strGWIP + "\"|grep -v grep|wc -l";
            //std::cout << strCMD << std::endl;
            strReturn = exec(strCMD.c_str());
            std::stringstream sstream(strReturn);
            sstream >> intReturn;
            std::cout << "Found " << to_string(intReturn) << " instance for Client ID " << strClientID << "." << std::endl;
            if(intReturn < 1) {
                std::cout << "Launching prim_aidoc_client for client ID " << strClientID << " and IP of " << strGWIP << "." << std::endl;
                strCMD = "/home/primal/prim_aidoc_client " + strClientID + " " + strGWIP + " >> ./" + strClientID + ".log 2>&1 &";
                system(strCMD.c_str());
            }
        } else {
            if(strLine.length() > 1) {
                std::cout << "Unable to process line: " << strLine << std::endl;
            }
        }
    }
    return 0;
}

int main() {
    std::size_t intReturn;
    //std::time_t tmFileModTime, tmFileNewModTime;

    std::cout << "prim_aidoc version 2.00.03" << std::endl;
    /*
    std::cout << "Reading configuraiton file..." << std::endl;
    auto ftime = fs::last_write_time("/etc/primal/aidoc.conf");
    tmFileModTime = decltype(ftime)::clock::to_time_t(ftime);
    intReturn = fLaunchChild();
    while(1) {
        auto ftime = fs::last_write_time("/etc/primal/aidoc.conf");
        tmFileNewModTime = decltype(ftime)::clock::to_time_t(ftime);
        if(tmFileNewModTime > tmFileModTime) {
            std::cout << "File /etc/primal/aidoc.conf has changed.  Launching any new client IDs." << std::endl;
            intReturn = fLaunchChild();
            if(intReturn == 1) {
                std::cout << "Aborting Launch.  Please check the config file /etc/primal/aidoc.conf";
                return 1;
            }
        }
        std::this_thread::sleep_for (std::chrono::seconds(30));
    }
    */
    while(1) {
        intReturn = fLaunchChild();
        if(intReturn == 1) {
            return 1;
        }
        std::cout << "Sleeping for 60 seconds." << std::endl;
        std::this_thread::sleep_for (std::chrono::seconds(60));
    }

    std::cout << "Exiting prim_aidoc." << std::endl;
    return 0;
}