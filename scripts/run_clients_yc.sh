#!/bin/bash

if [ $# -ne 2 ]; then
        echo "Usage: $0 <LockCLient | SRLockClinet> <#threads_per_client>"
        exit 0
fi

if [[ $1 != "LockClient" && $1 != "SRLockClient" ]]; then
        echo "Usage: $0 <LockClient | SRLockClient> <#threads_per_client>"
        exit 0
fi

threads_per_client=$2
x=1

while [ $x -le $threads_per_client ]
do
  if [ $1 == "SRLockClient" ]; then
          echo "Launching SRLockClient client $x ..."
          ./executables/SRserverCentricLM/SRLockClient &
  elif [ $1 == "LockClient" ]; then
          echo "Launching LockClient client $x ..."
          ./executables/serverCentricLM/LockClient &
  fi
  x=$(( $x + 1 ))
  sleep 0.001
done
