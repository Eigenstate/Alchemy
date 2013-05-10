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
#include <vector>
#include <map>
#include <omp.h>
#include "Database.h"
#include "Reaction.h"
#include "Graph.h"
using namespace std;

Graph::Graph()
{
  db = new Database();
  mols = db->getMolecules();
  rxns = db->getReactions();
  createDummyReactions();
}

Graph::~Graph()
{
  delete db;
  for (unsigned int i=0; i<rxns.size(); ++i) {
    delete rxns.at(i);
  }
}

const string Graph::getMoleculebyID(int id)
{ return mols[id]; }

const vector<Reaction*>* Graph::getReactions()
{ return &rxns; }

void Graph::createDummyReactions()
{
  for (unsigned int i=0; i<rxns.size(); ++i) {
    // Extract each substrate and product separately
    vector<string> substrates, products;
    stringstream *ss = new stringstream( rxns[i]->getSubstrate() );
    string item;
    while (getline(*ss, item, '+')) {
      substrates.push_back(item);
    }
    delete ss;
    ss = new stringstream( rxns[i]->getProduct() );
    while (getline(*ss, item, '+'))
      products.push_back(item);
    
    // Create dummy reaction substrate -> substrates and products -> product
    for (unsigned int j=0; j<substrates.size(); ++j)
      rxns.push_back( new Reaction(substrates[j], rxns[i]->getSubstrate(), "0", "0", true) );
    for (unsigned int j=0; j<products.size(); ++j)
      rxns.push_back( new Reaction(rxns[i]->getProduct(), products[j], "0", "0", true) );
  }
}









