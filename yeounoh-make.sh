#!/bin/bash

mkdir -p ./obj
mkdir -p ./executables/serverCentricLM
mkdir -p ./executables/SRserverCentricLM


g++ -c -Wno-write-strings util/utils.cpp -o obj/utils.o

g++ -c -Wno-write-strings util/RDMACommon.cpp -o obj/RDMACommon.o

g++ -c -Wno-write-strings -std=c++11 util/BaseContext.cpp  -o obj/BaseContext.o


g++ -c -Wno-write-strings -std=c++11 SRserverCentricLM/SRClientContext.cpp -o obj/SRClientContext.o
g++ -c -Wno-write-strings -std=c++11 SRserverCentricLM/SRServerContext.cpp -o obj/SRServerContext.o

g++ -c -Wno-write-strings -std=c++11 SRserverCentricLM/SRLockServer.cpp -o obj/SRLockServer.o
g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/SRClientContext.o obj/SRServerContext.o obj/SRLockServer.o -libverbs -lpthread -o executables/SRserverCentricLM/SRLockServer

g++ -c -Wno-write-strings -std=c++11 SRserverCentricLM/SRLockClient.cpp -o obj/SRLockClient.o
g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/SRClientContext.o obj/SRLockClient.o -libverbs -lpthread -o executables/SRserverCentricLM/SRLockClient


g++ -c -Wno-write-strings -std=c++11 serverCentricLM/ClientContext.cpp -o obj/ClientContext.o
g++ -c -Wno-write-strings -std=c++11 serverCentricLM/ServerContext.cpp -o obj/ServerContext.o

g++ -c -Wno-write-strings -std=c++11 serverCentricLM/LockServer.cpp -o obj/LockServer.o
g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/ClientContext.o obj/ServerContext.o obj/LockServer.o -libverbs -lpthread -o executables/serverCentricLM/LockServer

g++ -c -Wno-write-strings -std=c++11 serverCentricLM/LockClient.cpp -o obj/LockClient.o
g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/ClientContext.o obj/LockClient.o -libverbs -lpthread -o executables/serverCentricLM/LockClient
