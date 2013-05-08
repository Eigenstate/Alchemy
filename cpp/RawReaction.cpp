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
#include "RawReaction.h"
using namespace std;

RawReaction::RawReaction(const string &sub, const string &prod, const string &en, const int &i, const int &p, const bool &m)
      : substrate(sub), 
        product(prod), 
        enzyme(en), 
        id(i), 
        partner(p),
        merged(m)
        {}

void RawReaction::print()
{
  cout << substrate << " -> " << product << " by " << enzyme << endl;
}

string RawReaction::getProduct()
{ return product; }

string RawReaction::getEnzyme()
{ return enzyme; }

string RawReaction::getSubstrate()
{ return substrate; }

int RawReaction::getPartner()
{ return partner; }

int RawReaction::getID()
{ return id; }

bool RawReaction::isMerged()
{ return merged; }

list<RawReaction*> RawReaction::queryBack(vector<RawReaction*>* l)
{
  list<RawReaction*> result;
  for (unsigned int i=0; i<l->size(); ++i) {
    if (getID() == l->at(i)->getPartner())
      result.push_back(l->at(i));
  }
  return result;
}

list<RawReaction*> RawReaction::queryForward(vector<RawReaction*>* l)
{
  list<RawReaction*> result;
  for (unsigned int i=0; i<l->size(); ++i) {
    if (l->at(i)->getID() == getPartner())
      result.push_back(l->at(i));
  }
  return result;
}

RawReaction::RawReaction(list<RawReaction*> merge)
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
    for (list<RawReaction*>::iterator it=merge.begin(); it!=merge.end(); ++it) {
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








