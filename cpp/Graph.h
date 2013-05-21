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
#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <map> // C++11 for unordered map and set isn't worth teensy speedup IMO
#include <set>
#include "Modes.h"
using namespace std;

class Database;
class Reaction;
class Molecule;
class MoleculeSet;


class Graph
{
  Database *db;
  vector<Reaction*> rxns;
  vector<Molecule*> mols;
  MoleculeSet* molecules;
  map<Molecule*, set<Molecule*>*> edges;
  graph_mode_t mode;
  string start;
  string end;

public:
  Graph(const graph_mode_t m);
  ~Graph();
  vector<Reaction*> shortestPath(const string &s, const string &e);
  Reaction* getReaction(Molecule *sub, Molecule *prod);
  string getMolName(const string nid);
  void draw(vector<Reaction*>* res, const char *filename);
  void setStart(const string &s);
  void setEnd(const string &e);

private:
  void createDummyReactions();
  set<Molecule*>* getNeighbors(Molecule *u);
  const vector<Reaction*>* getReactions();
  const char* getGraphviz(vector<Reaction*>* res);
  void render(string &temp, const char *filename);
  graph_mode_t getMode();
  const string getStart();
  const string getEnd();
};

#endif
