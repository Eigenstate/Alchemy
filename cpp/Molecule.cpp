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
#include <string>
#include <climits>
#include "Molecule.h"

using namespace std;
Molecule::Molecule(const string &kid, const string &n)
  : distance(INT_MAX),
    ename(n),
    prev(NULL),
    kegg_id(kid),
    cost(1)
{
  if (n=="DUMMY") {
    for (unsigned int i=0; i<n.size(); ++i) {
      if (n[i] == '+')
        ++cost;
  } }
}

const string Molecule::getMolID() { return kegg_id; }
const string Molecule::getName() { return ename; }
const int Molecule::getDistance() { return distance; }
const int Molecule::getCost() { return cost; }
void Molecule::setDistance(int d) { distance = d; }
void Molecule::setPrevious(Molecule *p) { prev = p; }
Molecule* Molecule::getPrevious() { return prev; }
bool Molecule::isDummy() { return (getName() == "DUMMY"); }
