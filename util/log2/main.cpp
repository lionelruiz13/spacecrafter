/*
 * main.cpp
 * 
 * Copyright 2018 Olivier NIVOIX <olivier@orion>
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
#include "log2.hpp"

int main(int argc, char **argv)
{
	ncLog* test = ncLog::get();
	test->open("test.log");
	test->setDebug(true);
	//~ test->setLevel(LOG_TYPE::L_DEBUG);

	test->write("test initial");
	test->write("test info matin malin ", LOG_TYPE::L_INFO);
	test->write("test debug program", LOG_TYPE::L_DEBUG);
	test->write("test warning hot coffy", LOG_TYPE::L_WARNING);
	test->write("test error string ", LOG_TYPE::L_ERROR);

	ncLog::get()->write("oki instanciation");

	test->close();
	delete test;

	return 0;
}

