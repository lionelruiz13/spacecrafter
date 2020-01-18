#ifndef EVENT_HPP
#define EVENT_HPP

class Event{

public :
    enum Event_Type { 
        Not_Set,
        Altitude_Changed,
        Atmosphere,
        E_Altimetre,
        //....
    };
    
    Event(){ 
        type = Not_Set;
    }
    
    Event(Event_Type _type, bool _bvalue){
		type=_type;
		bValue = _bvalue;
	}
    
    Event(Event_Type _type, float _fvalue){
		type=_type;
		fValue = _fvalue;
	}
    
    Event(Event_Type _type, double _dvalue){
		type=_type;
		dValue = _dvalue;
	}

    Event_Type getEventType(){
        return type;
    }
    
    double getDouble() {
		return dValue;
	}
    
    float getFloat() {
		return fValue;
	}
	
	double getBool() {
		return bValue;
	}
    
protected :
    Event_Type type;
    double dValue;
    float fValue;
    bool bValue;
};

#endif
