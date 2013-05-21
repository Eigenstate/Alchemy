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
#include "MoleculeSet.h"
#include "Graph.h"

using namespace std;

Graph::Graph(const graph_mode_t m)
  : mode(m)
{
  // Read in database info
  db = new Database();
  mols = db->getMolecules();
  rxns = db->getReactions();
  molecules = new MoleculeSet();
  for (unsigned int i=0; i<mols.size(); ++i)
    molecules->insertMolecule( mols[i] );
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
  for (unsigned int i=0; i<currsize; ++i) { // can't parallelize since map is not thread safe
    // Create edge for this reaction
    Molecule *m = molecules->getMolecule( rxns[i]->getSubstrate() );
    if (!edges.count(m))
      edges[m] = new set<Molecule*>();
    edges[m]->insert(molecules->getMolecule( rxns[i]->getProduct() ));

    Molecule *pm = molecules->getMolecule( rxns[i]->getProduct() );
    if (!edges.count(pm))
      edges[pm] = new set<Molecule*>();

    // Extract each substrate and product separately
    vector<string> substrates, products;
    string ps = rxns[i]->getSubstrate();
    if (ps.find("(n+1)")!=string::npos) ps = ps.erase(ps.find("(n+1)"),5);
    stringstream *ss = new stringstream( ps );
    string item;
    while (getline(*ss, item, '+')) {
      // Remove numbers, like "5 H2O" should just be "H2O"
      if (item.find(" ") != string::npos)
        item = item.substr(item.find(" ")+1);
      substrates.push_back(item);
    }
    delete ss;
    ps = rxns[i]->getProduct();
    if (ps.find("(n+1)")!=string::npos) ps = ps.erase(ps.find("(n+1)"),5);
    ss = new stringstream( ps );
    while (getline(*ss, item, '+'))
      if (item.find(" ") != string::npos)
        item = item.substr(item.find(" ")+1);
      products.push_back(item);
    
    // Create dummy reaction substrate -> substrates and products -> product
    // Reactions still required for graph reassembly
    // Create appropriate edges as well
    if (substrates.size() > 1) {
      for (unsigned int j=0; j<substrates.size(); ++j) {
        rxns.push_back( new Reaction(substrates[j], rxns[i]->getSubstrate(), "0", "0", true) );
        Molecule *sm = molecules->getMolecule( substrates[j] );
        if (!edges.count(sm))
          edges[sm] = new set<Molecule*>();
        edges[sm]->insert(molecules->getMolecule( rxns[i]->getSubstrate() ));
    } }
    if (products.size() > 1) {
      for (unsigned int j=0; j<products.size(); ++j) {
        rxns.push_back( new Reaction(rxns[i]->getProduct(), products[j], "0", "0", true) );
        edges[pm]->insert(molecules->getMolecule( products[j] ));
    } }
  }
}

vector<Reaction*> Graph::shortestPath(const string &s, const string &e)
{
  setStart(s);
  setEnd(e);
  // Initialize distance and previous array
  // This is because molecule IDs dont start from zero
  vector<Molecule*> Queue = mols;
  Molecule* u = Queue[0];
  for (unsigned int i=0; i<Queue.size(); ++i) {
    Queue[i]->setPrevious(NULL);
    if (Queue[i]->getMolID() == getStart()) {
      if (getMode() == FEWEST_NODES)
        Queue[i]->setDistance(Queue[i]->getCost());
      else if (getMode() == FEWEST_EDGES)
        Queue[i]->setDistance(0);
    } else
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
    if (u->getDistance() == INT_MAX) {
      cout << "ERROR: No path found\n";
      exit(1);
    }
    if (u->getMolID() == getEnd())
      break;

    // Update neighbors
    set<Molecule*>* V = getNeighbors(u);
    if (V != NULL) {
      for (set<Molecule*>::iterator it=V->begin(); it!=V->end(); ++it) {
        int alt = 1;
        if (getMode() == FEWEST_NODES)
          alt = u->getDistance() + (*it)->getCost();
        else if (getMode() == FEWEST_EDGES)
          alt = u->getDistance() + 1; // dist (u,v) = 1 for now (edge cost)
       if (alt < (*it)->getDistance()) {
         (*it)->setDistance(alt);
          (*it)->setPrevious(u);
        }
    } }
  }
  // Reassemble path
  Molecule *p = u;
  vector<Reaction*> result;
  while (p->getPrevious() != NULL) {
    Molecule *s = p->getPrevious();
    if (s->isDummy()) {
      vector<string> parents;
      stringstream *ss = new stringstream( s->getName() ); 
      string item;
      while (getline(*ss, item, '+'))
        if (item != getStart() && item != getEnd()) {
          if ( s->getName().find(getEnd()) != string::npos )
            result.push_back( new Reaction( s->getName(), item, "0", "0", true) );
          else
            result.push_back( new Reaction( item, s->getName(), "0", "0", true) );
        }
      delete ss;
    }
    result.push_back(getReaction(s,p));
    p = s;
  }
  return result;
}

