#ifndef APP_COMMAND_EVAL_HPP
#define APP_COMMAND_EVAL_HPP

#include <map>
#include <string>
#include <functional>
#include "interfaceModule/base_command_interface.hpp"
#include "tools/no_copy.hpp"

class CoreLink;


/**
* \file app_command_eval.hpp
* \brief Processes script engine variables
* \author Elitit-40
* \version 1
*
* \class AppCommandEval
*
* \brief Processes script engine variables.
*
* Variables from the scripting engine are treated in this class. 
* 
* @section Description
*
* A variable is simply defined by a character string associated in a map with its value represented by a character string.
*
* The script engine only processes strings.
*
* All character strings are analyzed: if the character string represents a variable saved in the map, its saved value is returned.
* Otherwise, it is a numeric value. 
*
* All the variables declared by the user are managed in the map.
* However there are pre-declared variables which bypass the map.
* These variables are directly related to CoreLink.
* 
*/
class AppCommandEval : public NoCopy{
public: 
	// constructor
    AppCommandEval(CoreLink *_coreLink);
    // destructor ?
	~AppCommandEval();

	//! transform as possible the parameter to a string
	std::string evalString(const std::string &var);
	//! transform as possible the parameter to double
	double evalDouble(const std::string &var);
	//! transform as possible the parameter to int
	int evalInt(const std::string &var);

	//! create a string variable with value 
	void define(const std::string& mArg, const std::string& mValue);
	//! first becomes first added by the second
	void commandAdd(const std::string& mArg, const std::string& mValue);
	//! first becomes first substracted by the second
	void commandSub(const std::string& mArg, const std::string& mValue);
	//! first becomes first multiplied by the second
	void commandMul(const std::string& mArg, const std::string& mValue);
	//! fix the minimum random value for the internal random generator 
	void commandRandomMin(const std::string& mValue);
	//! fix the maximum random value for the internal random generator
	void commandRandomMax(const std::string& mValue);
	//! delete all variables defined with function define
	void deleteVar();
	//! print all defined variables on console
	void printVar();

private:
	void initReservedVariable();
	double evalReservedVariable(const std::string &var);
	void setReservedVariable(const std::string &var, double value);

	void evalOps(const std::string& mArg, const std::string& mValue, std::function<double(double,double)> f);
	
	//variables utilisées dans le moteur de scripts
	std::map<const std::string, std::string> variables;
	// map of system variables accessible through CoreLink
	std::map<const std::string, SC_RESERVED_VAR> m_reservedVar;
	// reverse map to avoid no reseach time
	std::map<SC_RESERVED_VAR, const std::string> m_reservedVarInv;
	double max_random;
	double min_random;
	CoreLink *coreLink;
};

#endif