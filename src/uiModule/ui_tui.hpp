/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

// Class which manages a Text User Interface "widgets"

#ifndef _TUI_H_
#define _TUI_H_

#include <ctime>
#include <SDL2/SDL.h>
#include <set>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <sstream>

#include "coreModule/callbacks.hpp"
#include "tools/vecmath.hpp"
#include "tools/utility.hpp"

class Translator;

namespace s_tui {
enum S_TUI_VALUE {
	S_TUI_PRESSED,
	S_TUI_RELEASED
};


static const std::string startActive("<");
static const std::string stopActive(">");

static const std::string startTui("");
static const std::string endTui("");

// Base class. Note that the method bool isEditable() has to be overrided by returning true
// for all the non passives components.
class Component {
public:
	Component() : active(false) {
		;
	}
	virtual ~Component() {
		;
	}
	virtual std::string getString() {
		return std::string();
	}
	virtual std::string getCleanString();
	// Return true if key signal intercepted, false if not
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE) {
		return false;
	}
	virtual bool isEditable() const {
		return false;
	}
	void setActive(bool a) {
		active = a;
	}
	bool getActive() const {
		return active;
	}
protected:
	bool active;
};

// simple display a std::string component
class Display : public Component {
public:
	Display(std::string _label, std::string _value) : Component(), label(_label), value(_value) {
		;
	}
	std::string getString() {
		return label + value;
	}
	std::string getCleanString() {
		return label + value;
	}
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	std::string label;
	std::string value;
};

// Store a mBoost::callback on a function taking no parameters
class CallbackComponent : public Component {
public:
	virtual void setOnChangeCallback(const mBoost::callback<void>& c) {
		onChangeCallback = c;
	}
protected:
	mBoost::callback<void> onChangeCallback;
};

// Manage lists of components
class Container : public CallbackComponent {
public:
	virtual ~Container();
	virtual std::string getString();
	virtual void addComponent(Component*);
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
protected:
	std::list<Component*> childs;
};

// Component which manages 2 states
class Bistate : public CallbackComponent {
public:
	Bistate(bool init_state = false) : CallbackComponent(), state(init_state) {
		;
	}
	virtual std::string getString() {
		return state ? string_activated : string_disabled;
	}
	bool getValue() const {
		return state;
	}
	void setValue(bool s) {
		state = s;
	}
protected:
	std::string string_activated;
	std::string string_disabled;
	bool state;
};

// Component which manages integer value
class Integer : public CallbackComponent {
public:
	Integer(int init_value = 0) : CallbackComponent(), value(init_value) {
		;
	}
	virtual std::string getString();
	int getValue() const {
		return value;
	}
	void setValue(int v) {
		value = v;
	}
protected:
	int value;
};

// Boolean item widget. The mBoost::callback function is called when the state is changed
class BooleanItem : public Bistate {
public:
	BooleanItem(bool init_state = false, const std::string& _label = std::string(),
	             const std::string& _string_activated = std::string("ON"),
	             const std::string& string_disabled  = std::string("OFF"));
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual std::string getString();
	virtual bool isEditable() const {
		return true;
	}
	void setLabel(const std::string& _label, const std::string& _active, const std::string& _disabled) {
		label = _label;
		string_activated=_active;
		string_disabled=_disabled;
	}
protected:
	std::string label;
};

// Component which manages decimal (double) value
class Decimal : public CallbackComponent {
public:
	Decimal(double init_value = 0.) : CallbackComponent(), value(init_value) {
		;
	}
	virtual std::string getString();
	double getValue() const {
		return value;
	}
	void setValue(double v) {
		value = v;
	}
protected:
	double value;
};

// Integer item widget. The mBoost::callback function is called when the value is changed
class IntegerItem : public Integer {
public:
	IntegerItem(int _min, int _max, int init_value, const std::string& _label = std::string()) :
		Integer(init_value), numInput(false), mmin(_min), mmax(_max), label(_label) {
		;
	}
	virtual std::string getString();
	virtual bool isEditable() const {
		return true;
	}
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	void setLabel(const std::string& _label) {
		label = _label;
	}
	virtual int increment() {
		return ++value;
	}
	virtual int decrement() {
		return --value;
	}

	int operator%(int i) {
		return value %i;
	}
protected:
	bool numInput;
	std::string strInput;
	int mmin, mmax;
	std::string label;
};

// logarithmic steps
class IntegerItemLogstep : public IntegerItem {
public:
	IntegerItemLogstep(int _min, int _max, int init_value, const std::string& _label = std::string()) :
		IntegerItem(_min, _max, init_value, _label) {
		;
	}

	virtual int increment();
	virtual int decrement();
};


