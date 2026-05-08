#include "interfaceModule/app_command_eval.hpp"
#include "coreModule/coreLink.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include <sstream>

std::function<double(double,double)> f_add = [](double x, double y){return x+y;};
std::function<double(double,double)> f_sub = [](double x, double y){return x-y;};
std::function<double(double,double)> f_mul = [](double x, double y){return x*y;};
std::function<double(double,double)> f_div = [](double x, double y){return x/y;};
std::function<double(double,double)> f_mod = [](double x, double y){return std::fmod(x,y);};
std::function<double(double,double)> f_tan = [](double x, double y){return tan(y*3.1415926/180.0);};
std::function<double(double,double)> f_trunc = [](double x, double y){return trunc(y);};
std::function<double(double,double)> f_sin = [](double x, double y){return sin(y*3.1415926/180.0);};

// Utility function to format numbers with significant digits only
static std::string formatNumber(double value) {
	const int64_t intVal = static_cast<int64_t>(value);
	if (value == static_cast<double>(intVal)) { // Representable as integer, return as integer
		return std::to_string(intVal);
	} else {
		// Float value, use adaptive precision
		std::ostringstream oss;
		oss << std::defaultfloat << value;
		return oss.str();
	}
}

AppCommandEval::AppCommandEval(std::shared_ptr<CoreLink> _coreLink)
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
	m_reservedVar[ACI_RW_SELECTED_AZ]=SC_RESERVED_VAR::SELECTED_AZ;
	m_reservedVar[ACI_RW_SELECTED_ALT]=SC_RESERVED_VAR::SELECTED_ALT;
	m_reservedVar[ACI_RW_SELECTED_RA]=SC_RESERVED_VAR::SELECTED_RA;
	m_reservedVar[ACI_RW_SELECTED_DE]=SC_RESERVED_VAR::SELECTED_DE;
	m_reservedVar[ACI_RW_SELECTED_DISTANCE]=SC_RESERVED_VAR::SELECTED_DISTANCE;
	m_reservedVar[ACI_RW_SELECTED_MAGNITUDE]=SC_RESERVED_VAR::SELECTED_MAGNITUDE;
	m_reservedVar[ACI_RW_SELECTED_STAR_RA]=SC_RESERVED_VAR::SELECTED_STAR_RA;
	m_reservedVar[ACI_RW_SELECTED_STAR_DE]=SC_RESERVED_VAR::SELECTED_STAR_DE;
	m_reservedVar[ACI_RW_DATE_YEAR]=SC_RESERVED_VAR::DATE_YEAR;
	m_reservedVar[ACI_RW_DATE_MONTH]=SC_RESERVED_VAR::DATE_MONTH;
	m_reservedVar[ACI_RW_DATE_DAY]=SC_RESERVED_VAR::DATE_DAY;
	m_reservedVar[ACI_RW_DATE_HOUR]=SC_RESERVED_VAR::DATE_HOUR;
	m_reservedVar[ACI_RW_DATE_MINUTE]=SC_RESERVED_VAR::DATE_MINUTE;
	m_reservedVar[ACI_RW_HEADING]=SC_RESERVED_VAR::HEADING;
	m_reservedVar[ACI_RW_BODY_SELECTED]=SC_RESERVED_VAR::BODY_SELECTED;
	m_reservedVar[ACI_RW_LANGUAGE]=SC_RESERVED_VAR::LANGUAGE;
	m_reservedVar[ACI_RW_JOYPAD]=SC_RESERVED_VAR::JOYPAD;
	m_reservedVar[ACI_RW_CURRENT_MODE]=SC_RESERVED_VAR::CURRENT_MODE;

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
	// Check if this is a reserved variable
	auto reservedVar = m_reservedVar.find(var);
	if (reservedVar != m_reservedVar.end()) {
		// Reserved variable found - evaluate and return its value
		return formatNumber(evalReservedVariable(var));
	}

	// Check if this is a direct variable lookup (old behavior)
	auto var_it = variables.find(var);
	if (var_it != variables.end()) {
		return var_it->second.first;
	}

	// Check if the string contains @{variable} patterns for interpolation
	std::string result = var;
	size_t pos = 0;
	while ((pos = result.find("@{", pos)) != std::string::npos) {
		size_t depth = 1;
		size_t pos2 = pos + 1;
		while (depth) {
			switch (result[++pos2]) {
				case '\0':
					return result; // Mismatching {}
				case '{':
					if (result[pos2-2U] == '@')
						++depth;
					break;
				case '}':
					--depth;
					break;
			}
		}
		std::string replacement = evalString(result.substr(pos + 2, pos2 - (pos + 2)));
		result.replace(pos, pos2 - pos + 1, replacement);
		pos += replacement.length();
	}
	return result;
}

