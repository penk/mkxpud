#!/bin/bash

if [ ! -e /tmp/firsttime ]; then

# run this script once
touch /tmp/firsttime

# start udev daemon
udevd --daemon
udevadm trigger

# work through NIC and wake them up
for NIC in eth0 eth1 eth2 wlan0 ath0 ra0; do
	/bin/ifconfig $NIC up
	/sbin/iwconfig $NIC mode Managed
done

# try to setup DHCP for eth0
/usr/local/bin/netdaemon eth0

# get wireless SSID list
/usr/local/bin/get_ssid &

# setup sound channel
CARD=`grep '0 \[' /proc/asound/cards | cut -d'[' -f2| cut -d']' -f1`
if [ ! -z $CARD ]; then 
sed -e "s/Intel/$CARD/g" /etc/asound.conf > /etc/asound.conf.tmp
mv /etc/asound.conf.tmp /etc/asound.conf
for channel in Master Front PCM; do
	/usr/bin/amixer set $channel 95% on
done
fi

# post hook
find /etc/post-boot.d/ -type f -exec {} \;  

# Atom-related setting 
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

# auto load user data if exist
if [ ! "$(cat /proc/cmdline | grep xpud-data.gz )" ]; then
	/usr/local/bin/load_data
fi
# auto reconnect
/usr/local/bin/auto-reconnect.pl &

# start hotplug script
/bin/cp /sbin/hotplug-x /sbin/hotplug

fi 
