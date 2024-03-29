/*
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006, 2007
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <iostream>

#include "starModule/string_array.hpp"


namespace BigStarCatalog {

void StringArray::initFromFile(const std::string& file_name)
{
	clear();
	std::list<std::string> list;
	FILE *f = fopen(file_name.c_str(),"r");
	if (f) {
		char line[256];
		while (fgets(line, sizeof(line), f)) {
			std::string s = line;
			// remove newline
			s.erase(s.length()-1,1);
			list.push_back(s);
			size++;
		}
		fclose(f);
	}
	if (size > 0) {
		array = new std::string[size];
		if (array == 0) {
			std::cerr << "ERROR: StringArray::initFromFile: no memory" << std::endl;
			exit(1);
		}
		for (int i=0; i<size; i++) {
			array[i] = list.front();
			list.pop_front();
		}
	}
}

} // namespace BigStarCatalog

