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
#include <list>
#include <stdlib.h>
#include "Reaction.h"
using namespace std;

Reaction::Reaction(const string &sub, const string &prod, const string &en, const int &i, const int &p, const bool &m)
      : substrate(sub), 
        product(prod), 
        enzyme(en), 
        id(i), 
        partner(p),
        merged(m)
        {}

void Reaction::print()
{
  cout << substrate << " -> " << product << " by " << enzyme << endl;
}

string Reaction::getProduct()
{ return product; }

string Reaction::getEnzyme()
{ return enzyme; }

string Reaction::getSubstrate()
{ return substrate; }

int Reaction::getPartner()
{ return partner; }

int Reaction::getID()
{ return id; }

bool Reaction::isMerged()
{ return merged; }

list<Reaction*> Reaction::queryBack(vector<Reaction*>* l)
{
  list<Reaction*> result;
  for (int i=0; i<l->size(); ++i) {
    if (getID() == l->at(i)->getPartner())
      result.push_back(l->at(i));
  }
  return result;
}

list<Reaction*> Reaction::queryForward(vector<Reaction*>* l)
{
  list<Reaction*> result;
  for (int i=0; i<l->size(); ++i) {
    if (l->at(i)->getID() == getPartner())
      result.push_back(l->at(i));
  }
  return result;
}

Reaction::Reaction(list<Reaction*> merge)
{
  enzyme = merge.front()->getEnzyme();
  if (merge.size() == 1) {
    substrate = merge.front()->getSubstrate();
    product = merge.front()->getProduct();
    id = merge.front()->getID();
    partner = 0;
    merged = false;
  } else {
    substrate = product = "";
    for (list<Reaction*>::iterator it=merge.begin(); it!=merge.end(); ++it) {
      if ((*it)->getSubstrate() != "NULL") substrate += (*it)->getSubstrate() + "+";
      if ((*it)->getProduct() != "NULL") product += (*it)->getProduct() + "+";
      if (enzyme != (*it)->getEnzyme()) {
        cout << "ERROR: Non-matching enzyme in merge!\n";
        exit(1);
      }
    }
    substrate.erase(substrate.length()-1); // get rid of extra +
    product.erase(product.length()-1);
    merged = true;
    id = 0;
    partner = 0;
  }
}








