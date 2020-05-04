#include "interfaceModule/app_command_eval.hpp"

#include "tools/utility.hpp"

AppCommandEval::AppCommandEval()
{
	min_random = 0.0;
	max_random = 1.0;
}

AppCommandEval::~AppCommandEval()
{
	this->deleteVar();
}

std::string AppCommandEval::evalString (const std::string &var)
{
	auto var_it = variables.find(var);
	if (var_it == variables.end()) //pas trouvé donc on renvoie la valeur de la chaine
		return var;
	else // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		return var_it->second;
}

double AppCommandEval::evalDouble (const std::string &var)
{
	auto var_it = variables.find(var);
	if (var_it == variables.end()) //pas trouvé donc on renvoie la valeur de la chaine
		return Utility::strToDouble(var);
	else // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		return Utility::strToDouble(var_it->second);
}


int AppCommandEval::evalInt (const std::string &var)
{
	double tmp=this->evalDouble(var);
	return (int) tmp;
}


void AppCommandEval::define(const std::string& mArg, const std::string& mValue)
{
	//std::cout << "Command define : " <<  mArg.c_str() << " => " << mValue.c_str() << std::endl;
	if (mValue == "random") {
		float value = (float)rand()/RAND_MAX* (max_random-min_random)+ min_random;
		variables[mArg] = Utility::floatToStr(value);
	} else {
		//~ printf("mValue = %s\n", mValue.c_str());
		//std::cout << "Cette valeur de mValue vaut " << evalDouble(mValue) << std::endl;
		variables[mArg] = Utility::doubleToString( evalDouble(mValue) );
	//	this->printVar();
	}
}

void AppCommandEval::commandAdd(const std::string& mArg, const std::string& mValue)
{
	auto var_it = variables.find(mArg);

	if (var_it == variables.end()) { //pas trouvé donc on renvoie la valeur de la chaine
		std::cout << "not possible to operate with undefined variable so define to 0" << std::endl;
		variables[mArg] = Utility::strToDouble (mValue);
	} else { // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		double tmp = Utility::strToDouble( variables[mArg] ) + this->evalDouble (mValue);
		variables[mArg] = Utility::floatToStr(tmp);
	}
}

void AppCommandEval::commandMul(const std::string& mArg, const std::string& mValue)
{
	auto var_it = variables.find(mArg);
	if (var_it == variables.end()) {
		std::cout << "not possible to operate with undefined variable so define to 1" << std::endl;
		variables[mArg] = Utility::strToDouble (mValue);
	} else {
		double tmp = Utility::strToDouble( variables[mArg] ) * Utility::strToDouble (mValue);
		variables[mArg] = Utility::floatToStr(tmp);
	}
}

void AppCommandEval::commandRandomMin(const std::string& mValue)
{
	min_random = evalDouble(mValue);
}

void AppCommandEval::commandRandomMax(const std::string& mValue)
{
	max_random = evalDouble(mValue);
}


void AppCommandEval::printVar()
{
	std::cout << "+++++++++++++++++" << std::endl;
	if (variables.size() == 0){
		std::cout << "No variable available" << std::endl;
	}
	else {
		for (auto var_it = variables.begin(); var_it != variables.end(); ++var_it)	{
			std::cout << var_it->first << " => " << var_it->second << '\n';
		}
	}
	std::cout << "-----------------" << std::endl;
}

void AppCommandEval::deleteVar()
{
	variables.clear();
}
