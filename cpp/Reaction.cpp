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
#include <iostream>
#include "Reaction.h"
using namespace std;

Reaction::Reaction(const int &sub, const int &prod, const std::string& en, const int &i, const int &partner)
      : substrate(sub), 
        product(prod), 
        enzyme(en), 
        id(i), 
        partner(partner)
        {}

Reaction::Reaction(const list<Reaction*> merge)
{
  if (merge.size() == 1) {
    substrate = list[0]->getSubstrate();
    product = list[0]->getProduct();
    enzyme = list[0]->getEnzyme();
    id = list[0]->getID();
    partner = 0;
  } else {
    substrate = product = "";
    for (int i=0; i<merge.size()-1; ++i) {
      substrate += merge[i]->getSubstrate() + "+";
      product += merge[i]->getProduct() + "+";
    }
  }
}

void Reaction::print()
{
  cout << substrate << " -> " << product << " by " << enzyme << endl;
}

string Reaction::getProduct()
{ return product; }

string Reaction::getEnzyme()
{ return enzyme; }

int Reaction::getSubstrate()
{ return substrate; }

int Reaction::getPartner()
{ return partner; }

int Reaction::getID()
{ return id; }

list<Reaction*> Reaction::queryBack(vector<Reactions>* l)
{
  list<Reaction*> result;
  for (int i=0; i<l.size(); ++i) {
    if (getID() == l[i].getPartner())
      result[0].push_back(&(l[i]));
  }
  return result;
}

list<Reaction*> Reaction::queryForward(vector<Reactions>* l)
{
  list<Reaction*> result;
  for (int i-0; i<l.size(); ++i) {
    if (l[i].getID() == getPatner())
      result.push_back(&(l[i]));
  }
  return result;
}




