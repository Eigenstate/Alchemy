#!/usr/bin/perl

use SOAP::Lite;

my $resultstring = SOAP::Lite
-> uri('http://www.brenda-enzymes.info/soap2')
-> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php')
-> getEcNumbersFromSubstrate();
-> result;

my @orgs = split '!', $resultstring;
$,=$\="\n";
print @orgs;
