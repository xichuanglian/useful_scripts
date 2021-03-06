#!/bin/sh

IPT_RULE="eth4.*udp dpt:italk"

pkill ^cdiff$
test=`iptables -L INPUT -v | grep -e "$IPT_RULE"`
if [ "x$test" = "x" ]; then
    iptables -D INPUT -i eth4 -p udp --dport 12345 -j ACCEPT
fi
