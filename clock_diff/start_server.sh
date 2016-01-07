#!/bin/sh

IPT_RULE="eth4.*udp dpt:italk"
test=`iptables -L INPUT -v | grep -e "$IPT_RULE"`
if [ "x$test" = "x" ]; then
    iptables -I INPUT -i eth4 -p udp --dport 12345 -j ACCEPT
fi
nohup clock_diff/cdiff -s &>/dev/null &
