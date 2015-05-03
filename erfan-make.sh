#!/bin/bash

mkdir -p ./obj
mkdir -p ./executables/ClientCentricLM

g++ -c -Wno-write-strings util/utils.cpp -o obj/utils.o

g++ -c -Wno-write-strings util/RDMACommon.cpp -o obj/RDMACommon.o

g++ -c -Wno-write-strings -std=c++11 util/BaseContext.cpp  -o obj/BaseContext.o


g++ -c -Wno-write-strings -std=c++11 ClientCentericLM/ServerContext.cpp -o obj/ServerContext.o
g++ -c -Wno-write-strings -std=c++11 ClientCentericLM/ClientContext.cpp -o obj/ClientContext.o

g++ -c -Wno-write-strings -std=c++11 ClientCentericLM/ClientCentricServer.cpp -o obj/ClientCentricServer.o
g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/ServerContext.o obj/ClientCentricServer.o -libverbs -lpthread -o executables/ClientCentricLM/ClientCentricServer

g++ -c -Wno-write-strings -std=c++11 ClientCentericLM/ClientCentricClient.cpp -o obj/ClientCentricClient.o
g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/ClientContext.o obj/ClientCentricClient.o -libverbs -lpthread -o executables/ClientCentricLM/ClientCentricClient