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
#ifndef MOLECULE_H
#define MOLECULE_H

#include <string>
using namespace std;

class Molecule {
  int distance;
  string ename;
  Molecule *prev;
  string kegg_id;
  int cost;

public:
  Molecule(const string &kid, const string &n);
  const string getMolID();
  const string getName();
  const int getDistance();
  const int getCost();
  void setDistance(int d);
  Molecule* getPrevious();
  void setPrevious(Molecule *p);
  bool isDummy();
};

#endif