double AppCommandEval::evalDouble(const std::string &var)
{
	// capture context with reservedVariables Elitit-40
	auto reservedVar = m_reservedVar.find(var);
	if (reservedVar != m_reservedVar.end())
		return evalReservedVariable(var);

	auto var_it = variables.find(var);
	if (var_it == variables.end()) //not found so we return the value of the string
		return Utility::strToDouble(var);
	else // found returns the value of what is stored in memory
		return var_it->second.second;
}

int AppCommandEval::evalInt(const std::string &var)
{
	return static_cast<int>(evalDouble(var));
}

void AppCommandEval::define(const std::string& mArg, const std::string& mValue)
{
	auto reservedVar = m_reservedVar.find(mArg);
	if (reservedVar != m_reservedVar.end()) {
		std::cout << mArg << " is a reservedVar so you can't define it" << std::endl;
		return;
	}
	auto &var = variables[mArg];
	reservedVar = m_reservedVar.find(mValue);
	if (reservedVar != m_reservedVar.end()) {
		var.first = formatNumber(var.second = evalReservedVariable(mValue));
	} else {
		auto var_it = variables.find(mValue);
		if (var_it == variables.end()) { //not found so we return the value of the string
			try {
				var.first = formatNumber(var.second = std::stod(mValue));
			} catch (...) {
				var.second = 0.;
				var.first = mValue;
			}
		} else { // found returns the value of what is stored in memory
			var = var_it->second;
		}
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

void AppCommandEval::commandMod(const std::string& mArg, const std::string& mValue)
{
	this->evalOps(mArg,mValue, f_mod);
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
		double tmp = f( evalReservedVariable(mArg), evalDouble(mValue));
		setReservedVariable(mArg,tmp);
	}

	auto var_it = variables.find(mArg);
	if (var_it == variables.end()) { //not found so we return the value of the string
		cLog::get()->write("Not possible to operate with undefined variable so define to null from ops", LOG_TYPE::L_WARNING , LOG_FILE::SCRIPT);
		define(mArg, mValue);
	} else { // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		var_it->second.first = formatNumber(var_it->second.second = f(var_it->second.second , evalDouble(mValue)));
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
	//std::cout << "+++++++++++++++++" << std::endl;
	cLog::get()->mark( LOG_FILE::SCRIPT);
	if (variables.size() == 0){
		//std::cout << "No variable available" << std::endl;
		cLog::get()->write("Not variable available", LOG_TYPE::L_INFO , LOG_FILE::SCRIPT);
	}
	else {
		for (auto var_it = variables.begin(); var_it != variables.end(); ++var_it)	{
			//std::cout << var_it->first << " => " << var_it->second << '\n';
			cLog::get()->write(var_it->first + " => " + var_it->second.first, LOG_TYPE::L_INFO , LOG_FILE::SCRIPT);
		}
	}
	//std::cout << "-----------------" << std::endl;
	cLog::get()->mark(LOG_FILE::SCRIPT);
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
		case  SC_RESERVED_VAR::LATITUDE : {
		    double lat = coreLink->observatoryGetLatitude();
			if (lat>90) lat = 90.0;
			if (lat<-90) lat = -90.0;
			return lat;
		    }
		case  SC_RESERVED_VAR::ALTITUDE :
			return coreLink->observatoryGetAltitude();
		case  SC_RESERVED_VAR::HEADING :{
			double azi = coreLink->getHeading() + 180;
			azi = azi - 360 * floor(azi / 360.);
			return azi - 180;
		    }
		case SC_RESERVED_VAR::SUN_ALTITUDE:
			return coreLink->getSunAltitude();
		case SC_RESERVED_VAR::SUN_AZIMUTH: {
			double azi = coreLink->getSunAzimuth() + 180;
			azi = azi - 360 * floor(azi / 360.);
			return azi - 180;
	        }
		case SC_RESERVED_VAR::SELECTED_AZ:
			return coreLink->getSelectedAZ();
		case SC_RESERVED_VAR::SELECTED_ALT:
			return coreLink->getSelectedALT();
		case SC_RESERVED_VAR::SELECTED_RA:
			return coreLink->getSelectedRA();
		case SC_RESERVED_VAR::SELECTED_DE:
			return coreLink->getSelectedDE();
		case SC_RESERVED_VAR::SELECTED_DISTANCE:
			return coreLink->getSelectedDistance();
		case SC_RESERVED_VAR::SELECTED_MAGNITUDE:
			return coreLink->getSelectedMagnitude();
		case SC_RESERVED_VAR::SELECTED_STAR_RA:
			return coreLink->getSelectedStarRA();
		case SC_RESERVED_VAR::SELECTED_STAR_DE:
			return coreLink->getSelectedStarDE();
		case SC_RESERVED_VAR::DATE_YEAR:
			return coreLink->getDateYear();
		case SC_RESERVED_VAR::DATE_MONTH:
			return coreLink->getDateMonth();
		case SC_RESERVED_VAR::DATE_DAY:
			return coreLink->getDateDay();
		case SC_RESERVED_VAR::DATE_HOUR:
			return coreLink->getDateHour();
		case SC_RESERVED_VAR::DATE_MINUTE:
			return coreLink->getDateMinute();
		case SC_RESERVED_VAR::BODY_SELECTED:
			return coreLink->getBodySelected();
		case SC_RESERVED_VAR::LANGUAGE:
			return coreLink->getLanguage();
		case SC_RESERVED_VAR::JOYPAD:
			return coreLink->isJoypadConnected;
		case SC_RESERVED_VAR::CURRENT_MODE:
			return coreLink->getCurrentModule();
		default:
			//std::cout << "Unknown reserved variable " << var << ". Default 0.0 is returned." << std::endl;
			cLog::get()->write("Unknown reserved variable " + var +". Default 0.0 is returned.", LOG_TYPE::L_WARNING , LOG_FILE::SCRIPT);
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
		case SC_RESERVED_VAR::BODY_SELECTED :
		case SC_RESERVED_VAR::LANGUAGE :
		case SC_RESERVED_VAR::SUN_AZIMUTH :
		case SC_RESERVED_VAR::DATE_YEAR :
		case SC_RESERVED_VAR::DATE_MONTH :
		case SC_RESERVED_VAR::DATE_DAY :
		case SC_RESERVED_VAR::DATE_HOUR :
		case SC_RESERVED_VAR::DATE_MINUTE :
			cLog::get()->write("No setter with reserved variable " + var +". Do nothing.", LOG_TYPE::L_WARNING , LOG_FILE::SCRIPT);
			//std::cout << "No setter with reserved variable " << var << ". Do nothing." << std::endl;
			break;
		default:
			cLog::get()->write("Unknown reserved variable " + var +". Do nothing.", LOG_TYPE::L_WARNING , LOG_FILE::SCRIPT);
			//std::cout << "Unknown reserved variable " << var << ". Do nothing." << std::endl;
			break;
	}
}
