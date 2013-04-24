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
  my $edges = ();  # key=start, value=end, value=name of enzyme
  my $self = {
    db => $db,
    nodes => $nodes,
    edges => $edges,
    query => $molecules,
  };
   
  bless $self, $class;
  $self->readData();

  return $self;
}

# Reads in the nodes and edges data
sub readData {
  my $self = shift;
  my $query = $self->{query};
  $self->{nodes} = $self->{db}->getLigands();
  $self->{edges} = $self->{db}->getReactions($query);
}

# Draws the graph
sub drawGraph {
  my $self = shift;
  my $output = shift;
  my $graph = GraphViz->new(directed => 1);

  # Add all the edges and nodes at once
  my %addednodes;
  my $addededges;
  print "Reading in data for graph\n";
  foreach my $rxn (@{$self->{edges}}) {
    # Add substrate node
    if (not exists $addednodes{ $rxn->{substrate} }) {
      if (not exists $self->{nodes}->{ $rxn->{substrate} }) {  # this is a merged node, add it
        $self->{nodes}->{ $rxn->{substrate} } = $rxn->{substrate};
      }
      $graph->add_node( $self->{nodes}->{ $rxn->{substrate} });
      print "added node $self->{nodes}->{ $rxn->{substrate} } of $rxn->{substrate}\n";
      $addednodes{ $rxn->{substrate} } = 1;
    }

    # Add product node
    if ( not exists $addednodes{ $rxn->{product} }) {
      if (not exists $self->{nodes}->{ $rxn->{product} }) {  # this is a merged node, add it
        $self->{nodes}->{ $rxn->{product} } = $rxn->{product};
      } 
      $graph->add_node( $self->{nodes}->{ $rxn->{product} } );
      $addednodes{ $rxn->{product} } = 1;
    }

    # Add reaction edge
    if (not exists $addededges->{ $self->{nodes}->{$rxn->{substrate}} }->{ $self->{nodes}->{$rxn->{product}} }) {
      $graph->add_edge( $self->{nodes}->{ $rxn->{substrate} } => $self->{nodes}->{ $rxn->{product} }, label => $rxn->{enzyme} );
      $addededges->{ $self->{nodes}->{$rxn->{substrate}} }->{ $self->{nodes}->{$rxn->{product}} } = 1;
    }
  }

  # Output a graph
  print "Writing graph image\n";
  $graph->as_png("$output");
  system("gwenview $output");
}
