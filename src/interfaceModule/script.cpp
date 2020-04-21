/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014-2017 Association Sirius
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

#include <iostream>
#include <fstream>
#include <cstddef>
#include "interfaceModule/script.hpp"
#include "tools/log.hpp"


Token::Token(const std::string &s, const std::string &p)
{
	elmt.clear();
	path.clear();
	pNext=nullptr;
	elmt=s;
	path=p;
}

Token::~Token()
{
	if (pNext != nullptr) delete pNext;
}

void Token::printToken()
{
	cLog::get()->write("Token script : " + path + " : " + elmt,  LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT);
}

Script::Script()
{
	//cout << "Script->init()" << endl;
	pFirst=nullptr;
	pLast=nullptr;
	p2First=nullptr;
	p2Last=nullptr;
}

Script::~Script()
{
	if (pFirst != nullptr) delete pFirst;
}

void Script::printScript()
{
	Token *pMove=pFirst;
	cLog::get()->write("Printing script: --BEGIN",  LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT);
	while (pMove !=nullptr) {
		cLog::get()->write("   " + pMove->getToken(),  LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT);
		pMove=pMove->pNext;
	}
	cLog::get()->write("Printing script: --END",  LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT);
}

void Script::clean()
{
	if (pFirst != nullptr) delete pFirst;
	pFirst = nullptr;
	pLast = nullptr;
	p2First = nullptr;
	p2Last = nullptr;
}

int Script::load(const std::string &script_file, const std::string &script_path )
{
	if (pFirst == nullptr) {
		//cout << "loading script "<< script_file << " when no script is playing" << endl;
		return loadInternal(script_file, script_path ,ListPosition::first);
	} else {
		//cout << "loading script " << script_file << " when script is playing" << endl;
		return loadInternal(script_file, script_path ,ListPosition::second);
	}
}

int Script::loadInternal(const std::string &script_file,const std::string & script_path , ListPosition wp)
{
	//~ printf("s : %s p : %s\n", script_file.c_str() , script_path.c_str() );
	input_file = new std::ifstream(script_file.c_str());

	if (! input_file->is_open()) {
		cLog::get()->write("Unable to open script: " + script_file,  LOG_TYPE::L_ERROR, LOG_FILE::SCRIPT);
		return 0;
	}

	bool is_script_empty=true;
	std::string line;
	Token *token=nullptr;
	while (! input_file->eof() ) {
		getline(*input_file,line);

		if ( line[0] != '#' && line[0] != 0 && line[0] != '\r' && line[0] != '\n') {
			//cout << "[script.cpp => Line is: " << line << "]"<< endl;
			token=new Token(line, script_path);
			is_script_empty=false;
			if (wp==ListPosition::first)
				addFirst(token);
			else
				addSecond(token);
		}
	}
	input_file->close();
	if ((! is_script_empty) && (wp==ListPosition::second)) {
		p2Last->pNext=pFirst;
		pFirst=p2First;
		p2Last=nullptr;
		p2First=nullptr;
	}
	//printScript();
	return 1;
}

void Script::addFirstInQueue(Token * token){
	
	if (pFirst == nullptr) {
		pFirst=token;
		pLast=token;
	}else {
		
		Token * temp = pFirst;
		pFirst = token;
		token->pNext = temp;
	}
}

void Script::addFirst(Token *token)
{
	if (pFirst == nullptr) {
		pFirst=token;
		pLast=token;
	} else {
		pLast->pNext=token;
		pLast=pLast->pNext;
	}
}

void Script::addSecond(Token *token)
{
	if (p2First == nullptr) {
		p2First=token;
		p2Last=token;
	} else {
		p2Last->pNext=token;
		p2Last=p2Last->pNext;
	}
}

int Script::getFirst(std::string &command, std::string &dataDir)
{
	Token *pMove;
	if (pFirst == nullptr) {
		cLog::get()->write("End of script",  LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
		command="script action end";
		dataDir="";
		return 0;
	} else {
		pMove=pFirst;
		dataDir=pFirst->getTokenPath();
		command=pFirst->getToken();
		if (command=="script action end" && (pFirst->pNext != nullptr)) {
			cLog::get()->write("End of script  detected but not at end of the execution stack", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
		}
		//cout << " script.cpp : " << command << endl;
		pFirst=pFirst->pNext;
		pMove->pNext=nullptr;
		delete pMove;
		return 1;
	}
}
