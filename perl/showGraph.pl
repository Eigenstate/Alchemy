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

use warnings;
use strict;

use AlchemyGraph;
#die "No molecules specified!\n" if (@ARGV == 0);
my $test = AlchemyGraph->new(\@ARGV);
#my $test = AlchemyGraph->new();

while (1) {
print "Enter starting molecule(s), separated by space: ";
my $s = <>; chomp $s; my @start = split ' ', $s;
print "Enter desired end product: ";
my $p = <>; chomp $p;

$test->shortestPath(\@start, $p);
}