// added wrap option (for latitude for example)
// Decimal item widget. The mBoost::callback function is called when the value is changed
class DecimalItem : public Decimal {
public:
	DecimalItem(double _min, double _max, double init_value, const std::string& _label = std::string(), double _delta = 1.0, bool _wrap = false) :
		Decimal(init_value), numInput(false), mmin(_min), mmax(_max), label(_label), delta(_delta) {
		wrap = _wrap;
	}
	virtual std::string getString();
	virtual bool isEditable() const {
		return true;
	}
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	bool numInput;
	std::string strInput;
	double mmin, mmax;
	std::string label;
	double delta;
	bool wrap;
};

// Passive widget which only display text
class LabelItem : public Component {
public:
	LabelItem(const std::string& _label) : Component(), label(_label) {
		;
	}
	virtual std::string getString() {
		return label;
	}
	void setLabel(const std::string& s) {
		label=s;
	}
protected:
	std::string label;
};

// Manage list of components with one of them selected.
// Can navigate thru the components list with the arrow keys
class Branch : public Container {
public:
	Branch();
	virtual std::string getString();
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual void addComponent(Component*);
	virtual Component* getCurrent() const {
		if (current==childs.end()) return nullptr;
		else return *current;
	}
	virtual bool setValue(const std::string&);
	virtual bool setValueSpecialSlash(const std::string&);
protected:
	std::list<Component*>::const_iterator current;
};

// Base widget used for tree construction. Can navigate thru the components list with the arrow keys.
// Activate the currently edited widget.
class MenuBranch : public Branch {
public:
	MenuBranch(const std::string& s);
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual std::string getString();
	virtual bool isEditable() const {
		return true;
	}
	std::string getLabel() const {
		return label;
	}
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	std::string label;
	bool isNavigating;
	bool isEditing;
};

// Widget quite like Menu Branch but always navigating, and always display the label
class MenuBranchItem : public Branch {
public:
	MenuBranchItem(const std::string& s);
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual std::string getString();
	virtual bool isEditable() const {
		return true;
	}
	std::string getLabel() const {
		return label;
	}
protected:
	std::string label;
	bool isEditing;
};


// Widget used to set time and date. The internal format is the julian day notation
class TimeItem : public CallbackComponent {
public:
	TimeItem(const std::string& _label = std::string(), double _JD = 2451545.0);
	~TimeItem();
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual std::string getString();
	virtual std::string getDateString();
	virtual bool isEditable() const {
		return true;
	}
	double getJDay() const {
		return JD;
	}
	void setJDay(double jd) {
		JD = jd;
	}
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	void computeYmdhms();
	void computeJD();
	double JD;
	double second;
	IntegerItem* current_edit;	// 0 to 5 year to second
	std::string label;
	IntegerItem *y, *m, *d, *h, *mn, *s;
};


// Widget which simply launch the mBoost::callback when the user press enter
class ActionItem : public CallbackComponent {
public:
	ActionItem(const std::string& _label = "", const std::string& sp1 = "Do", const std::string& sp2 = "Done") :
		CallbackComponent(), label(_label), englishStringPrompt1(sp1), englishStringPrompt2(sp2) {
		tempo = 0;
	}
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual std::string getString();
	virtual bool isEditable() const {
		return true;
	}
	void setLabel(const std::string& _label) {
		label = _label;
	}
	virtual void translateActions();

protected:
	std::string label;
	std::string englishStringPrompt1;
	std::string englishStringPrompt2;
	std::string string_prompt1;
	std::string string_prompt2;
	time_t tempo;
};

// Same as before but ask for a confirmation
class ActionConfirmItem : public ActionItem {
public:
	ActionConfirmItem(const std::string& _label = "", const std::string& sp1 = "Do", const std::string& sp2 = "Done",	const std::string& sc = "Are you sure ?") :
		ActionItem(_label, sp1, sp2), isConfirming(false), englishStringConfirm(sc) {
		;
	}
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual std::string getString();
	virtual void translateActions();

protected:
	bool isConfirming;
	std::string englishStringConfirm;
	std::string string_confirm;
};

