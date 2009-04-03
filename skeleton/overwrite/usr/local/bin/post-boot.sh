if [ ! -e /tmp/firsttime ]; then

touch /tmp/firsttime

udevd --daemon
udevadm trigger

for NIC in eth0 eth1 eth2 wlan0 ath0; do
	/bin/ifconfig $NIC up
	/sbin/iwconfig $NIC mode Managed
done

for channel in Master Front; do
	/usr/bin/amixer set $channel 80%
done

/bin/cp /sbin/hotplug-x /sbin/hotplug

fi 
