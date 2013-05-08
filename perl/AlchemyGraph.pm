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

package AlchemyGraph;
use strict;
use warnings;
use Exporter;  # for easy export
use Database;  # for database access
use GraphViz;  # for visualization
use Reaction;  # for reaction data structure

our @ISA=qw( Exporter );
our @EXPORT_OK=qw();
our @EXPORT=qw(readData drawGraph);

# Creates a new, empty graph
sub new {
  my $class = shift;
  my $molecules = shift;
  
  my $db = Database->new();
  my $nodes = ();  # key=number, value=name of ligand
  my $edges = ();  # array of reactions
  my $readin = (); # key=node, value=1 for uniqueness checks
  my $uedges = ();  # key=start, key=end, value=name of enzyme
  my $self = {
    db => $db,
    edges => $edges,
    nodes => $nodes,
    query => $molecules,
    readin => $readin,
    uedges => $uedges,
  };
   
  $self->{nodes} = $self->{db}->getLigands();
  bless $self, $class;
  return $self;
}

# Reads in the edges data for a given query number
# Returns array reference to all edges that were added
sub readData 
{
  my $self = shift;
  my $query = shift;
  my @added;

  print "Reading in data for node @$query\n";
  my @edge = @{$self->{db}->getReactions($query)};                # Get edges
  foreach my $cmp (@edge) {                                         
    if (not exists $self->{uedges}->{ $cmp->{substrate} }->{ $cmp->{product} }) {
      $self->{uedges}->{ $cmp->{substrate} }->{ $cmp->{product} } = 1;
      push @{$self->{edges}}, $cmp;                               # Add this reaction to edges
      push @added, $cmp;

      if (not exists $self->{nodes}->{ $cmp->{substrate} }) {     # Add compound nodes
        $self->{nodes}->{ $cmp->{substrate} } = $cmp->{substrate};
        $self->{dist}->{ $cmp->{substrate} } = 1E100;
      }
      if (not exists $self->{nodes}->{ $cmp->{product} }) {
        $self->{nodes}->{ $cmp->{product} } = $cmp->{product};
        $self->{dist}->{ $cmp->{product} } = 1E100;
      }
    }
  }
  return \@added;
  # Remove duplicate edges - no longer used because edge adding already checks for duplicates
  #$self->removeDuplicates();
}

# Draws the graph
sub drawGraph 
{
  my $self = shift;
  my $output = shift;
  my $graph = GraphViz->new(directed => 1);

  # Add all the edges and nodes at once
  my %addednodes;
  print "Reading in data for graph\n";
  foreach my $rxn (@{$self->{edges}}) {
    # Add substrate node
    if (not exists $addednodes{ $rxn->{substrate} }) {
      if (index($rxn->{substrate},'+') == -1) {  # normal node
        $graph->add_node( $self->{nodes}->{ $rxn->{substrate} });
      } else {                                  # merged node
        $graph->add_node( $self->{nodes}->{ $rxn->{substrate} }, label => "", style => "filled");
      }
      $addednodes{ $rxn->{substrate} } = 1;
    }

    # Add product node
    if ( not exists $addednodes{ $rxn->{product} }) {
      if (index($rxn->{product},'+') == -1) {    # normal node
        $graph->add_node( $self->{nodes}->{ $rxn->{product} } );
      } else {                                  # merged node
        $graph->add_node( $self->{nodes}->{ $rxn->{product} }, label => "", style => "filled");
      }
      $addednodes{ $rxn->{product} } = 1;
    }

    # Add reaction edge
    if ($rxn->{enzyme} ne "0") { # Not a merged node
      $graph->add_edge( $self->{nodes}->{ $rxn->{substrate} } => $self->{nodes}->{ $rxn->{product} }, label => $rxn->{enzyme} );
    } else {
      $graph->add_edge( $self->{nodes}->{ $rxn->{substrate} } => $self->{nodes}->{ $rxn->{product} }, label => "");
    }
  }

  # Output a graph
  print "Writing graph image\n";
  $graph->as_png("$output");
  system("gwenview $output 2&>/dev/null");
}

# Draws the graph of a pathway
# Argument is array of reactions
# Second argument is name for output image
sub drawPathway
{
  my $self = shift;
  my $rxns = shift;
  my $output = shift;
  my $graph = GraphViz->new(directed => 1);
  
  foreach my $rxn (@$rxns) {
    if (index($rxn->{substrate}, '+') == -1) {
      $graph->add_node( $self->{nodes}->{ $rxn->{substrate} });
    } else {
      $graph->add_node( $self->{nodes}->{ $rxn->{substrate} }, label=> "", style=>"filled");
    }
    if (index($rxn->{product},'+')==-1) {
      $graph->add_node( $self->{nodes}->{ $rxn->{product} });
    } else {
      $graph->add_node( $self->{nodes}->{ $rxn->{product} }, label=> "", style=>"filled");
    }

    if ($rxn->{enzyme} ne "0") { # Not a merged node
      $graph->add_edge( $self->{nodes}->{ $rxn->{substrate} } => $self->{nodes}->{ $rxn->{product} }, label => $rxn->{enzyme} );
    } else {
      $graph->add_edge( $self->{nodes}->{ $rxn->{substrate} } => $self->{nodes}->{ $rxn->{product} }, label => "");
    }
  }
  print "Writing graph image\n";
  $graph->as_png("$output");
  system("gwenview $output");
}

