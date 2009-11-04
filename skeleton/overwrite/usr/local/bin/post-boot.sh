#!/bin/bash

/usr/bin/xmodmap ~/.Xmodmap

# set mouse acceleration and threshold (pointer is too fast with new Xvesa build)
/usr/bin/xset m 1.5/1 4

if [ ! -e /tmp/firsttime ]; then

	# run this script once
	touch /tmp/firsttime

	# for aufs to merge opt-get plugin
	mount -t tmpfs none /opt

	# generate menu
	/usr/local/bin/update-menus

	# start udev daemon
	udevd --daemon
	udevadm trigger

	# start system bus
	dbus-launch --config-file=/etc/dbus-1/system.conf
	
	# start connman
	#connmand

	# work through NIC and wake them up
	/bin/ifconfig lo 127.0.0.1 netmask 255.0.0.0

	for NIC in eth0 eth1 eth2 wlan0 ath0 ra0; do
		/bin/ifconfig $NIC up
		/sbin/iwconfig $NIC mode Managed
	done

	# try to setup DHCP
	/sbin/dhclient &

	# get wireless SSID list
	/usr/local/bin/get_ssid &

	# mount all partitions
	for i in `fdisk -l | grep "^/dev" | cut -d' ' -f1`; do
	   mkdir -p /mnt/`basename $i`;
	   mount $i /mnt/`basename $i`;
	done

	# auto load user data if exist
	if [ ! "$(cat /proc/cmdline | grep xpud-data )" ]; then
		/usr/local/bin/load_data &
	fi

	# auto reconnect
	/usr/local/bin/auto-reconnect.pl &

	# start hotplug script
	/bin/cp /sbin/hotplug-x /sbin/hotplug

	# setup sound channel
	CARD=`grep '0 \[' /proc/asound/cards | cut -d'[' -f2| cut -d']' -f1`
	if [ ! -z $CARD ]; then 
	sed -e "s/Intel/$CARD/g" /etc/asound.conf > /etc/asound.conf.tmp
	mv /etc/asound.conf.tmp /etc/asound.conf
	for channel in Master Front PCM; do
		/usr/bin/amixer set $channel 95% on
	done
	fi

	# Atom-related setting 
	if [ "$(cat /proc/cpuinfo | grep Atom)" ] ; then
		modprobe acpi_cpufreq 
		modprobe cpufreq_ondemand
		echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 
		echo ondemand > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor
	fi

	# post hook
	find /etc/post-boot.d/ -type f -exec {} \;  

fi 
