g++ -c -Wno-write-strings util/utils.cpp

g++ -c -Wno-write-strings util/RDMACommon.cpp

g++ -c -Wno-write-strings -std=c++11 util/BaseContext.cpp


g++ -c -Wno-write-strings -std=c++11 serverCentricLM/ClientContext.cpp
g++ -c -Wno-write-strings -std=c++11 serverCentricLM/ServerContext.cpp

g++ -c -Wno-write-strings -std=c++11 serverCentricLM/LockServer.cpp
g++ utils.o RDMACommon.o BaseContext.o ClientContext.o ServerContext.o LockServer.o -libverbs -lpthread -o LockServer

g++ -c -Wno-write-strings -std=c++11 serverCentricLM/LockClient.cpp
g++ utils.o RDMACommon.o BaseContext.o ClientContext.o LockClient.o -libverbs -lpthread -o LockClient
