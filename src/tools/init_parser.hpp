/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

// Class which parses an init file
// C++ warper for the iniparser free library from N.Devillard

#ifndef _INIT_PARSER_H_
#define _INIT_PARSER_H_

#include <string>
#include <iostream>
#include "iniparser/iniparser.h"

class InitParser {
public:
	// Create the parser object from the given file
	// You need to call load() before using the get() functions
	InitParser();
	virtual ~InitParser();

	// Load the config file (the parsing operation)
	void load(const std::string& file_name);

	// Save the current config state in the original file
	void save(const std::string& file_name) const;

	// Get a std::string from the key.
	std::string getStr(const std::string& key) const;
	std::string getStr(const std::string& section, const std::string& key) const;
	std::string getStr(const std::string& section, const std::string& key, const std::string& def) const;

	// Get a integer from the key.
	int getInt(const std::string& key) const;
	int getInt(const std::string& section, const std::string& key) const;
	int getInt(const std::string& section, const std::string& key, int def) const;

	// Get a double from the key.
	double getDouble(const std::string& key) const;
	double getDouble(const std::string& section, const std::string& key) const;
	double getDouble(const std::string& section, const std::string& key, double def) const;

	// Get a boolean from the key.
	bool getBoolean(const std::string& key) const;
	bool getBoolean(const std::string& section, const std::string& key) const;
	bool getBoolean(const std::string& section, const std::string& key, bool def) const;

	// Set the given entry with the provided value. If the entry cannot be found
	// -1 is returned and the entry is created. Else 0 is returned.
	int setStr(const std::string& key, const std::string& val);
	int setInt(const std::string& key, int val);
	int setDouble(const std::string& key, double val);
	int setBoolean(const std::string& key, bool val);

	int getNsec() const;					// Get number of sections.
	std::string getSecname(int n) const;			// Get name for section n.
	int findEntry(const std::string& entry) const;	// Return 1 if the entry exists, 0 otherwise

private:
	// Check if the key is in the form section:key and if yes create the section in the dictionnary
	// if it doesn't exist.
	void makeSectionFromKey(const std::string& key);

	void freeDico();	// Unalloc memory
	dictionary * dico;		// The dictionnary containing the parsed data
};

#endif // _INIT_PARSER_H_
