#!/bin/bash

if [ ! -e /tmp/firsttime ]; then

# run this script once
touch /tmp/firsttime

# start udev daemon
udevd --daemon
udevadm trigger

# work through NIC 
for NIC in eth0 eth1 eth2 wlan0 ath0 ra0; do
	/bin/ifconfig $NIC up
	/sbin/iwconfig $NIC mode Managed
done
/usr/local/bin/netdaemon eth0

/usr/local/bin/auto-reconnect.pl &
/usr/local/bin/get_ssid &

# setup sound channel
for channel in Master Front; do
	/usr/bin/amixer set $channel 95%
done

# post hook
find /etc/post-boot.d/ -type f -exec {} \;  

if [ "$(cat /proc/cpuinfo | grep Atom)" ] ; then
    modprobe acpi_cpufreq 
    modprobe cpufreq_ondemand
    echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 
    echo ondemand > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor
fi

# mount all partitions
for i in `fdisk -l | grep "^/dev" | cut -d' ' -f1`; do
   mkdir -p /mnt/`basename $i`;
   mount $i /mnt/`basename $i`;
done

# start hotplug script
/bin/cp /sbin/hotplug-x /sbin/hotplug

fi 
