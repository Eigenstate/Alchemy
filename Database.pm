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
  my $useBrenda = shift; # Say yes/no to continuing to populate db
  my $dbh = DBI->connect('dbi:mysql:alchemy', 'robin', 'testpw') or die "Connection error: $DBI::errstr\n";
  my $self = {
    "dbh" => $dbh,
    "brenda" => $useBrenda;
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
sub addEntry {
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

# Finds all enzymes that use a substrate, in the database
# Takes one argument, the substrate, and returns an array with enzymes
sub findEnzymeBySubstrate {
  my $substrate = $_[0] or die "Incorrect arguments to database lookup\n";
  
  # Create and run the query on the database
  my $query = "select enzyme from reactions where substrate=\"$substrate\";";
  my $sth = $self{dbh}->prepare($query);
  $sth->execute() or die "Database error: $DBI::errstr\n";
  
  # Process and return the result of the database query
  my @enzymes; my %dbenzymes;
  while (my $output = $sth->fetch()) {
		push @enzymes, $output; # TODO correct?
    $dbenzymes{%output} = 1;
  }
  return @enzymes if exists $self{brenda} and $self{brenda} != 0;
  
  # Run BRENDA query to pull out enzymes that might not be found if desired
  # TODO: What is the input to this?
  my %brendaenzymes; # Hash for uniqueness
	my $rawECs = SOAP::Lite
 	 -> uri('http://www.brenda-enzymes.info/soap2')
 	 -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php')
 	 -> getEcNumbersFromSubstrate() 
 	 -> result;
my @ECs = split '!', $rawECs;


  # Add enzymes that aren't yet in the local database to the local database
  foreach my $enzyme (%brendaenzymes) {
    if exists $enzymes 
  return @enzymes; 
}

# Finds all products an enzyme creates, in the database
sub findEnzymeByProduct {
}

# Adds an enzyme to the database given its EC number. Adds all reactions
# and their associated partners. 
sub addEnzyme {
  # Get all reactions performed by the input EC enzyme
  my $ec = shift or die "No EC number specified in Database addEnzyme\n";
  my $rawProducts = SOAP::Lite
   -> uri('http://www.brenda-enzymes.info/soap2')
   -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php')
   -> getProduct("ecNumber*$ec#organism*Homo sapiens") # human enzymes only for now
   -> result;
    
  # output format ecnum*string#product*string#partners*string#organism*string#ligandstructurid*string
  my @results = split "!", $rawProducts;
  foreach my $res (@results) {
    if ($res =~ /.*product\*(.*)#reactionPartners\*(.*)#organism.*#ligandStructureId\*(.*)/) {
      my $prod = $1;
      my $rxnp = $2;
      my $id = $3;

      print "product $prod with $rxnp id $id\n";
      # Split up the reaction: a + b -> c + d
      my @split = split '->', $rxnp;
      my @substrates = split '+', $split[0];
      my @products = split '+', $split[1];
      
      # Insert into the database until no partner reactions left
      while (@substrates and @products) {
        my ($s,$p) = (pop @substrates, pop @products); # todo-nonempty
				my $query = "insert into... $s $p";
        my $sth = $self{dbh}->prepare($query);
  			$sth->execute() or die "Database error: $DBI::errstr\n";
      }

     	# Check if substrates/products are in database
     
 		 	# If not, look up ligandstructureID and add to molecule database
       
      # Get enzyme common name and add to enzyme database
    }
  }
}

1;
