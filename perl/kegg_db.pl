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
use LWP::Simple;
use Database;
my $ORGANISM = "$ARGV[0]";
my $db = Database->new();
my @enzymes; my @reactions;
# Get list of reactions for this organism
my $rr = get "http://rest.kegg.jp/link/reaction/$ORGANISM"
  or die "Could get reactions for $ORGANISM\n";
my @rawrxns = split "\n", $rr;
foreach my $r (@rawrxns) {
  my @temp = split ' ', $r;
  push @enzymes, $temp[0];
  push @reactions, $temp[1];
}

#for (my $i=0; $i<@reactions; ++$i) {
for (my $i=1250; $i<@reactions; ++$i) {
  print "On $i of ", scalar @reactions, "\n";
  my ($enz, $def) = &lookupEnzyme($enzymes[$i]);
  my $rxn = &lookupReaction($reactions[$i]);
  $db->addEntry($enz, $def, $ORGANISM, $rxn);
  sleep 1;
}

sub lookupEnzyme {
  my $enzyme=shift;
  my $url = "http://rest.kegg.jp/get/$enzyme";
  my $rr = get $url or die "Couldn't get enzyme $enzyme\n";
  my $up; my $definition;
  if ($rr =~ m/UniProt: (.*?)\s/) {
    $up = $1;
  } else { 
    if ($rr =~ m/\[EC:(.*)\]/) {
      $up = $1;
    } else {die "No ec number or uniprot found for $enzyme\n"; }
  }
  if ($rr =~ m/DEFINITION  (.*)\n/) {
    $definition = $1;
  } else { die "No definition found for $enzyme\n"; }
  chomp $up; chomp $definition;
  return ($up, $definition);
}

sub lookupReaction {
  my $rxn = shift;
  my $url = "http://rest.kegg.jp/get/$rxn";
  my $rr = get $url or die "Couldn't get reaction $rxn\n";
  my $eqn;
  if ($rr =~ m/EQUATION    (.*)\n/) {
    $eqn = $1;
  } else { die "No equation found for $rxn\n"; }
  chomp $eqn;
  return $eqn;
}
