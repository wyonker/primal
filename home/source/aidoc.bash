#!/bin/bash

echo "Building aidoc"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_aidoc prim_aidoc.cpp -lstdc++fs -lrt -lcrypto
echo "Building aidoc_client"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_aidoc_client prim_aidoc_client.cpp -lstdc++fs -lrt -lcrypto
chmod 755 prim_aidoc