// Returns products of all reactions with u as substrate
set<Molecule*>* Graph::getNeighbors(Molecule *u)
{
  /* This is slow and awful. Fixed now hopefully.
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
  */
  /* DEBUG PRINTING 
  cout << " NEighbors of " << u->getName() << " or " << u->getMolID() << endl;
  for (set<Molecule*>::iterator it=edges[u]->begin(); it!=edges[u]->end(); ++it) {
    if ((*it)->isDummy())
      cout << (*it)->getName() << endl;
    else
      cout << (*it)->getMolID() << endl;
  }
  exit(0);
  */
  return edges[u];
}

Reaction* Graph::getReaction(Molecule *sub, Molecule *prod)
{
  // Define search terms based on if this is a dummy reaction
  string ssubs, sprods;
  if (sub->isDummy()) ssubs = sub->getName();
  else ssubs = sub->getMolID();
  if (prod->isDummy()) sprods = prod->getName();
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
    int pc=0;
    for (unsigned int j=0; j<res->at(i)->getSubstrate().size(); ++j)
      if (res->at(i)->getSubstrate()[j]=='+') ++pc;
    if (pc>1 || (pc==1 && res->at(i)->getSubstrate().find("(n+1")==string::npos) )
      result += "\"" + res->at(i)->getSubstrate() + "\" [fillcolor=\"honeydew4\" label=\"\"];\n";
    else {
      if (res->at(i)->getSubstrate()==getStart())
        result += "\"" + res->at(i)->getSubstrate() + "\" [fillcolor=\"chartreuse3\" label=\"" + getMolName(res->at(i)->getSubstrate()) + "\"];\n";
      else
        result += "\"" + res->at(i)->getSubstrate() + "\" [fillcolor=\"white\" label=\"" + getMolName(res->at(i)->getSubstrate()) + "\"];\n";
    }
    pc=0;
    for (unsigned int j=0; j<res->at(i)->getProduct().size(); ++j)
      if (res->at(i)->getProduct()[j]=='+') ++pc;
    if(pc>1 || (pc==1 &&res->at(i)->getProduct().find("(n+1)")==string::npos) )
      result += "\"" + res->at(i)->getProduct() + "\" [fillcolor=\"honeydew4\" label=\"\"];\n";
    else {
      if (res->at(i)->getProduct()==getEnd())
        result += "\"" + res->at(i)->getProduct() + "\" [fillcolor=\"coral2\" label=\"" + getMolName(res->at(i)->getProduct()) + "\"];\n";
      else
        result += "\"" + res->at(i)->getProduct() + "\" [fillcolor=\"white\" label=\"" + getMolName(res->at(i)->getProduct()) + "\"];\n";
    }
    
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

graph_mode_t Graph::getMode() { return mode; }
const string Graph::getStart() { return start; }
const string Graph::getEnd() { return end; }
void Graph::setStart(const string &s) { start = s; }
void Graph::setEnd(const string &e) { end = e; }








