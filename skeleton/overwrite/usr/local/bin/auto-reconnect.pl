#!/usr/bin/perl

my @list = `ls -t /etc/netwiz/`;

my $current = `iwlist scan | grep Addr`;

for (@list) {

if ($current =~ /$_/) {

system "touch /tmp/netstat.tmp";
system "echo 1 > /tmp/netattempt";

system "sh -e /etc/netwiz/$_";
system "rm -f /tmp/netstat.tmp";

exit; 
}

}
