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
using namespace std;

void MoleculeSet::insertMolecule(Molecule* m)
{
  if (m->isDummy())
    data.insert( pair<string,Molecule*>(m->getName(), m) );
  else
    data.insert( pair<string,Molecule*>(m->getMolID(), m) );
}

Molecule* MoleculeSet::getMolecule(const string &name)
{
  return data.at(name);
}
