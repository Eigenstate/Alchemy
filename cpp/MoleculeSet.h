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
#ifndef MOLECULESET_H
#define MOLECULESET_H

#include <map>
#include <string>

using namespace std;
class Molecule;

class MoleculeSet
{
  // This is an ordered map because we do inserts only once
  // and then a lot of lookups
  map<string, Molecule*> data;
public:
  void insertMolecule(Molecule* m);
  Molecule* getMolecule(const string &name);
  map<string, Molecule*>::iterator getBeginIterator();
  map<string, Molecule*>::iterator getEndIterator();
  string generateName(const string &ids);
private:
  const string processName(const string &name);
};

#endif
