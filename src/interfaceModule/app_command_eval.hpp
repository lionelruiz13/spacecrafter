#ifndef APP_COMMAND_EVAL_HPP
#define APP_COMMAND_EVAL_HPP

#include <map>
#include <string>
#include <functional>
#include "interfaceModule/base_command_interface.hpp"
#include "tools/no_copy.hpp"

class CoreLink;

class AppCommandEval : public NoCopy{
public: 
    AppCommandEval(CoreLink *_coreLink);
    ~AppCommandEval();

	std::string evalString (const std::string &var);
	double evalDouble (const std::string &var);
	int evalInt (const std::string &Int);

	void define(const std::string& mArg, const std::string& mValue);
	void commandAdd(const std::string& mArg, const std::string& mValue);
	void commandSub(const std::string& mArg, const std::string& mValue);
	void commandMul(const std::string& mArg, const std::string& mValue);
	void commandRandomMin(const std::string& mValue);
	void commandRandomMax(const std::string& mValue);
	void deleteVar();
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