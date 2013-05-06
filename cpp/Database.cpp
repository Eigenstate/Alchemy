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

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include <string>
#include "Database.h"
#include "Reaction.h"
using namespace std;

Database::Database(const string &schem)
  : schema(schem)
{
  try {
   driver = get_driver_instance();
   con = driver->connect("tcp://127.0.0.1:3306", "robin", "testpw");
   con->setSchema(schem.c_str());
  } catch (sql::SQLException &e) {
    cout << "Database initialization error\n";
    handleError(e);
  }
}

sql::Driver* Database::getDriver()
{ return this->driver; }

sql::Connection* Database::getConnection()
{ return this->con; }

void Database::setSchema(const string &sch)
{
  try { con->setSchema(sch.c_str()); }
  catch (sql::SQLException &e) {
    cout << "Schema setting error\n";
    handleError(e);
  }
}

void Database::handleError(const sql::SQLException &e)
{
  cout << "ERROR: SQL Exception!" << endl;
  cout << "Error code: " << e.getErrorCode() << endl;
  cout << "SQLState: " << e.getSQLState() << endl;
  exit(1);
}

vector<Reaction> Database::getReaction(const string &q, const string &id)
{
  vector<Reaction> result;
  try {
    sql::Statement *stmt = getConnection()->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM molecules;");
    while (res->next()) {
      result.push_back( res->getString("name") );
    }
    delete res;
    delete stmt;
  } catch (sql::SQLException &e) {
    cout << "Error querying database\n";
    handleError(e);
  }
  return result;
}
