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
#include <sstream>
#include <climits>
#include <string.h>
#include "Molecule.h"

using namespace std;
Molecule::Molecule(string ids, string name)
  : distance(INT_MAX),
    prev(NULL),
    ename(name)
{
  if (!strcmp(name.c_str(),"DUMMY")) {
    if (ids.find("(n+1)") != string::npos) ids = ids.erase(ids.find("(n+1)"),5);
    if (ids.find("(n+2)") != string::npos) ids = ids.erase(ids.find("(n+2)"),5);
    stringstream *ss = new stringstream( ids ); 
    string item;
    while (getline(*ss, item, '+')) {
      if (item.find(" ") != string::npos) item = item.substr(item.find(" ")+1);
      kegg_id.push_back(item);
    }
  } else {
    kegg_id.push_back(ids);
  }
}

bool Molecule::search(const string id)
{
  for (unsigned int i=0; i<getMolIDs().size(); ++i)
    if (!strcmp(getMolIDs().at(i).c_str(),id.c_str())) return true;
  return false;
}

string Molecule::generateID()
{
  string s="";
  for (unsigned int i=0; i<getMolIDs().size(); ++i) {
    string t=getMolIDs().at(i);
    /*
    if (t.find(" ") != string::npos) t = t.erase(t.find(" ")+1);
    if (t.find("(n+1)")!=string::npos) t=t.erase(t.find("(n+1)"),5);
    if (t.find("(n+2)")!=string::npos) t=t.erase(t.find("(n+2)"),5);
    */
    s += t + "+";
  }
  return s.erase(s.length()-1);
}

vector<string> Molecule::getMolIDs() { return kegg_id; }
const string Molecule::getName() { return ename; }

const int Molecule::getDistance() { return distance; }
const int Molecule::getCost() { return kegg_id.size(); }
Molecule* Molecule::getPrevious() { return prev; }

void Molecule::setDistance(int d) { distance = d; }
void Molecule::setPrevious(Molecule *p) { prev = p; }
bool Molecule::isDummy() { return (!strcmp(getName().c_str(),"DUMMY")); }