// List item widget. The mBoost::callback function is called when the selected item changes
template <class T>
class MultiSetItem : public CallbackComponent {
public:
	MultiSetItem(const std::string& _label = std::string()) : CallbackComponent(), label(_label) {
		current = items.end();
	}
	MultiSetItem(const MultiSetItem& m) : CallbackComponent(), label(m.label) {
		setCurrent(m.getCurrent());
	}
	virtual std::string getString() {
		if (current==items.end()) return label;
		return label + (active ? startActive : "") + *current + (active ? stopActive : "");
	}
	virtual bool isEditable() const {
		return true;
	}
	virtual bool onKey(SDL_Scancode k, S_TUI_VALUE v) {
		if (current==items.end() || v==S_TUI_RELEASED) return false;
		if (k==SDL_SCANCODE_RETURN) {
			if (!onTriggerCallback.empty()) onTriggerCallback();
			return false;
		}
		if (k==SDL_SCANCODE_UP) {
			if (current!=items.begin()) --current;
			else current = --items.end();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDL_SCANCODE_DOWN) {
			if (current!= --items.end()) ++current;
			else current = items.begin();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDL_SCANCODE_LEFT || k==SDL_SCANCODE_ESCAPE) return false;
		return false;
	}
	void addItem(const T& newitem) {
		items.insert(newitem);
		if (current==items.end()) current = items.begin();
	}
	void addItemList(const std::string& s) {
		std::istringstream is(s);
		T elem;
		while (getline(is, elem)) {
			addItem(elem);
		}
	}
	void replaceItemList(std::string s, int selection) {
		items.clear();
		addItemList(s);
		current = items.begin();
		for (int j=0; j<selection; j++) {
			++current;
		}
	}
	const T& getCurrent() const {
		if (current==items.end()) return emptyT;
		else return *current;
	}
	void setCurrent(const T& i) {
		current = items.find(i);

		// if not found, set to first item!
		if (current==items.end()) {
			current = items.begin();
			if (!onChangeCallback.empty()) onChangeCallback();
		}
	}
	bool setValue(const T& i) {
		if (items.find(i) == items.end()) return false;
		else current = items.find(i);
		return true;
	}
	std::string getLabel() const {
		return label;
	}
	virtual void setOnTriggerCallback(const mBoost::callback<void>& c) {
		onTriggerCallback = c;
	}
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	T emptyT;
	std::multiset<T> items;
	typename std::multiset<T>::iterator current;
	std::string label;
	mBoost::callback<void> onTriggerCallback;
};

// Specialization for strings because operator wstream << std::string does not exists..
template<>
std::string MultiSetItem<std::string>::getString();


// List item widget with separation between UI keys (will be translated) and code value (never translated).
// Assumes one-to-one mapping of keys to values
// The mBoost::callback function is called when the selected item changes
template <class T>
class MultiSet2Item : public CallbackComponent {
public:
	MultiSet2Item(const std::string& _label = std::string()) : CallbackComponent(), label(_label) {
		current = items.end();
	}
	MultiSet2Item(const MultiSet2Item& m) : CallbackComponent(), label(m.label) {
		setCurrent(m.getCurrent());
	}
	virtual std::string getString() {
		if (current==items.end()) return label;
		return label + (active ? startActive : "") + *current + (active ? stopActive : "");
	}
	virtual bool isEditable() const {
		return true;
	}
	virtual bool onKey(SDL_Scancode k, S_TUI_VALUE v) {
		if (current==items.end() || v==S_TUI_RELEASED) return false;
		if (k==SDL_SCANCODE_RETURN) {
			if (!onTriggerCallback.empty()) onTriggerCallback();
			return false;
		}
		if (k==SDL_SCANCODE_UP) {
			if (current!=items.begin()) --current;
			else current = --items.end();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDL_SCANCODE_DOWN) {
			if (current!= --items.end()) ++current;
			else current = items.begin();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDL_SCANCODE_LEFT || k==SDL_SCANCODE_ESCAPE) return false;
		return false;
	}
	void addItem(const T& newkey, const T& newvalue) {
		items.insert(newkey);
		value[newkey] = newvalue;
		if (current==items.end()) current = items.begin();
	}
	void addItemList(std::string s) { // newline delimited, key and value alternate
		std::istringstream is(s);
		T key, value;
		while (getline(is, key) && getline(is, value)) {
			addItem(key, value);
		}
	}
	void replaceItemList(std::string s, int selection) {
		items.clear();
		value.clear();
		current = items.end();
		addItemList(s);
		current = items.begin();
		for (int j=0; j<selection; j++) {
			++current;
		}
	}
	const T& getCurrent() {
		if (current==items.end()) return emptyT;
		else return value[(*current)];
	}
	void setCurrent(const T& i) {  // set by value, not key

		typename std::multiset<T>::iterator iter;

		bool found =0;
		for (iter=items.begin(); iter!=items.end(); iter++ ) {
			if ( i == value[(*iter)]) {
				current = iter;
				found = 1;
				break;
			}
		}

		if (!found) current = items.begin();
		if (!onChangeCallback.empty()) onChangeCallback();
	}

