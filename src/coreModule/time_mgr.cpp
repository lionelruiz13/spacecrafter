#include "coreModule/time_mgr.hpp"
#include "tools/fmath.hpp"
#include "appModule/space_date.hpp"
#include <string>
#include <sstream>


TimeMgr::TimeMgr()
{
	JDay=0.;
	FlagChangeTimeSpeed = 0;
	move_to_coef = 0;
	move_to_mult = 0;
	start_time_speed = 0;
	end_time_speed = 0;
	time_speed= JD_SECOND;
	// time_multiplier = 1.0;
	temp_time_velocity = time_speed;
}


TimeMgr::~TimeMgr()
{}


// Increment time
void TimeMgr::update(int delta_time)
{

	if (FlagChangeTimeSpeed) {
		move_to_mult += move_to_coef*delta_time;

		if ( move_to_mult >= 1) {
			move_to_mult = 1;
			FlagChangeTimeSpeed = 0;
			saveTimeSpeed();
			setTimePause(false);
		}
		time_speed = start_time_speed - move_to_mult*(start_time_speed-end_time_speed);
	}

	JDay+=time_speed*(double)delta_time/1000.;

	// Fix time limits to avoid ephemeris breakdowns
	if(JDay > SpaceDate::getMaxSimulationJD()) JDay = SpaceDate::getMaxSimulationJD();
	if(JDay < SpaceDate::getMinSimulationJD()) JDay = SpaceDate::getMinSimulationJD();
}


// move gradually to a new time speed
void TimeMgr::changeTimeSpeed(double _time_speed, double duration)
{
	FlagChangeTimeSpeed = 1;

	start_time_speed = time_speed;
	end_time_speed = _time_speed;

	move_to_coef = 1.0f/(duration*1000);
	move_to_mult = 0;
}

double TimeMgr::dateSunRise (double jd, double longitude, double latitude)
{
	//~ double jd = timeMgr->getJDay();
//	std::ostringstream os;
	//Sol mig
	double sidereal;
	double T;
	//~ double Le;
	double LST;
	double /*edt,*/c,r,l,lct,m;

	T = (jd - 2451545.0) / 36525.0;
	//~ Le = getObservatory()->getLongitude();
	/* calc mean angle */
	sidereal = 280.46061837 + (360.98564736629 * (jd - 2451545.0)) + (0.000387933 * T * T) - (T * T * T / 38710000.0);
	while (sidereal>=360) sidereal-=360;
	while (sidereal<0)    sidereal+=360;
	//~ LST=sidereal+Le;
	LST=sidereal+longitude;
	while (LST>=360) LST-=360;
	while (LST<0)    LST+=360;
	m=357.5291+0.98560028*(jd-2451545);
	c=1.9148*sin(m*3.1415926/180)+0.02*sin(2*m*3.1415926/180)+0.0003*sin(3*m*3.1415926/180)/12;
	l=280.4665+0.98564736*(jd-2451545)+c;
	r=-2.468*sin(2*l*3.1415926/180)+0.053*sin(4*l*3.1415926/180)-0.0014*sin(6*l*3.1415926/180);
	//edt=c+r;
	lct=-longitude/*Le*/+c+r;
	while (lct>360) lct-=360;
	while (lct<0) lct+=360;


	//Sol posta sortida
	double d2r = C_PI/180.0;
	double r2d = 180.0/C_PI;

	//~ double latitude = getObservatory()->getLatitude();
	//~ cout << "LATITUDE: " << latitude << endl;
	//~ double longitude = getObservatory()->getLongitude();
	//~ cout << "LONGITUDE: " << longitude << endl;
	//~ jd = timeMgr->getJDay();
	double n = jd-2451545.0009-double(longitude/360.0);
	//~ cout << "n: " << n << endl;

	//julian cycle
	n = round(n);

	//Approximate Solar Noon
	double temp = (longitude/360.f);
	//~ cout << "Temp: " << temp;
	//double J = (double)2451545.0009f+temp+double(n);
	double J = temp+n+2451545.0009f;
	//~ cout << "J: " << J << endl;

	//solar mean anomaly
	double M = fmodf((357.5291+0.98560028*(J-2451545.0)),360.0);
	//~ cout << "M: " << M << endl;

	//Equation of Center
	double C = (1.9148*sin(d2r*M))+(0.0200*sin(d2r*2.0*M))+(0.0003*sin(d2r*3.0*M));
	//~ cout << "C: " << C << endl;

	//Ecliptic Longitude
	double el = fmodf((M+102.9372+C+180.0),360.0);
	//~ cout << "el: " << el << endl;

	//Solar Transit
	double jt = J+(0.0053f*sin(d2r*M))-(0.0069f*sin(d2r*2.0f*el));
	//~ cout << "jt: " << jt << endl;

	//Declination of the Sun
	double s = r2d*asin(sin(d2r*el)*sin(d2r*23.45));
	//~ cout << "s: " << s << endl;

	//Hour Angle
	double Ho = (sin(-0.83*d2r)-(sin(d2r*latitude)*sin(d2r*s)))/(cos(d2r*latitude)*cos(d2r*s));
	if (abs(Ho)<=1) {
		double w = r2d*acos(Ho);
		//~ cout << "w: " << w << endl;

		double jset = 2451545.0009+double((w+longitude)/360.0)+n+(0.0053*sin(d2r*M))-(0.0069*sin(d2r*2.0*el));
		double jrise = jt-(jset-jt);
		//~ cout << "HORA SORTIDA: " << jrise << "HORA POSTA: " << jset;
		//core->setJDay(jset);
		double diferencia = ((1./24.0)*(longitude/15.));

		//~ cout << "HORA DIFERENCIA: " << jset << " " << diferencia << " " << jset-diferencia;

		double nova = jrise-(2.0*diferencia);
		//if (longitude >= 60) nova += 1;
		//if (abs(nova-jd)<1.0) return nova; else return J;
		return nova;
	} else return J-0.5;
}


