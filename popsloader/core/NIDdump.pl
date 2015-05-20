#!/usr/bin/perl --
use strict;
use warnings;
use Net::HTTP;
my $http=Net::HTTP->new(Host=>"data.j416.info",KeepAlive=>1) || die $@;
my $i=0;
for(;$i<31;$i++){
	$http->write_request(GET=>"/nid/index.cgi?n=$i");
	my($code,$mess,%h)=$http->read_response_headers();
	my($body,$buf,$n);$body="";
	do{
		$n=$http->read_entity_body($buf,2048);
		$body.=$buf;
	}while($n);
	$body=~s/\r//g;
	$body=~s/\n//g;
	my @lines=split(/<\/tr>/,$body);
	foreach my $line(@lines){
		#print $line."\n";
		if($line =~ m|<tr><td Nowrap><nobr>.*</nobr></td><td Nowrap>(0x[0-9a-fA-F]{8})</td><td Nowrap>(0x[0-9a-fA-F]*)</td><td Nowrap>(0x[0-9a-fA-F]*)</td><td Nowrap>(0x[0-9a-fA-F]*)</td><td Nowrap>(0x[0-9a-fA-F]*)</td>$|){
			#print join(",",$1,$2,$3,$4,$5).",\n";
			print join(",",$4,$5).",\n";
		}
	}
}
