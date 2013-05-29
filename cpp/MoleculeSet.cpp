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
#include <utility> // for pair
#include "Molecule.h"
#include "MoleculeSet.h"
//#include <iostream>
using namespace std;

void MoleculeSet::insertMolecule(Molecule* m)
{
  string pname = processName( m->getMolID() );
  data.insert( pair<string,Molecule*>(pname, m) );
}

Molecule* MoleculeSet::getMolecule(const string &name)
{
  //cout << name << " -> " << processName(name) << endl;
  return data.at( processName(name) );
}

// Puts a string into the internal format used in the map
// Removes (n+1), numbers, etc.
const string MoleculeSet::processName(const string &name)
{
  string pname = name;
  // Compound dummy reaction needs no processing
  int pluscount=0;
  for (unsigned int i=0; i<name.length(); ++i)
    if (name.at(i)=='+') ++pluscount; 
  if ( (pluscount>1)
    || (pluscount==1 && name.find("(n+")==string::npos) ) {
    return pname;
  }
  // Remove chain length specification
  if (pname.find("(")!=string::npos)
    pname = pname.substr(0,pname.find("("));
  // Remove number of molecule specification
  if (pname.find(" ")!=string::npos)
    pname = name.substr(pname.find(" ")+1);
  return pname;
}
