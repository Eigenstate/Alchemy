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
CFLAGS= -O3 -Wall -ggdb
LINKS= -lmysqlcppconn

HEADERS = Database.h Reaction.h
SOURCE = Alchemy.cpp Database.cpp Reaction.cpp
OBJECTS = Alchemy.o Database.o Reaction.o

all: alchemy

alchemy:	$(OBJECTS)
	$(CC) -o alchemy $(OBJECTS) $(LINKS)
	@(./alchemy)

clean:
	/bin/rm -f $(OBJECTS) alchemy