	bool setValue(const T& i) {
		typename std::multiset<T>::iterator iter;

		bool found =0;
		for (iter=items.begin(); iter!=items.end(); iter++ ) {
			if ( i == value[(*iter)]) {
				current = iter;
				found = 1;
				break;
			}
		}
		return found;
	}
	std::string getLabel() const {
		return label;
	}
	virtual void setOnTriggerCallback(const mBoost::callback<void>& c) {
		onTriggerCallback = c;
	}
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	T emptyT;
	std::multiset<T> items;
	typename std::multiset<T>::iterator current;
	std::string label;
	mBoost::callback<void> onTriggerCallback;
	std::map<T, T> value;  // hash of key, value pairs
};


// List item widget. NOT SORTED (FIFO). The mBoost::callback function is called when the selected item changes
template <class T>
class ListItem : public CallbackComponent {
public:
	ListItem(const std::string& _label = std::string()) : CallbackComponent(), label(_label) {
		current = items.end();
	}
	ListItem(const ListItem& m) : CallbackComponent(), label(m.label) {
		setCurrent(m.getCurrent());
	}
	virtual std::string getString() {
		if (current==items.end()) return label;
		return label + (active ? startActive : "") + *current + (active ? stopActive : "");
	}
	virtual bool isEditable() const {
		return true;
	}
	virtual bool onKey(SDL_Scancode k, S_TUI_VALUE v) {
		if (current==items.end() || v==S_TUI_RELEASED) return false;
		if (k==SDL_SCANCODE_RETURN) {
			if (!onTriggerCallback.empty()) onTriggerCallback();
			return false;
		}
		if (k==SDL_SCANCODE_UP) {
			if (current!=items.begin()) --current;
			else current = --items.end();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDL_SCANCODE_DOWN) {
			if (current!= --items.end()) ++current;
			else current = items.begin();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDL_SCANCODE_LEFT || k==SDL_SCANCODE_ESCAPE) return false;
		return false;
	}
	void addItem(const T& newitem) {
		items.push_back(newitem);

		if (current==items.end()) current = items.begin();
	}
	void addItemList(const std::string& s) {
		std::istringstream is(s);
		T elem;
		while (getline(is, elem)) {
			addItem(elem);
		}
	}
	void replaceItemList(std::string s, int selection) {
		items.clear();
		current = items.end();
		addItemList(s);

		current = items.begin();
		for (int j=0; j<selection; j++) {
			++current;
		}
	}
	const T& getCurrent() const {
		if (current==items.end()) return emptyT;
		else return *current;
	}
	void setCurrent(const T& i) {

		typename std::list<T>::iterator iter;

		bool found =0;
		for (iter=items.begin(); iter!=items.end(); iter++ ) {
			if ( i == (*iter)) {
				current = iter;
				found = 1;
				break;
			}
		}
		if (!found) current = items.begin();
	}
	bool setValue(const T& i) {
		if (items.find(i) == items.end()) return false;
		else current = items.find(i);
		return true;
	}
	std::string getLabel() const {
		return label;
	}
	virtual void setOnTriggerCallback(const mBoost::callback<void>& c) {
		onTriggerCallback = c;
	}
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	T emptyT;
	std::list<T> items;
	typename std::list<T>::iterator current;
	std::string label;
	mBoost::callback<void> onTriggerCallback;
};


// Widget used to set time zone. Initialized from a file of type /usr/share/zoneinfo/zone.tab
class TimeZoneitem : public CallbackComponent {
public:
	TimeZoneitem(const std::string& zonetab_file, const std::string& _label = std::string());
	virtual ~TimeZoneitem();
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual std::string getString();
	virtual bool isEditable() const {
		return true;
	}
	std::string gettz(); // should be const but gives a boring error...
	void settz(const std::string& tz);
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	MultiSetItem<std::string> continents_names;
	std::map<std::string, MultiSetItem<std::string> > continents;
	std::string label;
	MultiSetItem<std::string>* current_edit;
};


// Widget used to edit a vector
class VectorItem : public CallbackComponent {
public:
	VectorItem(const std::string& _label = std::string(), Vec3d _init_vector = Vec3d(0,0,0));
	~VectorItem();
	virtual bool onKey(SDL_Scancode, S_TUI_VALUE);
	virtual std::string getString();
	virtual bool isEditable() const {
		return true;
	}
	Vec3d getVector() const {
		return Vec3d( a->getValue(), b->getValue(), c->getValue());
	}
	void setVector(Vec3d _vector) {
		a->setValue(_vector[0]);
		b->setValue(_vector[1]);
		c->setValue(_vector[2]);
	}
	void setLabel(const std::string& _label) {
		label = _label;
	}
protected:
	DecimalItem* current_edit;	// 0 to 2
	std::string label;
	DecimalItem *a, *b, *c;
};

}

#endif // _TUI_H_
