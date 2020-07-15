#ifndef _OBJECT_TYPE_H_
#define _OBJECT_TYPE_H_

#include "starModule/intrusive_ptr.hpp"

#include <limits>
#include <string>
#include <cstring>
#include "tools/utility.hpp"

enum OBJECT_TYPE {
	OBJECT_UNINITIALIZED,
	OBJECT_STAR,
	OBJECT_STARGALAXY,
	OBJECT_BODY,
	OBJECT_NEBULA,
	OBJECT_CONSTELLATION,
};

//! Intrusive pointer used to manage StelObject with smart pointers
class ObjectBase;
typedef IntrusivePtr<ObjectBase> ObjectBaseP;

#endif
