package Database;
use strict;
use warnings;
use Exporter; # Makes perl modules a little easier
use DBI;      # for mySQL

our @ISA = qw( Exporter );

# Things that can be exported
our @EXPORT_OK = qw();

# Things that are exported by default
our @EXPORT = qw(addEntry);

# Opens the database file at an indicated location
sub new 
{
  my $class = shift;
  my $dbh = DBI->connect('dbi:mysql:alchemy', 'robin', 'testpw') or die "Connection error: $DBI::errstr\n";
  my $self = {
    "_dbh" => $dbh,
  };

  my $test = "show tables;";
  my $sth = $dbh->prepare($test);
  $sth->execute() or die "Error $DBI::errstr\n";
  while (my  @output = $sth->fetchrow_array()) {
    print "@output\n";
  }
  $sth->finish();
  bless $self, $class;
  return $self;
}

# Takes 4 arguments: substrate, enzyme, product, rxn partners
# These should be in the ligandstructureID format
sub DBaddEntry {
  # Process command line arguments
  die "Incorrect arguments to Database addEntry\n" if ( @_ != 4 );
  my $substrate = $_[0];
  my $enzyme = $_[1];
  my $product = $_[2];
  my @rxnpartners = @{$_[3]};

  $,=" "; $\="";
  #open DB, ">>", $dbFile;
  #print DB "$substrate+$enzyme+$product+@rxnpartners\n";
  #close DB;
}

1;
