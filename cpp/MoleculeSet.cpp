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
#include <iostream>
#include "Molecule.h"
#include "MoleculeSet.h"
using namespace std;

void MoleculeSet::insertMolecule(Molecule* m)
{
  data.insert( pair<string,Molecule*>(m->getMolID(), m) );
}

Molecule* MoleculeSet::getMolecule(const string &name)
{
  string pname = name;
cout << "String: " << name << endl;
  // Compound dummy molecule needs no processing
  int pluscount=0;
  for (unsigned int i=0; i<name.length(); ++i)
    if (name.at(i)=='+') ++pluscount; 
  if ( (pluscount>1)
    || (pluscount==1 && name.find("(n+1)")==string::npos) ) {
    return data.at(name);
  }

  // Remove chain length specification
  if (name.find("(")!=string::npos)
    pname = name.substr(0,name.find("("));
  // Remove number of molecule specification
  if (name.find(" ")!=string::npos)
    pname = name.substr(name.find(" ")+1);
  return data.at(pname);
}
