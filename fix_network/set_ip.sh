#!/bin/sh

IFCE=eth4
host=`hostname | sed 's/^node-\([0-9]\+\)\.domain\.tld$/\1/'`
rack=$(((host - 1) / 15 + 1))
idx=$(((host - 1) % 15 + 1))

echo "Host $host: 10.5.$rack.$idx"
sudo /sbin/ifconfig $IFCE 10.5.$rack.$idx/16
