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
#ifndef DATABASE_H
#define DATABASE_H

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

class Reaction;
class RawReaction;
class Molecule;
class Database
{
  sql::Driver *driver;
  sql::Connection *con;
  string schema;

public:
  Database(const string &schem="alchemy");
  vector<RawReaction*> getRawReactions();
  vector<Molecule*> getMolecules();
  void executeInsertQuery( string cmd );
  sql::ResultSet* getReactions();
  string getEnzymeName(const string id);

private:
  void handleError(const sql::SQLException &e);
  void setSchema(const string &sch);
  sql::Driver* getDriver();
  sql::Connection* getConnection();
  void addProcessedReaction( RawReaction* rxn ); // deprecated
};

#endif

