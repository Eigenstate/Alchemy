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
#include <cstring>
#include <cstdio>
#include <sstream>
#include <vector>
#include <climits>
#include <fstream>
#include <graphviz/gvc.h>
#include "Database.h"
#include "Reaction.h"
#include "Molecule.h"
#include "Graph.h"
using namespace std;

Graph::Graph()
{
  // Read in database info
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

const vector<Reaction*>* Graph::getReactions()
{ return &rxns; }

void Graph::createDummyReactions()
{
  unsigned int currsize = rxns.size(); // needed since we're adding more to rxns
  for (unsigned int i=0; i<currsize; ++i) {
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
    if (substrates.size() > 1)
      for (unsigned int j=0; j<substrates.size(); ++j)
        rxns.push_back( new Reaction(substrates[j], rxns[i]->getSubstrate(), "0", "0", true) );
    if (products.size() > 1)
      for (unsigned int j=0; j<products.size(); ++j)
        rxns.push_back( new Reaction(rxns[i]->getProduct(), products[j], "0", "0", true) );
    }
}

vector<Reaction*> Graph::shortestPath(const string &start, const string &end)
{
  // Initialize distance and previous array
  // This is because molecule IDs dont start from zero
  vector<Molecule*> Queue = mols;
  Molecule* u = Queue[0];
  for (unsigned int i=0; i<Queue.size(); ++i) {
    Queue[i]->setPrevious(NULL);
    if (Queue[i]->getMolID() == start)
      Queue[i]->setDistance(0);
    else
      Queue[i]->setDistance(INT_MAX);
  }
 
  while (Queue.size()) {
    // Find smallest distance in queue and remove it
    int val = INT_MAX; int loc = 0;
    unsigned int i;
    for (i=0; i<Queue.size(); ++i) {
      if (Queue[i]->getDistance() < val) {
        val = Queue[i]->getDistance();
        loc = i;
      }
    }
    u = Queue[loc];
    Queue.erase(Queue.begin()+loc);

    // Check for solved condition
    cout << "SMALLEST = " << u->getMolID() << " dist = " << u->getDistance() << endl;
    if (u->getDistance() == INT_MAX) {
      cout << "ERROR: No path found\n";
      exit(1);
    }
    if (u->getMolID() == end) {
      cout << "Success! Reassembling path\n";
      cout << u->getMolID() << " = " << end << endl;
      break;
    }

    // Update neighbors
    vector<Molecule*> V = getNeighbors(u);
    for (unsigned int j=0; j<V.size(); ++j) {
      int alt = u->getDistance() + 1; // dist (u,v) = 1 for now
      if (alt < V[j]->getDistance()) { 
        V[j]->setDistance(alt);
        V[j]->setPrevious(u);
      }
    }
  }
  // Reassemble path
  Molecule *p = u;
  vector<Reaction*> result;
  while (p->getPrevious() != NULL) {
    Molecule *s = p->getPrevious();
    if (s->getStructureID() == "DUMMY") {
      vector<string> parents;
      stringstream *ss = new stringstream( s->getName() ); 
      string item;
      while (getline(*ss, item, '+'))
        result.push_back( new Reaction( item, s->getName(), "0", "0", true) );
      delete ss;
    }
    result.push_back(getReaction(s,p));
    p = s;
  }
  return result;
}

// Returns products of all reactions with u as substrate
vector<Molecule*> Graph::getNeighbors(Molecule *u)
{
  vector<Molecule*> result;
  string search;
  if (u->getStructureID() == "DUMMY") search = u->getName();
  else search = u->getMolID();

  for (unsigned int i=0; i<rxns.size(); ++i) {
    if (rxns[i]->getSubstrate()== search ) {
      string p = rxns[i]->getProduct();
      string search;
      for (unsigned int j=0; j<mols.size(); ++j) {
        if (mols[j]->getMolID() == p || mols[j]->getName() == p) {
          result.push_back(mols[j]);
          break;
        }
      }
    }
  }
  return result;
}

Reaction* Graph::getReaction(Molecule *sub, Molecule *prod)
{
  // Define search terms based on if this is a dummy reaction
  string ssubs, sprods;
  if (sub->getStructureID() == "DUMMY") ssubs = sub->getName();
  else ssubs = sub->getMolID();
  if (prod->getStructureID() == "DUMMY") sprods = prod->getName();
  else sprods = prod->getMolID();

  // Search
  for (unsigned int i=0; i<rxns.size(); ++i) {
    if (rxns[i]->getSubstrate() == ssubs && rxns[i]->getProduct() == sprods)
      return rxns[i];
  }
  return NULL;
}

const char* Graph::getGraphviz(vector<Reaction*>* res)
{
  string result = "digraph test {\n";
  result += "node [style=filled];\n";
  string sub, prod, enz;

  for (unsigned int i=0; i<res->size(); ++i) {
    if (res->at(i)->getSubstrate().find('+') != string::npos)
      result += "\"" + res->at(i)->getSubstrate() + "\" [fillcolor=\"honeydew4\" label=\"\"];\n";
    else
      result += "\"" + res->at(i)->getSubstrate() + "\" [fillcolor=\"white\" label=\"" + getMolName(res->at(i)->getSubstrate()) + "\"];\n";

    if (res->at(i)->getProduct().find('+') != string::npos)
      result += "\"" + res->at(i)->getProduct() + "\" [fillcolor=\"honeydew4\" label=\"\"];\n";
    else
      result += "\"" + res->at(i)->getProduct() + "\" [fillcolor=\"white\" label=\"" + getMolName(res->at(i)->getProduct()) + "\"];\n";
    
    if (res->at(i)->isDummy()) enz = "";
    else enz = res->at(i)->getEnzyme();
    result += "\"" + res->at(i)->getSubstrate() + "\" -> \"" + res->at(i)->getProduct() + "\" [label=\"" + enz + "\"];\n";
  }
  result += "}\n";
  return result.c_str();
}

string Graph::getMolName(const string nid)
{
  for (unsigned int i=0; i<mols.size(); ++i) {
    if (mols[i]->getMolID() == nid) return mols[i]->getName();
  }
  return "";
}

void Graph::render(string &temp, const char *filename)
{
  GVC_t *gvc = gvContext();
  Agraph_t *G = agmemread(const_cast<char*>(temp.c_str()));
  gvLayout(gvc, G, "dot");
  gvRenderFilename(gvc, G, "png", filename);
}

void Graph::draw(vector<Reaction*>* res, const char *filename)
{
  string temp = getGraphviz(res);
  render(temp, filename);
}








