g++ -c -Wno-write-strings util/utils.cpp

g++ -c -Wno-write-strings util/RDMACommon.cpp

g++ -c -Wno-write-strings -std=c++11 util/BaseContext.cpp




g++ -c -Wno-write-strings -std=c++11 ServerCentricLM/ServerContext.cpp
g++ -c -Wno-write-strings -std=c++11 ServerCentricLM/ClientContext.cpp

g++ -c -Wno-write-strings -std=c++11 ServerCentricLM/LockServer.cpp
g++ utils.o RDMACommon.o BaseContext.o ServerContext.o LockServer.o -libverbs -lpthread -o BenchmarkServerRDMA

g++ -c -Wno-write-strings -std=c++11 ServerCentricLM/LockClient.cpp
g++ utils.o RDMACommon.o BaseContext.o ClientContext.o LockClient.o -libverbs -lpthread -o BenchmarkClientRDMA