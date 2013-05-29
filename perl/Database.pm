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
use LWP::Simple;
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
  my $enzdef = shift;
  my $organism = shift;
  my $rxnpartners = shift;

  # Check if the enzyme is in the enzyme database, if not, get its name and add it in 
  $enzdef = quotemeta $enzdef;
  my $test = $self->executeInsertQuery("INSERT IGNORE INTO enzymes (uniprot,name) VALUES('$enzyme','$enzdef');", "enzymes");

  # Break up the reaction into substrates and products
  my @split = split / <=> /, $rxnpartners;
  my @substrates = split / \+ /, $split[0];
  my @products = split / \+ /, $split[1];
  $,=" ";

  # Add small molecules to database and create query with 
  # appropriately indexed small molecules
  my $s = join "+", @substrates;
  my $p = join "+", @products;
  while (@substrates or @products) {
    if (@substrates > 0) { $self->addLigand(pop @substrates); }
    if (@products > 0) { $self->addLigand(pop @products); }
  }
  # Remove spaces such as "5 H20" -> H20
  if ( index ($s, " ") != -1) { $s = substr( $s, index($s, " ")+1 ); }
  if ( index ($p, " ") != -1) { $p = substr( $p, index($p, " ")+1 ); }
  # Add substrate/product compund pairs to the database
  $self->addLigand( $s );
  $self->addLigand( $p );
  # Add substrate/product pairs to the database
  $self->executeInsertQuery("INSERT IGNORE INTO reactions (enzyme,substrate,product,organism) VALUES('$enzyme','$s','$p','$organism');", "reactions");
}

# Adds a named ligand to the database if it's not already there. Otherwise,
# does nothing. Returns the structure id of the ligand
sub addLigand {
  my $self = shift;
  my $ligand = shift;
  $ligand =~ s/^\s+//;  # remove leading spaces
  $ligand =~ s/\s+$//;  # remove trailing spaces
  # Check if already in database
  my @result = @{$self->executeSelectQuery("SELECT kegg_id FROM molecules WHERE kegg_id='$ligand';")};
  return $result[0][0] if (@result != 0); # Ligand is 1st row, only field returned

  # Add to database if it doesn't exist yet
  my $name;
  if ( index($ligand, '+') == -1) {
    $ligand =~ s/\(.*?\)//;
    my $rr = get "http://rest.kegg.jp/get/$ligand" or die "Unable to fetch ligand $ligand\n";
    if ($rr =~ m/NAME        (.*)\n/) {
      $name = $1;
      chop $name if ( substr $name, -1 eq ";" );
    } else { 
      if ($rr =~ m/ENTRY       (.*?)\s/) {
        $name = $1;
      } else {die "Error finding ligand $ligand\n"; }
    }
  }
  else { $name= "DUMMY"; }

  $name = quotemeta $name;
  my $idx = $self->executeInsertQuery("INSERT IGNORE INTO molecules (kegg_id,name) VALUES('$ligand','$name');","molecules");
  return $idx;
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
  $self->{dbh}->do("$query") or die "Database error: $DBI::errstr\nQuery was $query\n";
  return $self->{dbh}->last_insert_id("", "", "$table", "");
}

# Removes duplicate molecule counts like 2 C0001 and 3 C0001 should be listed as just C0001
sub removeDuplicates
{
  my $self = shift;
  my @ligands = @{$self->executeSelectQuery("SELECT * FROM molecules WHERE name!='DUMMY';")};
  my $errcount=0;
  for (my $i=0; $i<@ligands; ++$i) {
    for (my $j=$i+1; $j<@ligands; ++$j) { 
      # Check and handle duplicates
      if (index (@{$ligands[$i]}[0],@{$ligands[$j]}[0]) > 0) {
        print "Handling duplicates containing @{$ligands[$j]}[0]\n";
        $self->{dbh}->do("DELETE FROM molecules WHERE kegg_id like '%@{$ligands[$j]}[0]%';") 
          or die "Database error: $DBI::errstr\nDeletion was was '%@{$ligands[$j]}[0]%'\n";
        $self->{dbh}->do("INSERT IGNORE INTO molecules (kegg_id,name) VALUES('@{$ligands[$j]}[0]','@{$ligands[$j]}[1]');")
          or die "Database error: $DBI::errstr\nInsertion was '@{$ligands[$j]}[0]','@{$ligands[$j]}[1]'\n";
        ++$errcount;
        last;
      } elsif (index (@{$ligands[$j]}[0],@{$ligands[$i]}[0]) > 0) {
        print "Handling duplicates containing @{$ligands[$i]}[0]\n";
        $self->{dbh}->do("DELETE FROM molecules WHERE kegg_id like '%@{$ligands[$i]}[0]%';") 
          or die "Database error: $DBI::errstr\nDeletion was %@{$ligands[$i]}[0]%\n";
          my $qm = quotemeta @{$ligands[$i]}[1];
        $self->{dbh}->do("INSERT IGNORE INTO molecules (kegg_id,name) VALUES('@{$ligands[$i]}[0]','$qm');")
          or die "Database error: $DBI::errstr\nQuery was '@{$ligands[$i]}[0]','@{$ligands[$i]}[1]'\n";
        ++$errcount;
        last;
      }
    }
  }
  print "Removed $errcount duplicates\n";
  return;
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

# Creates dummy reactions
sub createDummies {
  my $self = shift;
  my @rxns = @{$self->executeSelectQuery("SELECT substrate,product FROM reactions;")};
  my $count=0;
  foreach my $rxn(@rxns) {
    my $sub = @{$rxn}[0];
    my $prod = @{$rxn}[1];
   
    # Check if the substrate needs a dummy 
    my $pc=0;
    for (my $i=0; $i<length $sub; ++$i) {
      if (substr($sub,$i,1) eq "+") { ++$pc; }
    }
    if ( ($pc>1) || 
         ($pc==1 and index($sub,"(n+")==-1) ) {
      $self->{dbh}->do("INSERT IGNORE INTO molecules (kegg_id,name) VALUES('$sub','DUMMY')")
        or die "Database error: $DBI::errstr\nInsert was $sub\n";
      ++$count;
    }

    # Now check if the product needs a dummy
    $pc=0;
    for (my $i=0; $i<length $prod; ++$i) {
      if (substr($prod,$i,1) eq "+") { ++$pc; }
    }
    if ( ($pc>1) || 
         ($pc==1 and index($prod,"(n+")==-1) ) {
      $self->{dbh}->do("INSERT IGNORE INTO molecules (kegg_id,name) VALUES('$prod','DUMMY')")
        or die "Database error: $DBI::errstr\nInsert was $prod\n";
      ++$count;
    }
  }
  print "Added $count dummies\n";
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
