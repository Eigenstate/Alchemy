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
  my @result = @{$self->executeSelectQuery("SELECT enzyme FROM reactions WHERE enzyme='$enzyme' AND organism='$organism';")};
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

  # Add small molecules to database and create query with 
  # appropriately indexed small molecules
  my $s = ''; my $p = '';
  while (@substrates or @products) {
    if (@substrates > 0) { $s .= $self->addLigand(pop @substrates); }
    if (@products > 0) { $p .= $self->addLigand(pop @products); }
    
    if (@substrates > 0) { $s .= "+"; };
    if (@products > 0) { $p .= "+"; }
  }

  # Add substrate/product pairs to the database
    $self->executeInsertQuery("INSERT IGNORE INTO reactions (enzyme,substrate,product,organism) VALUES('$enzyme','$s','$p','$organism');", "reactions");
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
  my $inpq = shift;
  my @q = @$inpq; 

  # Generate and get initial select query from database
  # We will handle results of partner reactions specially later
  my $query = "SELECT * FROM reactions";
  if (@q) {
    $query .= " WHERE ";
    while (@q) {
      my $molecule = pop @q;
      $query .= "substrate=$molecule or product=$molecule ";
      $query .= "or " if (@q);
    } 
  } else {
    print "Defaulting to reading in all reactions\n";
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
      my $queryback = "SELECT * FROM reactions where partner_rxn='$r->{id}';";

      push @blob, @{$self->selectAsReaction($queryback)};
      push @tomerge, $r;
    }

    # Now query forwards
    @blob = ( $rxn );
    while (@blob) {
      my $r = pop @blob;
      if ($r->{partner} != 0) {
        my $queryforw = "SELECT * FROM reactions where rxn_id='$r->{partner}';";
        push @blob, @{$self->selectAsReaction($queryforw)};
      }
      push @tomerge, $r if ($r != $rxn);
    }
    if (@tomerge == 1) {
      push @procrxns, @tomerge;
    } else {
      push @procrxns, @{Reaction->merge(\@tomerge)};
    }
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
    @{$row}[4] = 0 if (not defined @{$row}[4] or @{$row}[4] eq "");
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
