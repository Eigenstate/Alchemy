package Database;
use strict;
use warnings;
use Exporter;  # Makes perl modules a little easier
use DBI;       # for mySQL
use Reaction;  # chemical reaction data structure

our @ISA = qw( Exporter );

# Things that can be exported
our @EXPORT_OK = qw();

# Things that are exported by default
our @EXPORT = qw(addEntry checkEnzyme);

# Opens the database file at an indicated location
sub new 
{
  my $class = shift;
  my $dbh = DBI->connect('dbi:mysql:alchemy', 'robin', 'testpw') or die "Connection error: $DBI::errstr\n";

  my $self = {
    dbh => $dbh,
  };

  bless $self, $class;
  return $self;
}

# Checks if an enzyme is already in the database for a given organism
# Returns 1 if yes, 0 if not
sub checkEnzyme {
  my $self = shift;
  my $enzyme = shift;
  my $organism = shift;
  my @result = @{$self->executeSelectQuery("SELECT rxn_id FROM reactions WHERE enzyme='$enzyme' AND organism='$organism';")};
  return 1 if (@result > 0);
  return 0;
}

# Adds an entry to the database given an ec number, an organism,
# and a reaction equation
sub addEntry {
  my $self = shift;
  # Process command line arguments
  my $enzyme = shift;
  my $organism = shift;
  my $rxnpartners = shift;

  # Check if the enzyme is in the enzyme database, if not, get its name and add it in 
  my @result = @{$self->executeSelectQuery("SELECT name FROM enzymes WHERE ec_num='$enzyme';")};
  if (@result == 0) {
    my $test = $self->executeInsertQuery("INSERT INTO enzymes (ec_num) VALUES('$enzyme');", "enzymes");
  }

  # Break up the reaction into substrates and products
  my @split = split / = /, $rxnpartners;
  my @substrates = split / \+ /, $split[0];
  my @products = split / \+ /, $split[1];
  $,=" ";

  # Add substrate/product pairs to the database
  my $last = "NULL";
  while (@substrates or @products) {
    my $s = "NULL"; my $p = "NULL";
    if (@substrates > 0) { $s = $self->addLigand(pop @substrates); }
    if (@products > 0) { $p = $self->addLigand(pop @products); }
    $last = $self->executeInsertQuery("INSERT INTO reactions (enzyme,substrate,product,partner_rxn,organism) VALUES('$enzyme','$s','$p',$last,'$organism');", "reactions");
  }
}

# Adds a named ligand to the database if it's not already there. Otherwise,
# does nothing. Returns the structure id of the ligand
sub addLigand {
  my $self = shift;
  my $ligand = shift;
  $ligand = quotemeta $ligand;
  
  # Check if already in database
  my @result = @{$self->executeSelectQuery("SELECT idx FROM molecules WHERE name='$ligand';")};
  $,=" ";
  return $result[0][0] if (@result != 0); # Ligand is 1st row, only field returned

  # Add to database if it doesn't exist yet
  my $id = SOAP::Lite
   -> uri('http://www.brenda-enzymes.info/soap2')
   -> proxy('http://www.brenda-enzymes.info/soap2/brenda_server.php')
   -> getLigandStructureIdByCompoundName("$ligand")
   -> result;
  if (! length $id) { $id = "NULL"; } # Not a defined structure ID in brenda for some reason
  
  my $idx = $self->executeInsertQuery("INSERT INTO molecules (structure_id,name) VALUES('$id','$ligand');","molecules");
  return $idx;
}

# Finds all enzymes that use a substrate, in the database
# Takes one argument, the substrate, and returns an array with enzymes
sub findEnzymeBySubstrate {
  my $self = shift;
  my $substrate = shift or die "Incorrect arguments to database lookup\n";
  $substrate = quotemeta $substrate;
  
  # Create and run the query on the database
  my $query = "SELECT enzyme FROM reactions WHERE substrate='$substrate';";
  my @enzymes= @{$self->executeSelectQuery($query)};
  return @enzymes; 
}

# Executes a SQL query on the database and returns the result
# This is to be used for select queries
# Argument is the SQL query to run, returns string result or dies with error
# If multiple rows are returned, an array is returned
sub executeSelectQuery {
  my ($self, $query) = @_;
  my $sth = $self->{dbh}->prepare("$query");
  $sth->execute() or die "Database error: $DBI::errstr\n\nQuery was '$query'";
  my $result = $sth->fetchall_arrayref();
  $sth->finish();
  return $result;
}

# Executes a SQL insert query on the database and returns the row ID inserted
# This is to be used for insert queries
# Argument is the SQL query to run and the table, returns row id or dies with error
sub executeInsertQuery {
  my $self = shift;
  my $query = shift or die "No query specified in executeInsertQuery!\n";
  my $table = shift or die "No table specified in executeInsertQuery!\n";
  $self->{dbh}->do("$query") or die "Database error: $DBI::errstr\n";
  return $self->{dbh}->last_insert_id("", "", "$table", "");
}

# Finds all products an enzyme creates, in the database
sub findEnzymeByProduct {
  my $self = shift;
}

# Returns a hash reference to all ligands
# Key = index, value = ligand name
sub getLigands {
  my $self = shift;

  # Get select query from database
  my $query = "SELECT * FROM molecules;";
  my @data = @{$self->executeSelectQuery($query)};
  
  # Process into a hash
  my $ligands = ();
  foreach my $row (@data) {
    $ligands->{@{$row}[0]} = @{$row}[2];
  }
  return $ligands;
}

# Returns a hash reference to all reactions
# Key = start, next key = edge, value = enzyme
sub getReactions {
  my $self = shift;
  my $q = shift;

  # Generate and get initial select query from database
  # We will handle results of partner reactions specially later
  my $query = "SELECT * FROM reactions WHERE ";
  while (@{$q}) {
    my $molecule = pop @{$q};
    $query .= "substrate=$molecule or product=$molecule ";
    $query .= "or " if (@{$q});
  }
  $query .= ";";
  my @initreactions = @{$self->selectAsReaction($query)};

  # Get the partner reactions of each reaction
  my @procrxns;
  foreach my $rxn (@initreactions) {
    my @tomerge;
    
    # Query backwards first
    my @blob = ( $rxn );
    while (@blob) {
      my $r = pop @blob;
      print "looking for partners with id $r->{id}\n";
      my $queryback = "SELECT * FROM reactions where partner_rxn='$r->{id}';";

      push @blob, @{$self->selectAsReaction($queryback)};
      push @tomerge, $r;
    }

    # Now query forwards
    @blob = ( $rxn );
    while (@blob) {
      my $r = pop @blob;
      if ($r->{partner} != 0) {
        print "querying id $r->{id} for partner $r->{partner}\n";
        my $queryforw = "SELECT * FROM reactions where rxn_id='$r->{partner}';";
        push @blob, @{$self->selectAsReaction($queryforw)};
      }
    }
    push @procrxns, @{Reaction->merge(\@tomerge)};
  }
  return \@procrxns;
}

# Returns a reaction data structure containing all reactions
# returned by an input query
sub selectAsReaction {
  my $self = shift;
  my $query = shift;

  my @reactions;
  my @data = @{ $self->executeSelectQuery($query) };
  foreach my $row (@data) {
    @{$row}[4] = 0 if (not defined @{$row}[4] or @{$row}[4] == "");
    push @reactions, Reaction->new(
                      substrate => @{$row}[2],
                      product   => @{$row}[3],
                      enzyme    => @{$row}[1],
                      id        => @{$row}[0],
                      partner   => @{$row}[4],
                    );
  }
  return \@reactions;
}
