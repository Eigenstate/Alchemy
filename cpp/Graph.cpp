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
#include <cppconn/resultset.h>
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
   db = new Database();

  cout << "Reading in molecules\n";
  mols = db->getMolecules();
  molecules = new MoleculeSet();
  for (unsigned int i=0; i<mols.size(); ++i)
    molecules->insertMolecule( mols[i] );

  // Read in reaction data
  cout << "Populating reactions\n";
  populateReactions();
  cout << "Creating dummy reactions\n";
  createDummyReactions();

  // Construct list of available colors
  Colors.push_back("cornflowerblue");
  Colors.push_back("darkorchid1");
  Colors.push_back("goldenrod4");
  Colors.push_back("darkseagreen3");
  Colors.push_back("dodgerblue4");
  Colors.push_back("firebrick4");
  Colors.push_back("darkslategray");
}

Graph::~Graph()
{
  delete db;
  for (unsigned int i=0; i<rxns.size(); ++i) {
    delete rxns.at(i);
  }
}

void Graph::populateReactions()
{
  sql::ResultSet *res = db->getReactions();
  while (res->next()) {
    Molecule *sub = molecules->getMolecule( res->getString("substrate") );
    Molecule *pro = molecules->getMolecule( res->getString("product") );
    string enz = res->getString("enzyme");
    string org = res->getString("organism");
    rxns.push_back( new Reaction(sub, pro, enz, org, false) );
  }
  delete res;
}

const vector<Reaction*>* Graph::getReactions()
{ return &rxns; }

void Graph::createDummyReactions()
{
  unsigned int currsize = rxns.size(); // needed since we're adding more to rxns
  for (unsigned int i=0; i<currsize; ++i) { // can't parallelize since map is not thread safe
    // Create edge for this reaction from substrate to product
    Molecule *m = rxns[i]->getSubstrate();
    if (!edges.count(m)) {
      edges[m] = new set<Molecule*>();
    }
    edges[m]->insert(rxns[i]->getProduct());

    // Create dummy INEDGES to the reaction nodes
    for (unsigned int j=0; j<m->getMolIDs().size(); ++j) {
      Molecule *n = molecules->getMolecule( m->getMolIDs().at(j) );
      if (!edges.count(n)) {
        edges[n] = new set<Molecule*>();
      }
      edges[n]->insert(m);
      rxns.push_back( new Reaction(n, m, "0","0", true) );
    }
    //if (!edges.count(rxns[i]->getProduct()))
      //edges[rxns[i]->getProduct()] = new set<Molecule*>();

/* This is already pre-done and the reactions are in the DB
    // Create all other edges from product to other substrates
    Molecule *pm = rxns[i]->getProduct();
    if (!edges.count(pm))
      edges[pm] = new set<Molecule*>();
    for (map<string,Molecule*>::iterator it=molecules->getBeginIterator(); it!=molecules->getEndIterator(); ++it) {
      if (it->second->isDummy() && pm->generateID()!=it->first && pm->generateID().find(it->first) != string::npos) {
        edges[pm]->insert(it->second);
        cout << "Creating dummy edge from " << pm->generateID() << " -> " << it->second->generateID() << endl;
        rxns.push_back( new Reaction(pm, it->second, "0", "0", true) );
        // Insert into database
        string cmd("INSERT IGNORE INTO reactions(substrate,product,organism) VALUES('");
        cmd += pm->generateID() + "','" + it->second->generateID() + "','0');";
        db->executeInsertQuery(cmd);
      }
    }
*/
  }
}
/*
    // Extract each substrate and product separately
    vector<Molecule*> substrates, products;
    string ps = rxns[i]->getSubstrate()->getMolID();
    if (ps.find("(n+1)")!=string::npos) ps = ps.erase(ps.find("(n+1)"),5);
    if (ps.find("(n+2)")!=string::npos) ps = ps.erase(ps.find("(n+2)"),5);
    stringstream *ss = new stringstream( ps );
    string item;
    while (getline(*ss, item, '+')) {
      // Remove numbers, like "5 H2O" should just be "H2O"
      if (item.find(" ") != string::npos)
        item = item.substr(item.find(" ")+1);
      substrates.push_back(molecules->getMolecule(item));
    }
    delete ss;
    ps = rxns[i]->getProduct()->getMolID();
    if (ps.find("(n+1)")!=string::npos) ps = ps.erase(ps.find("(n+1)"),5);
    if (ps.find("(n+2)")!=string::npos) ps = ps.erase(ps.find("(n+2)"),5);
    ss = new stringstream( ps );
    while (getline(*ss, item, '+'))
      if (item.find(" ") != string::npos)
        item = item.substr(item.find(" ")+1);
      products.push_back(molecules->getMolecule(item));
    
    // Create dummy reaction substrate -> substrates and products -> product
    // Reactions still required for graph reassembly
    // Create appropriate edges as well
    if (substrates.size() > 1) {
      for (unsigned int j=0; j<substrates.size(); ++j) {
        rxns.push_back( new Reaction(substrates[j], rxns[i]->getSubstrate(), "0", "0", true) );
        rxns.push_back( new Reaction(rxns[i]->getSubstrate(), substrates[j], "0", "0", true) );
        edges[rxns[i]->getSubstrate()]->insert(substrates[j]);
        Molecule *sm = substrates[j];
        if (!edges.count(sm))
          edges[sm] = new set<Molecule*>();
        //edges[sm]->insert(rxns[i]->getSubstrate());
    } }
    if (products.size() > 1) {
      for (unsigned int j=0; j<products.size(); ++j) {
        rxns.push_back( new Reaction(rxns[i]->getProduct(), products[j], "0", "0", true) );
        rxns.push_back( new Reaction(products[j], rxns[i]->getProduct(), "0", "0", true) );
        edges[rxns[i]->getProduct()]->insert(products[j]);
        //edges[products[j]]->insert(rxns[i]->getProduct());
    } }
  }
}
*/

