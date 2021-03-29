#include "interfaceModule/app_command_eval.hpp"
#include "coreModule/coreLink.hpp"
#include "tools/utility.hpp"

std::function<double(double,double)> f_add = [](double x, double y){return x+y;};
std::function<double(double,double)> f_sub = [](double x, double y){return x-y;};
std::function<double(double,double)> f_mul = [](double x, double y){return x*y;};
std::function<double(double,double)> f_div = [](double x, double y){return x/y;};
std::function<double(double,double)> f_tan = [](double x, double y){return tan(y*3.1415926/180.0);};
std::function<double(double,double)> f_trunc = [](double x, double y){return trunc(y);};
std::function<double(double,double)> f_sin = [](double x, double y){return sin(y*3.1415926/180.0);};

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
	m_reservedVar[ACI_RW_SUN_AZIMUTH]=SC_RESERVED_VAR::SUN_AZIMUTH;
	m_reservedVar[ACI_RW_DATE_YEAR]=SC_RESERVED_VAR::DATE_YEAR;
	m_reservedVar[ACI_RW_DATE_MONTH]=SC_RESERVED_VAR::DATE_MONTH;
	m_reservedVar[ACI_RW_DATE_DAY]=SC_RESERVED_VAR::DATE_DAY;
	m_reservedVar[ACI_RW_DATE_HOUR]=SC_RESERVED_VAR::DATE_HOUR;
	m_reservedVar[ACI_RW_HEADING]=SC_RESERVED_VAR::HEADING;

	// for conivence, the map inverse
	for (const auto& [key, val] : m_reservedVar)
		m_reservedVarInv.emplace(val, key);
}

AppCommandEval::~AppCommandEval()
{
	this->deleteVar();
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
	auto reservedVar = m_reservedVar.find(mArg);
	if (reservedVar != m_reservedVar.end()) {
		std::cout << mArg << " is a reservedVar so you can't define it" << std::endl;
		return;
	}
	//std::cout << "C_define : " <<  mArg.c_str() << " => " << mValue.c_str() << std::endl;
	if (mValue == "random") {
		//std::cout << "C_define random: min " <<  min_random << " max " << max_random << std::endl;
		float value = (float)rand()/RAND_MAX* (max_random-min_random)+ min_random;
		//std::cout << "C_define random: value " <<  value  << std::endl;
		variables[mArg] = std::to_string(value);
	} else {
		//~ printf("mValue = %s\n", mValue.c_str());
		// std::cout << "Cette valeur de mValue vaut " << evalDouble(mValue) << std::endl;
		//std::cout << "C_define : " <<  mArg.c_str() << " => " << evalDouble(mValue) << std::endl;
		double v = evalDouble(mValue);
		if (v == trunc(v)) 
			variables[mArg] = std::to_string(evalInt(mValue));
		else
			variables[mArg] = std::to_string(v);
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

void AppCommandEval::commandDiv(const std::string& mArg, const std::string& mValue)
{
	this->evalOps(mArg,mValue, f_div);
}

void AppCommandEval::commandTan(const std::string& mArg, const std::string& mValue)
{
	this->evalOps(mArg,mValue, f_tan);
}

void AppCommandEval::commandTrunc(const std::string& mArg, const std::string& mValue)
{
	this->evalOps(mArg,mValue, f_trunc);
}

void AppCommandEval::commandSin(const std::string& mArg, const std::string& mValue)
{
	this->evalOps(mArg,mValue, f_sin);
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
		std::cout << "not possible to operate with undefined variable so define to null from ops" << std::endl;
		variables[mArg] = Utility::strToDouble (mValue);
	} else { // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		double tmp = f( Utility::strToDouble( variables[mArg] ) , this->evalDouble(mValue));
		variables[mArg] = std::to_string(tmp);
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
		variables[mArg] = std::to_string(tmp);
	}
}*/

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

double AppCommandEval::evalReservedVariable(const std::string &var)
{
	switch (m_reservedVar[var]) {
		case  SC_RESERVED_VAR::LONGITUDE : {
			double lon = coreLink->observatoryGetLongitude() + 180;
			lon = lon - 360 * floor(lon / 360.);
			return lon - 180;
		      }
		case  SC_RESERVED_VAR::LATITUDE :  
			return coreLink->observatoryGetLatitude(); 
		case  SC_RESERVED_VAR::ALTITUDE :  
			return coreLink->observatoryGetAltitude(); 
		case  SC_RESERVED_VAR::HEADING :  
			return coreLink->getHeading(); 
		case SC_RESERVED_VAR::SUN_ALTITUDE:
			return coreLink->getSunAltitude(); 
		case SC_RESERVED_VAR::SUN_AZIMUTH: {
			double azi = coreLink->getSunAzimuth() + 180;
			azi = azi - 360 * floor(azi / 360.);
			return azi - 180;
		      }
		case SC_RESERVED_VAR::DATE_YEAR:
			return coreLink->getDateYear(); 
		case SC_RESERVED_VAR::DATE_MONTH:
			return coreLink->getDateMonth(); 
		case SC_RESERVED_VAR::DATE_DAY:
			return coreLink->getDateDay(); 
		case SC_RESERVED_VAR::DATE_HOUR:
			return coreLink->getDateHour(); 
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
		case SC_RESERVED_VAR::SUN_AZIMUTH :
			std::cout << "No setter with reserved variable " << var << ". Do nothing." << std::endl; break;
		case SC_RESERVED_VAR::DATE_YEAR :
			std::cout << "No setter with reserved variable " << var << ". Do nothing." << std::endl; break;
		case SC_RESERVED_VAR::DATE_MONTH :
			std::cout << "No setter with reserved variable " << var << ". Do nothing." << std::endl; break;
		case SC_RESERVED_VAR::DATE_DAY :
			std::cout << "No setter with reserved variable " << var << ". Do nothing." << std::endl; break;
		case SC_RESERVED_VAR::DATE_HOUR :
			std::cout << "No setter with reserved variable " << var << ". Do nothing." << std::endl; break;
		default:
			std::cout << "Unknown reserved variable " << var << ". Do nothing." << std::endl;
	}
}
