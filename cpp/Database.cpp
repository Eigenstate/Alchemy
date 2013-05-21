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
#include <map>
#include "Database.h"
#include "Reaction.h"
#include "RawReaction.h"
#include "Molecule.h"
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

// Deprecated
vector<RawReaction*> Database::getRawReactions()
{
  vector<RawReaction*> rawresults;
  try {
    sql::Statement *stmt = getConnection()->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT rxn_id,substrate,product,enzyme,partner_rxn FROM raw_reactions;");
    while (res->next()) {
    int rid = atoi(res->getString("rxn_id").c_str());
    string sub = res->getString("substrate");
    string pro = res->getString("product");
    string enz = res->getString("enzyme");
    int pid = atoi(res->getString("partner_rxn").c_str());
      rawresults.push_back( new RawReaction(sub, pro, enz, rid, pid) );
    }
    delete res;
    delete stmt;
  } catch (sql::SQLException &e) {
    cout << "Error querying database\n";
    handleError(e);
  }

  // Find partner reactions
  vector<RawReaction*> procresults;
  while ( rawresults.size() ) {
    RawReaction *rxn = rawresults.back();
    rawresults.pop_back();
    list<RawReaction*> tomerge;

    // Query backwards
    list<RawReaction*> back;
    back.push_back(rxn);
    while ( back.size() ) {
      RawReaction *r = back.front();
      back.pop_front();
      list<RawReaction*> q = r->queryBack(&rawresults);
      back.merge( q );
      tomerge.push_back(r);
    }

    // Query forwards
    list<RawReaction*> forw;
    forw.push_back(rxn);
    while ( forw.size() ) {
      RawReaction *r = forw.front();
      forw.pop_front();
      list<RawReaction*> q = r->queryForward(&rawresults);
      forw.merge( q );
      if (r != rxn) 
        tomerge.push_back(r);
    }
    RawReaction *n = new RawReaction(tomerge);
    procresults.push_back(n);
  }
  return procresults;
}

// Deprecated
void Database::addProcessedReaction( RawReaction* rxn )
{
  cout << "You are trying to add rxn to database... NO!";
  return; // DO not run this
  string command = "INSERT IGNORE INTO reactions(enzyme,substrate,product,organism) VALUES ('";
  command += rxn->getEnzyme() + "','" + rxn->getSubstrate() + "','" + rxn->getProduct() + "','homo sapiens');";
  try {
    sql::Statement *stmt = getConnection()->createStatement();
    stmt->execute(command.c_str());
    delete stmt;
  } catch (sql::SQLException &e) {
    cout << "Error inserting reaction into database\n";
    cout << "Reaction was: "; rxn->print();
    handleError(e);
  }
}

void Database::executeInsertQuery( string cmd )
{
  try {
    sql::Statement *stmt = getConnection()->createStatement();
    stmt->execute(cmd.c_str());
    delete stmt;
  } catch (sql::SQLException &e) {
    cout << "Error executing insert query. Query was:\n" << cmd << endl;
    handleError(e);
  }
}

vector<Reaction*> Database::getReactions()
{
  vector<Reaction*> results;
  try {
    sql::Statement *stmt = getConnection()->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT enzyme,substrate,product,organism FROM reactions;");
    while (res->next()) {
    string sub = res->getString("substrate");
    string pro = res->getString("product");
    string enz = res->getString("enzyme");
    string org = res->getString("organism");
      results.push_back( new Reaction(sub, pro, enz, org, false) );
    }
    delete res;
    delete stmt;
  } catch (sql::SQLException &e) {
    cout << "Error querying database for reactions\n";
    handleError(e);
  }
  return results;
}

vector<Molecule*> Database::getMolecules()
{
  vector<Molecule*> molecules;
  try {
    sql::Statement *stmt = getConnection()->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT kegg_id,name FROM molecules;");
    while (res->next()) {
      molecules.push_back( new Molecule( res->getString("kegg_id"), res->getString("name")) );
    }
    delete res;
    delete stmt;
  } catch (sql::SQLException &e) {
    cout << "Error querying database for molecules\n";
    handleError(e);
  }
  return molecules;
}
