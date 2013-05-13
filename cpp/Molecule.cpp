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
Molecule::Molecule(const string &rn, const string &en, const string &sid)
  : distance(INT_MAX),
    rxnname(rn),
    ename(en),
    prev(NULL),
    struc_id(sid)
{}

const string Molecule::getMolID() { return rxnname; }
const string Molecule::getName() { return ename; }
const string Molecule::getStructureID() { return struc_id; }
const int Molecule::getDistance() { return distance; }
void Molecule::setDistance(int d) { distance = d; }
void Molecule::setPrevious(Molecule *p) { prev = p; }
Molecule* Molecule::getPrevious() { return prev; }
bool Molecule::isDummy() { return (struc_id == "DUMMY"); }
