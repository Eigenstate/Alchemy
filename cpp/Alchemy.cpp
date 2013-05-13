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
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include "Modes.h"
#include "Graph.h"
#include "Reaction.h"

using namespace std;

int main (int argc, char **argv)
{
  if (argc != 4) { // program name + 3
    cout << "Improper number of arguments!\n";
    cout << "You gave " << argc << endl;
    exit(1);
  }
  graph_mode_t mode = (graph_mode_t)atoi( argv[1] );
  Graph *g = new Graph( mode );
  vector<Reaction*> res = g->shortestPath(string(argv[2]),string(argv[3]));
  g->draw(&res, "output.png");
  exit(0);
}
