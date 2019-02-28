g++ database.cpp -std=c++11 -c -O2 -Wall -DNo_Delay
g++ client.cpp   -std=c++11 -c -O2 -Wall -DNo_Delay -DUsingSystemCurl
g++ transmit.cpp -std=c++11 -c -O2 -Wall -DNo_Delay
g++ report.cpp   -std=c++11 -c -O2 -Wall -DNo_Delay
g++ God.cpp      -std=c++11 -c -O2 -Wall -DNo_Delay
g++ User.cpp     -std=c++11 -c -O2 -Wall -DNo_Delay
g++ Server.cpp   -std=c++11 -c -O2 -Wall -DNo_Delay
g++ Checker.cpp  -std=c++11 -c -O2 -Wall -DNo_Delay

g++ God.o     database.o client.o transmit.o report.o -lws2_32 -lUrlmon -lwininet -lpsapi -static -O2 -Wall -o God
g++ User.o    database.o client.o transmit.o report.o -lws2_32 -lUrlmon -lwininet -lpsapi -static -O2 -Wall -o User
g++ Server.o  database.o client.o transmit.o report.o -lws2_32 -lUrlmon -lwininet -lpsapi -static -O2 -Wall -o Server
g++ Checker.o database.o -static -O2 -Wall -o Checker
