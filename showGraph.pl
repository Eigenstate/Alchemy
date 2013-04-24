#!/usr/bin/perl

use AlchemyGraph;
die "No molecules specified!\n" if (@ARGV == 0);
my $test = AlchemyGraph->new(\@ARGV);
$test->drawGraph("test2.png");
