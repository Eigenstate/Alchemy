package Reaction;
use strict;
use warnings;
use Exporter; # Makes perl modules a little easier

our @ISA = qw( Exporter );

# Things that can be exported
our @EXPORT_OK = qw();

# Things that are exported by default
our @EXPORT = qw(merge);

# Creates an enzyme with the indicated properties
sub new 
{
  my $class = shift;
  my %params = @_;
  my $self = {
    substrate => $params{substrate},
    product   => $params{product},
    enzyme    => $params{enzyme},
    id        => $params{id},
    partner   => $params{partner},
  };
  bless $self, $class;
  return $self;
}

# Creates a new array of reactions that represents the union
# of an array of input reactions (used for reconstructing partner
# reactions)
sub merge
{
  my $self = shift;
  my $mer = shift;
  my @merge= @{$mer};

  # Find all non-null substrates and products
  my @s; my @p; 
  foreach my $rxn (@merge) {
    push @s, $rxn->{substrate} if ($rxn->{substrate} ne "NULL");
    push @p, $rxn->{product} if ($rxn->{product} ne "NULL");
  }

  # Create the substrate merged name
  my $subs = "";
  for (my $i=0; $i<(@s-1); ++$i) {
    $subs .= "$s[$i] + ";
  }
  $subs .= $s[scalar(@s)-1];

  # Create the product merged name
  my $prods = "";
  for (my $i=0; $i<(@p-1); ++$i) {
    $prods.= "$p[$i] + ";
  }
  $prods.= $p[scalar(@p)-1];
  # Create the merged reaction 
  my @reactions; 
  push @reactions, Reaction->new(
                    substrate  => $subs,
                    product    => $prods,
                    enzyme     => $merge[0]->{enzyme},
                    id         => 0,
                    partner    => 0,
                  );

  # Create "free" reactions to the merged reaction
  if (@s > 1) {
    foreach my $r (@s) {
     push @reactions, Reaction->new (
                        substrate => $r,
                        product   => $subs,
                        enzyme    => 0,
                        id        => 0,
                        partner   => 0,
                      );
    }
  }
  if (@p > 1) {
    foreach my $r (@p) {
     push @reactions, Reaction->new (
                        substrate => $prods,
                        product   => $r,
                        enzyme    => 0,
                        id        => 0,
                        partner   => 0,
                      );
    }
  }
  return \@reactions;
}

1;


