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
#include "Molecule.h"
using namespace std;

Reaction::Reaction(Molecule *sub, Molecule *prod, const string& en, const string &org, const bool &d)
      : substrate(sub), 
        product(prod), 
        enzyme(en), 
        organism(org),
        dummy(d)
        {}

void Reaction::print()
{
  cout << substrate << " -> " << product << " by " << enzyme << endl;
}

Molecule* Reaction::getProduct()
{ return product; }

string Reaction::getEnzyme()
{ return enzyme; }

Molecule* Reaction::getSubstrate()
{ return substrate; }

string Reaction::getOrganism()
{ return organism; }

bool Reaction::isDummy()
{ return dummy; }
