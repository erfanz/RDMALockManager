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
scp -r  $file yeounoh@bsn01.cs.brown.edu:/data/yeounoh/RDMALockManager/$dest
scp -r  $file yeounoh@bsn02.cs.brown.edu:/data/yeounoh/RDMALockManager/$dest
scp -r  $file yeounoh@bsn03.cs.brown.edu:/data/yeounoh/RDMALockManager/$dest
scp -r  $file yeounoh@bsn04.cs.brown.edu:/data/yeounoh/RDMALockManager/$dest
scp -r  $file yeounoh@bsn06.cs.brown.edu:/data/yeounoh/RDMALockManager/$dest
scp -r  $file yeounoh@bsn07.cs.brown.edu:/data/yeounoh/RDMALockManager/$dest
