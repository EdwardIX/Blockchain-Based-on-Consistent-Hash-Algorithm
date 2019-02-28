g++ database.cpp -std=c++11 -c -O2 -Wall
g++ client.cpp   -std=c++11 -c -O2 -Wall 
g++ transmit.cpp -std=c++11 -c -O2 -Wall
g++ report.cpp   -std=c++11 -c -O2 -Wall
g++ God.cpp      -std=c++11 -c -O2 -Wall
g++ User.cpp     -std=c++11 -c -O2 -Wall
g++ Server.cpp   -std=c++11 -c -O2 -Wall
g++ Checker.cpp  -std=c++11 -c -O2 -Wall

g++ God.o     database.o client.o transmit.o report.o -lws2_32 -lUrlmon -lwininet -lpsapi -static -O2 -Wall -o God
g++ User.o    database.o client.o transmit.o report.o -lws2_32 -lUrlmon -lwininet -lpsapi -static -O2 -Wall -o User
g++ Server.o  database.o client.o transmit.o report.o -lws2_32 -lUrlmon -lwininet -lpsapi -static -O2 -Wall -o Server
g++ Checker.o database.o -static -O2 -Wall -o Checker