vector<Reaction*> Graph::shortestPath(const string &s, const string &e)
{
  setStart(s);
  setEnd(e);
  // Initialize distance and previous array
  // This is because molecule IDs dont start from zero
  cout << "Initializing pathfinding\n";
  vector<Molecule*> Queue = mols;
  Molecule* u = Queue[0];
  for (unsigned int i=0; i<Queue.size(); ++i) {
    Queue[i]->setPrevious(NULL);
    if (Queue[i]->generateID() == getStart()) {
      if (getMode() == FEWEST_NODES)
        Queue[i]->setDistance(Queue[i]->getCost());
      else if (getMode() == FEWEST_EDGES)
        Queue[i]->setDistance(0);
    } else
      Queue[i]->setDistance(INT_MAX);
  }

  cout << "Conducting pathfinding\n"; 
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
    if (u->search(getEnd())) {
      cout << "DISTANCE: " << u->getDistance() << endl;
      break;
    }

    // Update neighbors
    set<Molecule*>* V = getNeighbors(u);
    cout << "Found " << V->size() << " neigh\n";
    if (V != NULL) {
      for (set<Molecule*>::iterator it=V->begin(); it!=V->end(); ++it) {
        int alt = 1;
        if (getMode() == FEWEST_NODES)
          alt = u->getDistance() + (*it)->getCost();
        else if (getMode() == FEWEST_EDGES) {
          if (u->isDummy()!=(*it)->isDummy()) // free transition costs 10
            alt = u->getDistance() + 1000;
          else
            alt = u->getDistance() + 1; // dist (u,v) = 1 for now (edge cost)
        }
       if (alt < (*it)->getDistance()) {
         (*it)->setDistance(alt);
          (*it)->setPrevious(u);
        }
    } }
  }
  // Reassemble path
  cout << "Reassembling path\n";
  Molecule *p = u;
  vector<Reaction*> result;
  while (p->getPrevious() != NULL) {
    Molecule *s = p->getPrevious();
    cout << "Looking for inedges of " << s->generateID() << endl;
    if (s->isDummy()) {
     vector<string> parents;
     for (unsigned int k=0; k<s->getMolIDs().size(); ++k) {
       if (strcmp(s->getMolIDs().at(k).c_str(),getStart().c_str()) && s->search(getEnd())) // Contains end but not start
         result.push_back(new Reaction(molecules->getMolecule(s->getMolIDs().at(k)), s, "0","0", true));
       if (s->search(getEnd()))
        result.push_back(new Reaction(s, molecules->getMolecule(getEnd()), "0", "0", true));
      }
    }
    result.push_back(getReaction(s,p));
    p = s;
  }
  return result;
}

