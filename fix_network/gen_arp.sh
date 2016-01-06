#!/bin/sh

IFCE=eth4

echo "#!/bin/sh" > set_arp.sh
echo "#!/bin/sh" > clear_arp.sh
chmod +x set_arp.sh
chmod +x clear_arp.sh

while read line
do
echo "sudo /sbin/arp -i $IFCE -s $line" >> set_arp.sh
ip=`echo $line | awk '{print $1}'`
echo "sudo /sbin/arp -i $IFCE -d $ip" >> clear_arp.sh
done
