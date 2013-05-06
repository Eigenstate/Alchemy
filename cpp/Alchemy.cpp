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
#include "Database.h"
using namespace std;

int main (int argc, char **argv)
{
  Database *db = new Database();
  vector<string> ans = db->Query(string("SELECT * FROM molecules AS _message WHERE name='ethanol';"), string("_message"));
  cout << ans.size() << " strings found\n";
  cout << "First = " << ans[1] << endl;
  exit(0);
}
