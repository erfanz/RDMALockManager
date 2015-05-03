#!/bin/bash

if [ $# -eq 0 ]; then
	echo "Needs at least one argument"
	exit 0
fi

file=$1
dest=""

if [ $# -eq 2 ]; then
	dest=$2
fi

echo "sending file(s) to /data/yeounoh/RDMALockManager/$dest"

scp -r  $file yeounoh@bsn00.cs.brown.edu:/data/yeounoh/RDMALockManager/$dest