// Returns products of all reactions with u as substrate
set<Molecule*>* Graph::getNeighbors(Molecule *u)
{
  return edges[u];
}

Reaction* Graph::getReaction(Molecule *sub, Molecule *prod)
{
  // Search
  for (unsigned int i=0; i<rxns.size(); ++i) {
    if (rxns[i]->getSubstrate() == sub && rxns[i]->getProduct() == prod)
      return rxns[i];
  }
  cout << "ERROR: No reaction found\n";
  cout << "Substrate = " << sub->generateID() << " Product = " << prod->generateID() << endl;
  return NULL;
}

const char* Graph::getGraphviz(vector<Reaction*>* res)
{
  string result = "digraph test {\n";
  result += "node [style=filled];\n";
  string sub, prod, enz;
  map<string,string> orgColors;

  for (unsigned int i=0; i<res->size(); ++i) {
    // Print out reaction URL to screen
    if (!res->at(i)->isDummy())
      cout << "http://rest.kegg.jp/get/" << res->at(i)->getEnzyme() << endl;

    // Get or define color for this organism
    string color;
    try { color = orgColors.at(res->at(i)->getOrganism()); }
    catch (const out_of_range &e) {
      orgColors.insert(pair<string,string>(res->at(i)->getOrganism(), Colors.back()));
      color = Colors.back();
      Colors.pop_back();
    }
    // Write out this reaction
    Molecule *s = res->at(i)->getSubstrate();
    Molecule *p = res->at(i)->getProduct();
    if (!strcmp(s->generateID().c_str(),getStart().c_str()))
      result += "\"" + s->generateID() + "\" [fillcolor=\"chartreuse3\" label=\"" + s->getName() + "\"];\n";
    else {
      result += "\"" + s->generateID() + "\" ";
      if (s->isDummy())
        result += "[fillcolor=\"honeydew4\" label=\"\"];\n";
      else
        result += "[fillcolor=\"white\" label=\"" + s->getName() + "\"];\n";
    }
    if (!strcmp(p->generateID().c_str(),getEnd().c_str()))
      result += "\"" + p->generateID() + "\" [fillcolor=\"coral2\" label=\"" + p->getName() + "\"];\n";
    else {
      result += "\"" + p->generateID() + "\" ";
      if (p->isDummy())
        result += "[fillcolor=\"honeydew4\" label=\"\"];\n";
      else
        result += "[fillcolor=\"white\" label=\"" + p->getName() + "\"];\n";
    }
    if (res->at(i)->isDummy()) enz = "";
    else enz = res->at(i)->getEnzyme();
    result += "\"" + s->generateID() + "\" -> \"" + p->generateID() + "\" [label=\"" + enz + "\" color=\"" + color + "\"];\n";
  }

  // Draw the legend as a subgraph with HTML markup (awks I know but whatever)
  result += "rankdir=LR\nnode [shape=plaintext fillcolor=\"white\"]\n";
  result += "subgraph cl1 {\nlabel=\"Legend\";\nkey [label=<<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" cellborder=\"0\">\n";
  int i=0;
  for (map<string,string>::iterator it=orgColors.begin(); it!=orgColors.end(); ++it) {
    result += "<tr><td align=\"right\" port=\"i"+to_string(i)+"\">" + it->first + "</td></tr>\n";
    ++i;
  }
  result += "</table>>]\n";
  result += "key2 [label=<<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" cellborder=\"0\">\n";
  i=0;
  for (map<string,string>::iterator it=orgColors.begin(); it!=orgColors.end(); ++it) {
    result += "<tr><td port=\"i"+to_string(i)+"\">&nbsp;</td></tr>\n"; 
    ++i;
  }
  result += "</table>>]";
  i=0;
  for (map<string,string>::iterator it=orgColors.begin(); it!=orgColors.end(); ++it) {
    result += "key:i"+to_string(i)+":e -> key2:i"+to_string(i)+":w [color=\"" + it->second + "\"]\n";
    ++i;
  }
  result += "}\n"; // end legend subgraph

  result += "}\n"; // end graphviz file
  return result.c_str();
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








