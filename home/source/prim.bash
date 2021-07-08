#!/bin/bash

echo "Building receive_server"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_receive_server prim_receive_server.cpp -lstdc++fs -lrt -lcrypto
chmod 755 prim_receive_server
echo "Building process_server"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_process_server prim_process_server.cpp -lstdc++fs -lrt -lcrypto -lcurl
chmod 755 prim_process_server
echo "Building send_server"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_send_server prim_send_server.cpp -lstdc++fs -lrt -lssh2
chmod 755 prim_send_server
echo "Building send_worker"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_send_worker prim_send_worker.cpp -lstdc++fs -lrt -lssh2
chmod 755 prim_send_worker
echo "Building qr_server"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_qr_server prim_qr_server.cpp -lstdc++fs -lrt -lcrypto
chmod 755 prim_qr_server
echo "Building qr_poll_server"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_qr_poll_server prim_qr_poll_server.cpp -lstdc++fs -lrt -lcrypto
chmod 755 prim_qr_server
echo "Building store_server"
g++ -g -Wall -Wextra -std=c++17 -pthread -lstdc++fs -I/usr/include/mysql -I/usr/include/mysql++ -L/usr/lib64/mysql -Bstatic -lmysqlpp -lmysqlclient -o prim_store_server prim_store_server.cpp -lstdc++fs -lrt -lcrypto
chmod 755 prim_store_server
