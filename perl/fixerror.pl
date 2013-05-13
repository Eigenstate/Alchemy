#!/usr/bin/perl

# Adds all enzymes for the specified organism to the database

use warnings;
use strict;
use SOAP::Lite;
use Database;

my $ORGANISM = "Arabidopsis thaliana";
my $db = Database->new();
 
  local $/=undef;
  open FILE, "error.txt" or die "No such file";
  my $file = <FILE>;
  close FILE;
  my @results = split '!', $file;

  # output format ecnum*string#product*string#partners*string#organism*string#ligandstructurid*string
  my $i = 0;
  foreach my $res (@results) {
    ++$i;
    print "On results $i of ", scalar(@results), "\n";
    if ($res =~ m/ecNumber\*(.*)#product.*#reactionPartners\*(.*)#organism./) {
      my $enzyme = $1;
      my $rxnp = $2;
    # Add this to the database
     $db->addEntry($enzyme, $ORGANISM, $rxnp);
   }
  }
  exit 0;

sub getProducts {
  my $string = shift;
  my $result2 = SOAP::Lite
  -> uri('http://www.brenda-enzymes.info/soap2')
  -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php');
  my $str = $result2->getProduct("$string")->result(); 
  my @results = split "!", $str;
  return \@results;
}
