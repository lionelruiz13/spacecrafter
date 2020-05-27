#include "interfaceModule/app_command_eval.hpp"
#include "coreModule/coreLink.hpp"
#include "tools/utility.hpp"

std::function<double(double,double)> f_add = [](double x, double y){return x+y;};
std::function<double(double,double)> f_sub = [](double x, double y){return x-y;};
std::function<double(double,double)> f_mul = [](double x, double y){return x*y;};

AppCommandEval::AppCommandEval(CoreLink *_coreLink)
{
	min_random = 0.0;
	max_random = 1.0;
	this->initReservedVariable();
	coreLink = _coreLink;
}

void AppCommandEval::initReservedVariable()
{
	// owr reserved variables
	m_reservedVar[ACI_RW_ALTITUDE]=SC_RESERVED_VAR::ALTITUDE;
	m_reservedVar[ACI_RW_LONGITUDE]=SC_RESERVED_VAR::LONGITUDE;
	m_reservedVar[ACI_RW_LATITUDE]=SC_RESERVED_VAR::LATITUDE;
	m_reservedVar[ACI_RW_SUN_ALTITUDE]=SC_RESERVED_VAR::SUN_ALTITUDE;
	m_reservedVar[ACI_RW_HEADING]=SC_RESERVED_VAR::HEADING;

	// for conivence, the map inverse
	for (const auto& [key, val] : m_reservedVar)
		m_reservedVarInv.emplace(val, key);
}


std::string AppCommandEval::evalString(const std::string &var)
{
	auto var_it = variables.find(var);
	if (var_it == variables.end()) //pas trouvé donc on renvoie la valeur de la chaine
		return var;
	else // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		return var_it->second;
}

double AppCommandEval::evalDouble(const std::string &var)
{
	// capture context with reservedVariables Elitit-40
	auto reservedVar = m_reservedVar.find(var);
	if (reservedVar != m_reservedVar.end())
		return evalReservedVariable(var);

	auto var_it = variables.find(var);
	if (var_it == variables.end()) //pas trouvé donc on renvoie la valeur de la chaine
		return Utility::strToDouble(var);
	else // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		return Utility::strToDouble(var_it->second);
}

int AppCommandEval::evalInt(const std::string &var)
{
	double tmp=this->evalDouble(var);
	return (int) tmp;
}

void AppCommandEval::define(const std::string& mArg, const std::string& mValue)
{
	//std::cout << "C_define : " <<  mArg.c_str() << " => " << mValue.c_str() << std::endl;
	if (mValue == "random") {
		//std::cout << "C_define random: min " <<  min_random << " max " << max_random << std::endl;
		float value = (float)rand()/RAND_MAX* (max_random-min_random)+ min_random;
		//std::cout << "C_define random: value " <<  value  << std::endl;
		variables[mArg] = Utility::floatToStr(value);
	} else {
		//~ printf("mValue = %s\n", mValue.c_str());
		// std::cout << "Cette valeur de mValue vaut " << evalDouble(mValue) << std::endl;
		//std::cout << "C_define : " <<  mArg.c_str() << " => " << evalDouble(mValue) << std::endl;
		variables[mArg] = Utility::doubleToString( evalDouble(mValue) );
	//	this->printVar();
	}
}

void AppCommandEval::commandAdd(const std::string& mArg, const std::string& mValue)
{
	this->evalOps(mArg,mValue, f_add);
}

void AppCommandEval::commandSub(const std::string& mArg, const std::string& mValue)
{
	this->evalOps(mArg,mValue, f_sub);
}

void AppCommandEval::commandMul(const std::string& mArg, const std::string& mValue)
{
	this->evalOps(mArg,mValue, f_mul);
}


void AppCommandEval::evalOps(const std::string& mArg, const std::string& mValue, std::function<double(double,double)> f)
{
	// capture context with reservedVariables Elitit-40
	auto reservedVar = m_reservedVar.find(mArg);
	if (reservedVar != m_reservedVar.end()) {
		double tmp = f( evalReservedVariable(mArg), this->evalDouble(mValue));
		setReservedVariable(mArg,tmp);
	}

	auto var_it = variables.find(mArg);
	if (var_it == variables.end()) { //pas trouvé donc on renvoie la valeur de la chaine
		std::cout << "not possible to operate with undefined variable so define to 0" << std::endl;
		variables[mArg] = Utility::strToDouble (mValue);
	} else { // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		double tmp = f( Utility::strToDouble( variables[mArg] ) , this->evalDouble(mValue));
		variables[mArg] = Utility::floatToStr(tmp);
	}
}


/*
//@TODO : this fonction is a copy/paste from commandAdd and should be refactorized
void AppCommandEval::commandMul(const std::string& mArg, const std::string& mValue)
{
	// capture context with reservedVariables Elitit-40
	auto reservedVar = m_reservedVar.find(mArg);
	if (reservedVar != m_reservedVar.end()) {
		double tmp = evalReservedVariable(mArg) * this->evalDouble(mValue);
		setReservedVariable(mArg,tmp);
	}

	auto var_it = variables.find(mArg);
	if (var_it == variables.end()) {
		std::cout << "not possible to operate with undefined variable so define to 1" << std::endl;
		variables[mArg] = Utility::strToDouble(mValue);
	} else {
		double tmp = Utility::strToDouble( variables[mArg] ) * Utility::strToDouble(mValue);
		variables[mArg] = Utility::floatToStr(tmp);
	}
}*/


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


double AppCommandEval::evalReservedVariable(const std::string &var)
{
	switch (m_reservedVar[var]) {
		case  SC_RESERVED_VAR::LONGITUDE :  
			return coreLink->observatoryGetLongitude(); break;
		case  SC_RESERVED_VAR::LATITUDE :  
			return coreLink->observatoryGetLatitude(); break;
		case  SC_RESERVED_VAR::ALTITUDE :  
			return coreLink->observatoryGetAltitude(); break;
		case  SC_RESERVED_VAR::HEADING :  
			return coreLink->getHeading(); break;
		case SC_RESERVED_VAR::SUN_ALTITUDE:
			return coreLink->getSunAltitude(); break;
		default:
			std::cout << "Unknown reserved variable " << var << ". Default 0.0 is returned." << std::endl;
			return 0.0;
	}
}


void AppCommandEval::setReservedVariable(const std::string &var, double value)
{
	switch (m_reservedVar[var]) {
		case  SC_RESERVED_VAR::LONGITUDE :  
			coreLink->observatorySetLongitude(value); break;
		case  SC_RESERVED_VAR::LATITUDE :  
			coreLink->observatorySetLatitude(value); break;
		case  SC_RESERVED_VAR::ALTITUDE :  
			coreLink->observatorySetAltitude(value); break;
		case  SC_RESERVED_VAR::HEADING :  
			coreLink->setHeading(value); break;
		case SC_RESERVED_VAR::SUN_ALTITUDE :
			std::cout << "No setter with reserved variable " << var << ". Do nothing." << std::endl; break;
		default:
			std::cout << "Unknown reserved variable " << var << ". Do nothing." << std::endl;
	}
}
