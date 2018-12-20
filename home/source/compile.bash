#!/bin/bash
g++ -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -lmysqlpp -lmysqlclient -std=c++11 rec_check.cc -o rec_check
g++ -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -lmysqlpp -lmysqlclient -std=c++11 pull_markup.cc -o pull_markup
