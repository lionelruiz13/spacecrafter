/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014 Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

/* This class manage the low level execution for spacecrafter script
 *
 */

#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include <vector>
#include <string>

//management of the lines of code of a script
class Token {
public:
	//s: ligne of script_file p: path of script_file
	Token(const std::string &s, const std::string &p);
	~Token();
	void printToken();
	Token * pNext= nullptr;

	std::string getToken() const {
		return elmt;
	}

	std::string getTokenPath() const {
		return path;
	}

private:
	std::string elmt;
	std::string path;
};

//complete management of scripts
class Script {
public:
	Script();
	~Script();

	//! displays the script content
	void printScript();

	//! loads a script file into memory
	int load(const std::string &script_file, const std::string &script_path );

	//! empties the stack of script instructions in memory
	void clean();

	//!returns an item from the list with its path
	int getFirst(std::string &command, std::string &dataDir);

	//! adds the given Token in first position in the command queue
	void addFirstInQueue(Token * token);

private:

	typedef enum {
		first,
		second
	} ListPosition;

	Token *pFirst=nullptr;
	Token *pLast=nullptr;
	Token *p2First=nullptr;
	Token *p2Last=nullptr;
	int loadInternal(const std::string &script_file , const std::string &script_path , ListPosition wp);

	/* 
	 * adds the given Token in the queue named pFirst
	 * /!\ the value is inserted at the end of the queue
	 * if you want to insert at the begining of the queue use addFirst_in_queue
	 */
	void addFirst(Token *token);
	
	//! adds the given Token int the queue named pSecond
	void addSecond(Token *token);

	std::ifstream *input_file=nullptr;
	std::string the_path;
};

#endif
