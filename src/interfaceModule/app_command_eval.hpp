#ifndef APP_COMMAND_EVAL_HPP
#define APP_COMMAND_EVAL_HPP

#include <map>
#include <string>

class AppCommandEval {
public: 
    AppCommandEval();
    ~AppCommandEval();
	AppCommandEval(AppCommandEval const &) = delete;
	AppCommandEval& operator = (AppCommandEval const &) = delete;

	std::string evalString (const std::string &var);
	double evalDouble (const std::string &var);
	int evalInt (const std::string &Int);

	void define(const std::string& mArg, const std::string& mValue);
	void commandAdd(const std::string& mArg, const std::string& mValue);
	void commandMul(const std::string& mArg, const std::string& mValue);
	void commandRandomMin(const std::string& mValue);
	void commandRandomMax(const std::string& mValue);
	void deleteVar();
	void printVar();

private:
	//variables utilisées dans le moteur de scripts
	std::map<const std::string, std::string> variables;
	double max_random;
	double min_random;
};

#endif