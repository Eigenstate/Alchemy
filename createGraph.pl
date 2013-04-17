#!/usr/bin/perl
use SOAP::Lite;
use warnings;
use strict;

use Database;

# Check if command line arguments have been given, otherwise
# prompt user for starting product and desired product
my ($start, $end);
if ( @ARGV == 0 ) {
  print "Enter starting product: ";
  $start = <>;
  print "Enter desired product at end: ";
  $end = <>;
} elsif ( @ARGV == 2) {
  $start = $ARGV[0];
  $end = $ARGV[1];
} else {
  die "Usage: ./createGraph.ph <start> <end>\n";
}

# Open the database
my $Database = new Database();
exit 0;

$\ = $, = "\n"; # Makes print nicer
# Get all enzymes that use this as a substrate
# String format Num1!Num2!Num3! etc
my $rawECs = SOAP::Lite
 -> uri('http://www.brenda-enzymes.info/soap2')
 -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php')
 -> getEcNumbersFromSubstrate()
 -> result;
my @ECs = split '!', $rawECs;

# Ensure all ECs found are unique before adding to db
my %uniqueECs;
foreach my $ec (@ECs) {
  $uniqueECs{$ec} = 1;
}

# Get products and add each enzyme to database
#foreach my $ec (keys %uniqueECs) {
print $ECs[0]; 
my $ec = $ECs[0]; { #debug
  my $rawProducts = SOAP::Lite
   -> uri('http://www.brenda-enzymes.info/soap2')
   -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php')
   -> getProduct("ecNumber*$ec#organism*Homo sapiens") # human enzymes only for now
   -> result;
  
  # output format ecnum*string#product*string#partners*string#organism*string#ligandstructurid*string
  my @results = split "!", $rawProducts;
  foreach my $res (@results) {
    if ($res =~ /.*product\*(.*)#reactionPartners\*(.*)#organism.*#ligandStructureId\*(.*)/) {
      my $prod = $1;
      my $rxnp = $2;
      my $id = $3;

      print "product $prod with $rxnp id $id\n";
    }
  }
}


# Returns the common names of an enzyme from its EC number using 
# the SOAP interface to BRENDA
# Returns an array reference
sub getEnzymeName
{
  my $ecnum = $_[0] or die ("getEnzymeNames requires an argument\n");
  my $resultstring = SOAP::Lite
  -> uri('http://www.brenda-enzymes.info/soap2')
  -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php')
  -> getEnzymeNames("ecNumber*$ecnum#organism*Homo sapiens") # humans only for now
  -> result;

  # Result format has entries separated by !
  my @enzymes = split "!", $resultstring;
  return \@enzymes;
}

# Returns the structure ID of a compound from the name
# This enables more standardization as names can vary
sub getStructureID {
  my $name = $_[0] or die "getStructureID requires an argument";
  my $result = SOAP::Lite
  -> uri('http://www.brenda-enzymes.info/soap2')
  -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php')
  -> getLigandStructureIdByCompoundName("$name")
  -> result;
  return $result;
}
