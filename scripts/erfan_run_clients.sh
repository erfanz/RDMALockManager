#!/bin/bash

if [ $# -ne 2 ]; then
	echo "Usage: $0 <BACKOFF> <#threads_per_client>"
	exit 0
fi

if [[ $1 != "BACKOFF" && $1 != "ALAKI" ]]; then
	echo "Usage: $0 <BACKOFF | ALAKI> <#threads_per_client>"
	exit 0
fi

threads_per_client=$2
x=1

while [ $x -le $threads_per_client ]
do
  #echo "Launching transaction $x at $(date +%s%N | cut -b1-13)..." 
  if [ $1 == "BACKOFF" ]; then
	  echo "Launching BACKOFF client $x ..."
	  ./executables/clientCentericLM_Backoff/BClientCentericClient &
  elif [ $1 == "Trad" ]; then
	  echo "Launching Traditional client $x ..."
	  ./executables/serverCentricLM/SRLockClient &
  elif [ $1 == "IPTrad" ]; then
	  echo "Launching IP Traditional client $x ..."
	  ./executables/serverCentricLM/LockClient &
  else
	  echo "Launching Benchmark client $x ..."
	  ./BenchmarkClient &
  fi
  x=$(( $x + 1 ))
  sleep 0.001
done
