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
		$db{$1}=$2;
	}
}

open(my $file,"<",$ARGV[0]);
my @file=<$file>;
close($file);

open($file,">",$ARGV[0]);
foreach my $line(@file){
	#print $line;
	if($line=~/	{ (0x[0-9a-fA-F]{8}), (0x[0-9a-fA-F]{8}), },/){
		if($db{$1}){
			print $1." translated into ".$db{$1}."\n";
			print $file "	{ $1, $db{$1}, },\n";
		}else{
			print $1." cannot translate\n";#goto fallback;
			print $file "	{ $1, $1, },\n";
		}
	}else{
		fallback:
		print $file $line;
	}
}
