#!/usr/bin/perl
# -*- coding: utf-8 -*-

# po transifex files to rosetta.js script by tvelter

use strict;
use warnings;
my $i = 0;my $j = 0;my $k = 0;
my @lang;my @label;my $dirname = "../skeleton/locale";my $filename ;
my %total;

opendir ( DIR, $dirname ) || die "Error in opening dir $dirname\n";
while( ($filename =  readdir(DIR)))
{
	next if $filename eq '.' or $filename eq '..';
	my $docname = $dirname."\/".$filename ;
	$lang[$j]=  $filename;
	$lang[$j]=~ s/.po$// ;
	$lang[$j] = substr($lang[$j], -2);
	open(my $file,  "<",  $docname)  or die "Can't open file: $!";
	my @temp = <$file>;
	close $file or die "$file: $!";
$k=0;
	for ($i = 1 ; $i != $#temp+1 ; $i++)
	{
		if ($temp[$i]=~ /^msgid/)
		{
			$label[$k] = $temp[$i];
			$label[$k] =~ s/"$//;
			$label[$k] =~ s/^msgid "//;
			$temp[$i+1] =~ s/^msgstr/$lang[$j]:/;
			$total {$lang[$j]} {$label[$k]} = $temp[$i+1] ;
			$k +=1;
		};
	}
	$j += 1;
}

closedir(DIR);
my $temp;
open (MYFILE, '>>../skeleton/overwrite/usr/share/plate/chrome/content/rosetta.txt');

print MYFILE "// Plate UI translation file, generated from po files\n\nvar i18n =	{\n\n";
for ($j = 0 ; $j != $#label+1 ; $j++)
{
	$temp= $label[$j];
	$temp=~ s/$/ : {/;
	print MYFILE "\t$temp";
	for ($i = 0 ; $i != $#lang+1 ; $i++)
	{
		if ($label[$j] ne "")
		{
			if ($total {$lang[$i]} {$label[$j]} ne "$lang[$i]: \"\"\n")
			{
				$temp= $total {$lang[$i]} {$label[$j]};
				$temp=~ s/$/,/;
				print MYFILE "\t\t$temp";
			}
		}
	}
	print MYFILE "\t},\n\n";
}
print MYFILE "}";

print MYFILE "Bob\n";
close (MYFILE);
