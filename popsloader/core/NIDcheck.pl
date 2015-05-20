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
		if($db{$2}){print "duplicate $2\n";}
		else{$db{$2}=$1;}
	}
}

open(my $file,"<",$ARGV[0]);
my @file=<$file>;
close($file);

#open($file,">",$ARGV[0]);
foreach my $line(@file){
	#print $line;
	if($line=~/	{ (0x[0-9a-fA-F]{8}), (0x[0-9a-fA-F]{8}), },/){
		if($1 eq $2){
			print $2." doesn't need patch\n";
		}elsif($db{$2}){
			print $2." is 6.60\n";
			#print $2.",".$db{$2}.", #j416dy's list\n";
			#print $file "	{ $1, $db{$2}, },\n";
		}else{
			print $2." is NOT 6.60\n";goto fallback;
		}
	}else{
		fallback:
		#print $file $line;
	}
}
