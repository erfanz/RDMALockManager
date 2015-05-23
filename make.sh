#!/bin/bash

mkdir -p ./obj
mkdir -p ./executables/clientCentericLM_Backoff
mkdir -p ./executables/combinedLM


g++ -c -Wno-write-strings util/utils.cpp -o obj/utils.o

g++ -c -Wno-write-strings util/RDMACommon.cpp -o obj/RDMACommon.o

g++ -c -Wno-write-strings -std=c++11 util/BaseContext.cpp  -o obj/BaseContext.o


########## Combined Lock Manager (Start)  ##########
# g++ -c -Wno-write-strings -std=c++11 combinedLM/CombinedServerContext.cpp -o obj/CombinedServerContext.o
# g++ -c -Wno-write-strings -std=c++11 combinedLM/CombinedClientContext.cpp -o obj/CombinedClientContext.o
#
# g++ -c -Wno-write-strings -std=c++11 combinedLM/CombinedServer.cpp -o obj/CombinedServer.o
# g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/CombinedServerContext.o obj/CombinedServer.o -libverbs -lpthread -o executables/combinedLM/CombinedServer
#
# g++ -c -Wno-write-strings -std=c++11 combinedLM/CombinedClient.cpp -o obj/CombinedClient.o
# g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/CombinedClientContext.o obj/CombinedClient.o -libverbs -lpthread -o executables/combinedLM/CombinedClient
########## Combined Lock Manager (End)  ##########


########## Backoff Lock Manager (Start)  ##########
g++ -c -Wno-write-strings -std=c++11 clientCentericLM_Backoff/BServerContext.cpp -o obj/BServerContext.o
g++ -c -Wno-write-strings -std=c++11 clientCentericLM_Backoff/BClientContext.cpp -o obj/BClientContext.o

g++ -c -Wno-write-strings -std=c++11 clientCentericLM_Backoff/BClientCentricServer.cpp -o obj/BClientCentericServer.o
g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/BServerContext.o obj/BClientCentericServer.o -libverbs -lpthread -o executables/clientCentericLM_Backoff/BClientCentericServer

g++ -c -Wno-write-strings -std=c++11 clientCentericLM_Backoff/BClientCentricClient.cpp -o obj/BClientCentericClient.o
g++ obj/utils.o obj/RDMACommon.o obj/BaseContext.o obj/BClientContext.o obj/BClientCentericClient.o -libverbs -lpthread -o executables/clientCentericLM_Backoff/BClientCentericClient
########## Backoff Lock Manager (End)  ##########