double TimeMgr::dateSunSet (double jd, double longitude, double latitude)
{
	//~ double jd = timeMgr->getJDay();
	//std::ostringstream os;
	//Sol mig
	double sidereal;
	double T;
	double Le;
	double LST;
	double /*edt,*/ c,r,l,lct,m;

	T = (jd - 2451545.0) / 36525.0;
	Le = longitude; //getObservatory()->getLongitude();
	/* calc mean angle */
	sidereal = 280.46061837 + (360.98564736629 * (jd - 2451545.0)) + (0.000387933 * T * T) - (T * T * T / 38710000.0);
	while (sidereal>=360) sidereal-=360;
	while (sidereal<0)    sidereal+=360;
	LST=sidereal+Le;
	while (LST>=360) LST-=360;
	while (LST<0)    LST+=360;
	m=357.5291+0.98560028*(jd-2451545);
	c=1.9148*sin(m*3.1415926/180)+0.02*sin(2*m*3.1415926/180)+0.0003*sin(3*m*3.1415926/180)/12;
	l=280.4665+0.98564736*(jd-2451545)+c;
	r=-2.468*sin(2*l*3.1415926/180)+0.053*sin(4*l*3.1415926/180)-0.0014*sin(6*l*3.1415926/180);
	//edt=c+r;
	lct=-Le+c+r;
	while (lct>360) lct-=360;
	while (lct<0) lct+=360;

	//Sol posta sortida
	double d2r = C_PI/180.0;
	double r2d = 180.0/C_PI;

	//~ double latitude = getObservatory()->getLatitude();
	//~ cout << "LATITUDE: " << latitude << endl;
	//~ double longitude = getObservatory()->getLongitude();
	//~ cout << "LONGITUDE: " << longitude << endl;
	//~ jd = timeMgr->getJDay();
	double n = jd-2451545.0009-double(longitude/360.0);
	//~ cout << "n: " << n << endl;

	//julian cycle
	n = round(n);

	//Approximate Solar Noon
	double temp = (longitude/360.f);
	//~ cout << "Temp: " << temp;
	//double J = (double)2451545.0009f+temp+double(n);
	double J = temp+n+2451545.0009f;
	//~ cout << "J: " << J << endl;

	//solar mean anomaly
	double M = fmodf((357.5291+0.98560028*(J-2451545.0)),360.0);
	//~ cout << "M: " << M << endl;

	//Equation of Center
	double C = (1.9148*sin(d2r*M))+(0.0200*sin(d2r*2.0*M))+(0.0003*sin(d2r*3.0*M));
	//~ cout << "C: " << C << endl;

	//Ecliptic Longitude
	double el = fmodf((M+102.9372+C+180.0),360.0);
	//~ cout << "el: " << el << endl;

	//Solar Transit
	//~ double jt = J+(0.0053f*sin(d2r*M))-(0.0069f*sin(d2r*2.0f*el));
	//~ cout << "jt: " << jt << endl;

	//Declination of the Sun
	double s = r2d*asin(sin(d2r*el)*sin(d2r*23.45));
	//~ cout << "s: " << s << endl;

	//Hour Angle
	double Ho = (sin(-0.83*d2r)-(sin(d2r*latitude)*sin(d2r*s)))/(cos(d2r*latitude)*cos(d2r*s));
	if (abs(Ho)<=1) {
		double w = r2d*acos(Ho);
		//~ cout << "w: " << w << endl;

		double jset = 2451545.0009+double((w+longitude)/360.0)+n+(0.0053*sin(d2r*M))-(0.0069*sin(d2r*2.0*el));
		//~ double jrise = jt-(jset-jt);
		//~ cout << "HORA SORTIDA: " << jrise << "HORA POSTA: " << jset;
		//core->setJDay(jset);
		double diferencia = ((1./24.0)*(longitude/15.));

		//~ cout << "HORA DIFERENCIA: " << jset << " " << diferencia << " " << jset-diferencia;

		double nova = jset-(2.0*diferencia);
		return nova;
	} else return J+0.5;
}

