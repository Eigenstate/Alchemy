#!/usr/bin/perl
use warnings;
use strict;
use Database;

my $db = Database->new();
$db->removeDuplicates();
$db->createDummies();
