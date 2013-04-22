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
  my @merge = @{$mer};
  my @merge2 = ( @merge );
 
  my $subs = my $prods = ""; 
  while (@merge) {
    my $rxn = pop @merge;
    $subs .= "$rxn->{substrate}";
    $prods .= "$rxn->{product}";
    if (@merge) {
      $subs .= " + ";
      $prods .= " + ";
    }
  }
 
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
  foreach my $rxn (@merge2) {
    push @reactions, Reaction->new (
                      substrate => $rxn->{substrate},
                      product   => $subs,
                      enzyme    => 0,
                      id        => 0,
                      partner   => 0,
                    );
    push @reactions, Reaction->new (
                      substrate => $prods,
                      product   => $rxn->{product},
                      enzyme    => 0,
                      id        => 0,
                      partner   => 0,
                    );
  }
  return \@reactions;
}

1;