# Removes duplicates from the input edges
# This is sort of a hack that hides underlying duplicates
# in the database that are more difficult to get rid of
sub removeDuplicates 
{
  print "Removing duplicate edges\n";
  my $self = shift;
  my @uniques;
  for (my $i=0; $i<@{$self->{edges}}; ++$i) {
    my $rxn = $self->{edges}[$i];
    my $unique = 1;
    for (my $j=$i+1; $j<@{$self->{edges}}; ++$j) {
      my $cmp = $self->{edges}[$j]; 
      if ( $rxn->{substrate} eq $cmp->{substrate} && 
           $rxn->{product}   eq $cmp->{product}   &&
           $rxn->{enzyme}    eq $cmp->{enzyme} ) {
        $unique = 0;
        last;
      }
    }
    push @uniques, $rxn if ($unique); }
  $self->{edges} = \@uniques;
}

# Returns the shortest path from a->b
# Takes an array of starting materials and tries to turn them into 
# the ending material
sub shortestPath
{
  my $self = shift;
  my $start = shift or die "Specify starting materials\n";
  my $end = shift or die "Specify ending material\n"; 

  # Read in initial stuffs if necessary
  if (not exists $self->{readin}->{$start}) {
    $self->readData($start);
    $self->{readin}->{$start} = 1;
  }

  # Initial distance is infinity except for source nodes
  print "Initializing distances\n";
  my $inf = 1E100; # stand in for ifinity
  my %previous;
  my %Queue;
  foreach my $node (keys %{$self->{nodes}}) {
    $self->{dist}->{ $node } = $inf;
    $Queue{ $node } = $inf;
  }
  print "There are ", scalar @$start, " starting nodes\n";
  foreach my $node (@{$start}) {
    $self->{dist}->{ $node } = 0;
    $Queue{ $node } = 0; # if (exists $Queue{ $node });
    print "Value node $node = $Queue{$node}\n";
  }
 
  # Calculate distances until the product node is found
  print "Calculating distances\n";
  while (scalar (keys %Queue)) {
    my $u; my $min = $inf;                           # Get minimal distance in queue
    my $first = 1;
    while (my ($v,$vv) = each %Queue) {
      if ($first) { $u = $v; $min = $vv; $first = 0; }
      if ($vv < $min) {
        $min = $vv;
        $u = $v;
    } }
    delete $Queue{$u};                               # Pop off the queue
    
    # Check for ending conditions
    last if ( $u eq $end );                          # Found the target 
    if ($self->{dist}->{$u} == $inf) {                         # Target unreachable
      print "No path found\n";
      return;
    }

    # Calculate distance to neighbors
    foreach my $rxn (@{$self->{edges}}) {
      if ($rxn->{substrate} eq $u) {
        my $v = $rxn->{product};
        next if (index($u,'+')!=-1 and index($v,'+')==-1);  # Ignore edges to non-compound nodes 

        # Add child nodes to internal memory if not there already
        if (index($v, '+') == -1) {
          if (not exists  $self->{readin}->{$v}) {
            $self->{readin}->{$v} = 1;
            my @inp = ( $v );
            $self->readData(\@inp)
          }
        }
        
        # Update distances
        my $alt = $self->{dist}->{$u} + 1;                     # Here, dist u,v=1, but this may change with KI and stuff.
        $alt = $self->{dist}->{$u} if ($rxn->{enzyme} eq "0"); # free transitions penalized
        if ($alt < $self->{dist}->{$v}) {
          $self->{dist}->{$v} = $alt;
          $previous{$v} = $u;
          $Queue{ $v } = $self->{dist}->{ $v };                # Add node to the queue
        }
      }
    }
  }

  # Retrive the path from start to end
  print "Retrieving sequence\n";
  my @S; my %uniqs;
  my $u = $end;
  while (exists $previous{$u}) {
    my $r1 = $self->getReaction($previous{$u},$u);
    push @S, $r1;
    $uniqs{$r1} = 1;

    if (index($u,'+') != -1) {                       # Pull in dependencies
      my @deps = split / \+ /, $u;
      foreach my $dep (@deps) {
        my $r2 = $self->getReaction($dep, $u);
        if ($r2 ne "NONE" and not exists $uniqs{$r2}) {
          push @S, $r2;
          $uniqs{$r2} = 1;
        }
      }
    }
    $u = $previous{$u};
  }
  print "Minimum distance = $self->{dist}->{$end}\n";
  $self->drawPathway(\@S, "test2.png");
}

# Returns the first reaction that turns a->b
# Input args a and b
sub getReaction
{
  my $self = shift;
  my $a = shift;
  my $b = shift;

print "Looking up '$a' -> '$b'\n";
  foreach my $rxn (@{$self->{edges}}) {
    if ( $rxn->{substrate} eq $a and
         $rxn->{product} eq $b ) {
      return $rxn;
    }
  }
  return "NONE";
}
