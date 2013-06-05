#
#    Alchemy, a metabolic pathway generator 
#    Copyright (C) 2013  Robin Betz
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
CC = g++
CFLAGS= -O3 -Wall -std=c++11
LINKS= -lmysqlcppconn -lgvc -lgraph -lcdt

HEADERS =  Database.h Reaction.h RawReaction.h Graph.h Molecule.h MoleculeSet.h
SOURCE = Alchemy.cpp Database.cpp Reaction.cpp RawReaction.cpp Graph.cpp Molecule.cpp \
				 MoleculeSet.cpp
OBJECTS = Alchemy.o Database.o Reaction.o RawReaction.o Graph.o Molecule.o MoleculeSet.o

all: alchemy

alchemy: $(HEADERS) $(OBJECTS)
	$(CC) -o alchemy $(OBJECTS) $(LINKS)
#	@(./alchemy)

Database.o: Database.cpp
	$(CC) $(CFLAGS) -c Database.cpp

Reaction.o: Reaction.cpp Molecule.o
	$(CC) $(CFLAGS) -c Reaction.cpp

RawReaction.o: RawReaction.cpp
	$(CC) $(CFLAGS) -c RawReaction.cpp

Graph.o: Graph.cpp Database.o Reaction.o
	$(CC) $(CFLAGS) -c Graph.cpp

Molecule.o: Molecule.cpp
	$(CC) $(CFLAGS) -c Molecule.cpp

MoleculeSet.o: MoleculeSet.cpp Molecule.o
	$(CC) $(CFLAGS) -c MoleculeSet.cpp

Alchemy.o: Alchemy.cpp Graph.o
	$(CC) $(CFLAGS) -c Alchemy.cpp

clean:
	/bin/rm -f $(OBJECTS) alchemy

