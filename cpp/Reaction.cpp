/*
 *    Alchemy, a metabolic pathway generator 
 *    Copyright (C) 2013  Robin Betz
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
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
      */