double TimeMgr::dateSunMeridian (double jd, double longitude, double latitude)
{
	//~ double jd = timeMgr->getJDay();
	//std::ostringstream os;
	//Sol mig
	double sidereal;
	double T;
	double Le;
	double LST;
	double /*edt,*/ c,r,l,lct,m;

	T = (jd - 2451545.0) / 36525.0;
	Le = longitude; //getObservatory()->getLongitude();
	/* calc mean angle */
	sidereal = 280.46061837 + (360.98564736629 * (jd - 2451545.0)) + (0.000387933 * T * T) - (T * T * T / 38710000.0);
	while (sidereal>=360) sidereal-=360;
	while (sidereal<0)    sidereal+=360;
	LST=sidereal+Le;
	while (LST>=360) LST-=360;
	while (LST<0)    LST+=360;
	m=357.5291+0.98560028*(jd-2451545);
	c=1.9148*sin(m*3.1415926/180)+0.02*sin(2*m*3.1415926/180)+0.0003*sin(3*m*3.1415926/180)/12;
	l=280.4665+0.98564736*(jd-2451545)+c;
	r=-2.468*sin(2*l*3.1415926/180)+0.053*sin(4*l*3.1415926/180)-0.0014*sin(6*l*3.1415926/180);
	//edt=c+r;
	lct=-Le+c+r;
	while (lct>360) lct-=360;
	while (lct<0) lct+=360;

	//Sol posta sortida
	double d2r = C_PI/180.0;
	double r2d = 180.0/C_PI;

	//~ double latitude = getObservatory()->getLatitude();
	//~ cout << "LATITUDE: " << latitude << endl;
	//~ double longitude = getObservatory()->getLongitude();
	//~ cout << "LONGITUDE: " << longitude << endl;
	//~ jd = timeMgr->getJDay();
	double n = jd-2451545.0009-double(longitude/360.0);
	//~ cout << "n: " << n << endl;

	//julian cycle
	n = round(n);

	//Approximate Solar Noon
	double temp = (longitude/360.f);
	//~ cout << "Temp: " << temp;
	//double J = (double)2451545.0009f+temp+double(n);
	double J = temp+n+2451545.0009f;
	//~ cout << "J: " << J << endl;

	//solar mean anomaly
	double M = fmodf((357.5291+0.98560028*(J-2451545.0)),360.0);
	//~ cout << "M: " << M << endl;

	//Equation of Center
	double C = (1.9148*sin(d2r*M))+(0.0200*sin(d2r*2.0*M))+(0.0003*sin(d2r*3.0*M));
	//~ cout << "C: " << C << endl;

	//Ecliptic Longitude
	double el = fmodf((M+102.9372+C+180.0),360.0);
	//~ cout << "el: " << el << endl;

	//Solar Transit
	double jt = J+(0.0053f*sin(d2r*M))-(0.0069f*sin(d2r*2.0f*el));
	//~ cout << "jt: " << jt << endl;

	//Declination of the Sun
	double s = r2d*asin(sin(d2r*el)*sin(d2r*23.45));
	//~ cout << "s: " << s << endl;

	//Hour Angle
	double Ho = (sin(-0.83*d2r)-(sin(d2r*latitude)*sin(d2r*s)))/(cos(d2r*latitude)*cos(d2r*s));
	if (abs(Ho)<=1) {
		double w = r2d*acos(Ho);
		//~ cout << "w: " << w << endl;

		double jset = 2451545.0009+double((w+longitude)/360.0)+n+(0.0053*sin(d2r*M))-(0.0069*sin(d2r*2.0*el));
		double jrise = jt-(jset-jt);
		//~ cout << "HORA SORTIDA: " << jrise << "HORA POSTA: " << jset;
		//core->setJDay(jset);
		double diferencia = ((1./24.0)*(longitude/15.));

		//~ cout << "HORA DIFERENCIA: " << jset << " " << diferencia << " " << jset-diferencia;

		double nova = (jset+jrise-(4.0*diferencia))/2.0;
		return nova;
	} else return J;
}
