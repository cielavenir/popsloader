#!/usr/bin/perl --
use strict;
use warnings;

if(!$ARGV[0]){exit;}


open(my $db,"<","NID635.csv");
my @db=<$db>;
close($db);
my %db=();
foreach my $line(@db){
	chomp($line);
	if($line=~/^(0x[0-9a-fA-F]*),(0x[0-9a-fA-F]*),/){
		if($db{$1}){print "duplicate $1\n";}
		else{$db{$1}=$2;}
	}
}

open(my $file,"<",$ARGV[0]);
my @file=<$file>;
close($file);

open($file,">",$ARGV[0]);
foreach my $line(@file){
	#print $line;
	if($line=~/	{ (0x[0-9a-fA-F]{8}), (0x[0-9a-fA-F]{8}), },/){
		if($db{$2}){
			print $2." translated into ".$db{$2}."\n";
			#print $2.",".$db{$2}.", #j416dy's list\n";
			print $file "	{ $1, $db{$2}, },\n";
		}else{
			print $2." cannot translate\n";goto fallback;
		}
	}else{
		fallback:
		print $file $line;
	}
}
