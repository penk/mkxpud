#!/usr/bin/perl

my @list = `ls /.mozilla/netwiz/`;

my $current = `iwlist scan | grep Addr`;

for (@list) {

if ($current =~ /$_/) {

system "touch /tmp/netstat.tmp";
system "echo 1 > /tmp/netattempt";

system "sh -e /.mozilla/netwiz/$_";
system "dhclient ath0; rm -f /tmp/netstat.tmp";

exit; 
}

}
