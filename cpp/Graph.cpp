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

  mols = db->getMolecules();
  molecules = new MoleculeSet();
  for (unsigned int i=0; i<mols.size(); ++i)
    molecules->insertMolecule( mols[i] );

  // Read in reaction data
  populateReactions();
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
    if (getReaction(m, rxns[i]->getProduct())->isDummy()) {
      if (!dummyedges.count(m))
        dummyedges[m] = new set<Molecule*>();
      dummyedges[m]->insert(rxns[i]->getProduct());
    } else {
      if (!rxnedges.count(m)) 
        rxnedges[m] = new set<Molecule*>();
      rxnedges[m]->insert(rxns[i]->getProduct());
    }

    // Create dummy INEDGES to the reaction nodes
    for (unsigned int j=0; j<m->getMolIDs().size(); ++j) {
      Molecule *n = molecules->getMolecule( m->getMolIDs().at(j) );
      if (!dummyedges.count(n)) {
        dummyedges[n] = new set<Molecule*>();
      }
      dummyedges[n]->insert(m);
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
  vector<Molecule*> Queue = mols;
  Molecule* u = Queue[0];
   Molecule *endm=molecules->getMolecule(getEnd());
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
    if (u->search(getEnd()) && !getReaction(u->getPrevious(),u)->isDummy()) { // final reaction can't be a dummy
      cout << "DISTANCE SCORE: " << u->getDistance() << endl;
      // Add final reaction
      rxns.push_back( new Reaction(u,endm,"0","0",true) );
      endm->setPrevious(u); u=endm;
      break;
    }

    // Update neighbors
    set<Molecule*>* V;
    V = getNeighbors(V,u);
    if (V != NULL) {
      for (set<Molecule*>::iterator it=V->begin(); it!=V->end(); ++it) {
        int alt = 1;
        if (getMode() == FEWEST_NODES)
          alt = u->getDistance() + (*it)->getCost();
        else if (getMode() == FEWEST_EDGES) {
          if (u->isDummy()!=(*it)->isDummy()) // free transition costs 10
            alt = u->getDistance() + 10;
          else
            alt = u->getDistance() + 1; // dist (u,v) = 1 for now (edge cost)
        }
       if (alt < (*it)->getDistance()) {
         (*it)->setDistance(alt);
          (*it)->setPrevious(u);
        }
    } }
    delete V; V=NULL;
  }
  // Reassemble path
  Molecule *p = u;
  vector<Reaction*> result;
  while (p->getPrevious() != NULL) {
    Molecule *s = p->getPrevious();
    if (s->isDummy()) {
     vector<string> parents;
     for (unsigned int k=0; k<s->getMolIDs().size(); ++k)
       if (s->getMolIDs().at(k) != getStart() && s->getMolIDs().at(k) != getEnd()){ // don't draw extra line to start or from end
        if (s->search(getEnd()))
          result.push_back(new Reaction(s, molecules->getMolecule(s->getMolIDs().at(k)), "0","0", true));
        else 
          result.push_back(new Reaction(molecules->getMolecule(s->getMolIDs().at(k)), s, "0","0", true));
       //if (strcmp(s->getMolIDs().at(k).c_str(),getStart().c_str()) && s->search(getEnd())) // Contains end but not start
      }
    }
    result.push_back(getReaction(s,p));
    p = s;
  }
  // Print path
  for (unsigned int i=result.size()-1; i>0; --i) {
    if (result[i]->isDummy()) continue;
    if (result[i]->getSubstrate()->isDummy() || result[i]->getProduct()->isDummy())
      cout << molecules->generateName(result[i]->getSubstrate()->generateID()) << "->" <<
              molecules->generateName(result[i]->getProduct()->generateID()) << " by " << result[i]->getEnzyme() << endl;
  }
  return result;
}

// Returns products of all reactions with u as substrate
// If previous reaction was a dummy, only return non-dummy edges
// Don't forget to delete memory allocated for V
set<Molecule*>* Graph::getNeighbors(set<Molecule*>* V, Molecule *u)
{
  if (rxnedges.count(u))
    V = new set<Molecule*>(*(rxnedges[u]));
  else
    V = new set<Molecule*>();

  if (dummyedges.count(u) && 
      (!u->getPrevious() || !getReaction(u->getPrevious(), u)->isDummy()) )
    V->insert(dummyedges[u]->begin(),dummyedges[u]->end());
  return V;
}

Reaction* Graph::getReaction(Molecule *sub, Molecule *prod)
{
  // Search
  for (unsigned int i=0; i<rxns.size(); ++i) {
    if (rxns[i]->getSubstrate() == sub && rxns[i]->getProduct() == prod)
      return rxns[i];
  }
  cout << "ERROR: No reaction found." << endl;
  cout << "Substrate = " << sub->generateID() << " Product = " << prod->generateID() << endl;
  exit(1);
  return NULL;
}

string Graph::getGraphviz(vector<Reaction*>* res)
{
  string result = "digraph test {\n";
  result += "node [style=filled];\n";
  string sub, prod, enz;
  map<string,string> orgColors;

  // Print out reaction URLs to screen
  cout << "\nENZYMES NEEDED:\n";
  for (unsigned int i=0; i<res->size(); ++i) {
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
    if (s->generateID()==getStart())
      result += "\"" + s->generateID() + "\" [fillcolor=\"chartreuse3\" label=\"" + s->getName() + "\"];\n";
    else {
      result += "\"" + s->generateID() + "\" ";
      if (s->isDummy())
        result += "[fillcolor=\"honeydew4\" label=\"" + p->generateID() + "\"];\n";
        //result += "[fillcolor=\"honeydew4\" label=\"\"];\n";
      else
        result += "[fillcolor=\"white\" label=\"" + s->getName() + "\"];\n";
    }
    if (p->generateID()==getEnd())
      result += "\"" + p->generateID() + "\" [fillcolor=\"coral2\" label=\"" + p->getName() + "\"];\n";
    else {
      result += "\"" + p->generateID() + "\" ";
      if (p->isDummy())
        result += "[fillcolor=\"honeydew4\" label=\"" + p->generateID() + "\"];\n";
        //result += "[fillcolor=\"honeydew4\" label=\"\"];\n";
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

  // Draw the enzyme legend as a subgraph with HTML markup
  result += "subgraph cl2{\nlabel=\"Enzymes\";\nekey [label=<<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" cellborder=\"1\">\n";
  for (int j=res->size()-1; j>=0; --j) {
    if (res->at(j)->isDummy()) continue;
    result += "<tr><td align=\"center\">"+ res->at(j)->getEnzyme() +"</td>";
    result += "<td align=\"center\">"+db->getEnzymeName(res->at(j)->getEnzyme())+"</td></tr>\n";
  }
  result += "</table>>]\n}\n"; // end enzyme legend subgraph

  result += "}\n"; // end graphviz file
  return result;
}

void Graph::render(string temp, const char *filename)
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








