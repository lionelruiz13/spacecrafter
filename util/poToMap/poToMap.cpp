/*
 * poToMap
 * 
 * Copyright 2020 Jeviensdelazone57
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <iostream>
#include <fstream>
#include <utility>
#include <map>

int main(int argc, char **argv)
{
	// test number of arguments
	if (argc <2) {
		exit(-1);
	}

	//Reading
	// here argv[1] exists, we can use it
	std::ifstream infile;
	infile.open(argv[1], std::ifstream::in);
	if (!infile.is_open()) {
		std::cout << "unable to open file" << std::endl;
		exit(-2);
	}

	//Writing
	std::ofstream outfile;
    std::string oldName = argv[1];
    std::size_t found = oldName.find_first_of(".");
    std::string outName = oldName.substr(0,found);
    outName = outName + ".txt";
	outfile.open(outName, std::ifstream::out);
	if (!outfile.is_open()) {
		std::cout << "unable to open file" << "\n";
		exit(-3);
	}

    std::map<std::string, std::string> m_translator;

    //treatment
	std::string line;
    std::size_t foundGID, foundSTR;

    std::pair<std::string, std::string> token;
	while (std::getline(infile, line))
	{
        if (line[0] != '#' && line[0] != '\r' && line[0] != '\n' ) {

            // find GID
            foundGID = line.find("msgid");
            if (foundGID!=std::string::npos){
                token.first = line.substr(6);
            }

            // find GID
            foundSTR = line.find("msgstr");
            if (foundSTR!=std::string::npos){
                token.second = line.substr(7);
                if (token.first!="\"\"" && token.first!=token.second) {
                    outfile << token.first << ";" << token.second << std::endl;
                    //std::cout << token.first << ";" << token.second  << std::endl;
                    m_translator[token.first] = token.second;
                }
                token.first.clear();
                token.second.clear();
            }
        }
    //    std::cout << "Ukn: " << line << std::endl;
	}

	infile.close();
	outfile.close();

    //test it !
    std::string value;
    auto it= m_translator.find("\"Moon\"");
    if( it != m_translator.end() )
        std::cout << "moon: " << it->second << "\n";
    else
        std::cout << "moon:\n";
    m_translator.clear();


    infile.open(outName, std::ifstream::in);

	if (infile.is_open()) {
		std::string line;
		std::string key;
		std::string value;
		std::size_t found;
		while (std::getline(infile, line))
		{
        	if (line[0] != '#' && line[0] != '\r' && line[0] != '\n' ) {
				//std::cout << " candidat: " << line << std::endl;
				found = line.find("\";\"");
				if (found != std::string::npos ) {
					key = line.substr(1, found-1);
					value = line.substr(found+3, line.length()-(found+4) );
					
                    std::cout << key << "<->" << value << std::endl;
                    
                    m_translator[key] = value;
				}
        	}
		}	
		infile.close();
	}




 	return 0;
}

