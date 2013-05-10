#!/usr/bin/perl
#
#    Alchemy, a metabolic pathway generator 
#    Copyright (C) 2013  Robin Betz
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


# Adds all enzymes for the specified organism to the database

use warnings;
use strict;
use SOAP::Lite;
use Database;

my $ORGANISM = "Saccharomyces cerevisiae";
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
