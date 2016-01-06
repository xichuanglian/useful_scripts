#!/bin/sh

IFCE=eth4

host=`hostname | sed 's/^node-\([0-9]\+\)\.domain\.tld$/\1/'`
rack=$(((host - 1) / 15 + 1))
idx=$(((host - 1) % 15 + 1))

ip=10.5.$rack.$idx
mac=`/sbin/ifconfig $IFCE | grep HWaddr | awk '{print $5}'`
echo $ip $mac
