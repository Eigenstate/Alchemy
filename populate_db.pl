#!/usr/bin/perl

# Adds all enzymes for the specified organism to the database

use warnings;
use strict;
use SOAP::Lite;
use Database;

my $ORGANISM = "homo sapiens";
my $db = Database->new();
# Get list of enzymes for this organism
my $resoap = SOAP::Lite
-> uri('http://www.brenda-enzymes.info/soap2')
-> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php');
my $resultString = $resoap->getEcNumber("organism*$ORGANISM")->result();
my @enzs = split '!', $resultString;

# Put enzymes in a hash since they're not unique
my %enzymes;
foreach my $en (@enzs) {
  $enzymes{$en} = 1 if (not exists $enzymes{$en});
}

# Loop through each enzyme and process all products
my $counter = 0;
foreach my $en (keys %enzymes) {
  ++$counter;
  if ($en =~ m/ecNumber\*(.*)#commentary/) {
    $en = $1;
  }
  print "On enzyme $counter of ", scalar(keys %enzymes),": $en\n";
  if ($db->checkEnzyme($en, $ORGANISM)) {
    print "  Already in db\n";
    next;
  }

  # Get the products of this enzyme
  my $string = "ecNumber*" . $en;
  my @results = @{&getProducts($string)};

  # output format ecnum*string#product*string#partners*string#organism*string#ligandstructurid*string
  foreach my $res (@results) {
    if ($res =~ m/ecNumber\*(.*)#product.*#reactionPartners\*(.*)#organism./) {
      my $enzyme = $1;
      my $rxnp = $2;
    # Add this to the database
     $db->addEntry($enzyme, $ORGANISM, $rxnp);
   }
  }
}

sub getProducts {
  my $string = shift;
  my $result2 = SOAP::Lite
  -> uri('http://www.brenda-enzymes.info/soap2')
  -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php');
  my $str = $result2->getProduct("$string")->result(); 
  my @results = split "!", $str;
  return \@results;
}
