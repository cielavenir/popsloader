#!/usr/bin/perl --
use strict;
use warnings;

open(my $file,"<","nid_660_to_635.c");
my @file=<$file>;
close($file);

foreach my $line(@file){
	#print $line;
	if($line=~/	{ (0x[0-9a-fA-F]{8}), (0x[0-9a-fA-F]{8}), },/){
		print "$2,$1, #PROCFW\n";
	}
}
