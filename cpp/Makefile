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
CFLAGS= -O3 -Wall -fopenmp -ggdb
LINKS= -lmysqlcppconn

HEADERS =  Database.h Reaction.h RawReaction.h Graph.h
SOURCE = Alchemy.cpp Database.cpp Reaction.cpp RawReaction.cpp Graph.cpp
OBJECTS = Alchemy.o Database.o Reaction.o RawReaction.o Graph.o

all: alchemy

alchemy: $(OBJECTS)
	$(CC) -o alchemy $(OBJECTS) $(LINKS)
#	@(./alchemy)

Database.o: $(HEADERS)
	$(CC) $(CFLAGS) -c Database.cpp

Reaction.o: $(HEADERS)
	$(CC) $(CFLAGS) -c Reaction.cpp

RawReaction.o: $(HEADERS)
	$(CC) $(CFLAGS) -c RawReaction.cpp

Graph.o: $(HEADERS)
	$(CC) $(CFLAGS) -c Graph.cpp

Alchemy.o: $(HEADERS)
	$(CC) $(CFLAGS) -c Alchemy.cpp

clean:
	/bin/rm -f $(OBJECTS) alchemy

