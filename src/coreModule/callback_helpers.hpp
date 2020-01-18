//  boost CallbackHelpers.hpp  -------------------------------------------//

//  (C) Copyright Jesse Jones 2000. Permission to copy, use, modify, sell
//  and distribute this software is granted provided this copyright
//  notice appears in all copies. This software is provided "as is" without
//  express or implied warranty, and with no claim as to its suitability for
//  any purpose.

//  Revision History
//   20 Aug 2003 by Fabien Chéreau
//                Changed a ARG1 into ARG2 which was causing weird bugs..!
//                Removed an implicitly typename warning that occured with gcc 3.2.2
//                and changed file names for consistency
//   22 Nov 2000  1) Introduced numbered base classes to get compile time errors with too few arguments
//              2) Sprinkled the code with typename keywords for gcc.
//              3) Renamed unused unused_arg.
//              4) Method ctors no longer require raw pointers.
//   21 Nov 2000  collapsed callback0, callback1, callback2 into one class
//              (this was inspired by Doug Gregor's callback classes)
//   18 Nov 2000  Initial version

#ifndef BOOST_CALLBACKHELPERS_HPP
#define BOOST_CALLBACKHELPERS_HPP

namespace mBoost {
namespace details {

typedef int atomic_int;   // $$$ use something from the thread library here


// ===============================================================================
//  struct unused_arg
// ===============================================================================
struct unused_arg {};


// ===============================================================================
//  class BaseCallFunctor
// ===============================================================================
template <typename RETURN_TYPE, typename ARG1, typename ARG2>
class BaseCallFunctor {

public:
	virtual        ~BaseCallFunctor()         {}
	BaseCallFunctor()          {
		mRefCount = 1;
	}

	void     AddReference()               {
		++mRefCount;
	}
	void     RemoveReference()            {
		if (--mRefCount == 0) delete this;
	}
	// $$$ cloning would be a bit safer for functors with mutable state...

private:
	BaseCallFunctor(const BaseCallFunctor& rhs);   // $$$ use non_copyable
	BaseCallFunctor& operator=(const BaseCallFunctor& rhs);
private:
	atomic_int   mRefCount;
};


// ===============================================================================
//  0-arguments
// ===============================================================================
template <typename RETURN_TYPE>
class BaseCallFunctor0 : public BaseCallFunctor<RETURN_TYPE, unused_arg, unused_arg> {

public:
	virtual RETURN_TYPE Call() = 0;
};


template <typename FUNCTOR, typename RETURN_TYPE>
class CallFunctor0 : public BaseCallFunctor0<RETURN_TYPE> {

public:
	CallFunctor0(FUNCTOR functor)   : mFunctor(functor) {}

	virtual RETURN_TYPE Call()                      {
		return mFunctor();
	}

private:
	FUNCTOR      mFunctor;
};


// ===============================================================================
//  1-argument
// ===============================================================================
template <typename RETURN_TYPE, typename ARG1>
class BaseCallFunctor1 : public BaseCallFunctor<RETURN_TYPE, ARG1, unused_arg> {

public:
	virtual RETURN_TYPE Call(ARG1 arg1) = 0;
};


template <typename FUNCTOR, typename RETURN_TYPE, typename ARG1>
class CallFunctor1 : public BaseCallFunctor1<RETURN_TYPE, ARG1> {

public:
	CallFunctor1(FUNCTOR functor)   : mFunctor(functor) {}

	virtual RETURN_TYPE Call(ARG1 arg1)             {
		return mFunctor(arg1);
	}

private:
	FUNCTOR      mFunctor;
};


// ===============================================================================
//  2-arguments
// ===============================================================================
template <typename RETURN_TYPE, typename ARG1, typename ARG2>
class BaseCallFunctor2 : public BaseCallFunctor<RETURN_TYPE, ARG1, ARG2> {

public:
	virtual RETURN_TYPE Call(ARG1 arg1, ARG2 arg2) = 0;
};


template <typename FUNCTOR, typename RETURN_TYPE, typename ARG1, typename ARG2>
class CallFunctor2 : public BaseCallFunctor2<RETURN_TYPE, ARG1, ARG2> {

public:
	CallFunctor2(FUNCTOR functor)   : mFunctor(functor) {}

	virtual RETURN_TYPE Call(ARG1 arg1, ARG2 arg2) {
		return mFunctor(arg1, arg2);
	}

private:
	FUNCTOR      mFunctor;
};

// $$$ and call_functor3, call_functor4, etc


// ===============================================================================
//  Method Functors
//    $$$ note that the standard library only suffices for method_functor1
//    $$$ we can probably replace these with either the binder library or the
//    $$$ lambda library...
// ===============================================================================
template <typename RETURN_TYPE, typename OBJECT, typename METHOD>
class method_functor0 {
public:
	method_functor0(OBJECT object, METHOD method) : mObject(object), mMethod(method) {}

	RETURN_TYPE operator()() const      {
		return (mObject->*mMethod)();
	}
private:
	OBJECT    mObject;
	METHOD    mMethod;
};

template <typename RETURN_TYPE, typename OBJECT, typename METHOD, typename ARG1>
class method_functor1 {
public:
	method_functor1(OBJECT object, METHOD method) : mObject(object), mMethod(method) {}

	RETURN_TYPE operator()(ARG1 arg1) const      {
		return (mObject->*mMethod)(arg1);
	}
private:
	OBJECT    mObject;
	METHOD    mMethod;
};

template <typename RETURN_TYPE, typename OBJECT, typename METHOD, typename ARG1, typename ARG2>
class method_functor2 {
public:
	method_functor2(OBJECT object, METHOD method) : mObject(object), mMethod(method) {}

	RETURN_TYPE operator()(ARG1 arg1, ARG2 arg2) const      {
		return (mObject->*mMethod)(arg1, arg2);
	}
private:
	OBJECT    mObject;
	METHOD    mMethod;
};


// ===============================================================================
//  struct IF
// ===============================================================================
struct SelectThen {
	template<class Then, class Else>
	struct Result {
		typedef Then RET;
	};
};


struct SelectElse {
	template<class Then, class Else>
	struct Result {
		typedef Else RET;
	};
};


template<bool Condition>
struct Selector {
	typedef SelectThen RET;
};


template<>
struct Selector<false> {
	typedef SelectElse RET;
};


template<bool Condition, class Then, class Else>
struct IF {
	typedef typename Selector<Condition>::RET select;
	typedef typename mBoost::details::Selector<Condition>::RET::template Result<Then,Else>::RET RET;
};


// ===============================================================================
//  struct SWITCH
// ===============================================================================
const int DEFAULT = -32767;

const int NilValue = -32768;

struct NilCase  {
	enum {tag = NilValue};
	typedef NilCase RET;
};


template <int Tag,class Statement,class Next = NilCase>
struct CASE {
	enum {tag = Tag};
	typedef Statement statement;
	typedef Next next;
};


template <int Tag,class aCase>      // non partial specialization version...
struct SWITCH {
	typedef typename aCase::next nextCase;
	enum {  tag = aCase::tag,               // VC++ 5.0 doesn't operate directly on aCase::value in IF<>
	        nextTag = nextCase::tag,// Thus we need a little cheat
	        found = (tag == Tag || tag == DEFAULT)
	     };
	typedef typename IF<(nextTag == NilValue),
	        NilCase,
	        SWITCH<Tag,nextCase> >
	        ::RET nextSwitch;
	typedef typename IF<(found != 0),
	        typename aCase::statement,
	        typename nextSwitch::RET>
	        ::RET RET;
};


// ===============================================================================
//  struct GenerateBaseFunctor
// ===============================================================================
template <typename T>
struct is_used {
	enum {RET = 1};
};

template <>
struct is_used<unused_arg> {
	enum {RET = 0};
};


template <typename RETURN_TYPE, typename ARG1, typename ARG2>
struct GenerateBaseFunctor {
	enum {type = is_used<ARG1>::RET + is_used<ARG2>::RET};

	typedef BaseCallFunctor0<RETURN_TYPE> f0;
	typedef BaseCallFunctor1<RETURN_TYPE, ARG1> f1;
	typedef BaseCallFunctor2<RETURN_TYPE, ARG1, ARG2> f2;

	typedef typename SWITCH<(type),
	        CASE<0, f0,
	        CASE<1, f1,
	        CASE<2, f2> > > >::RET RET;
};


// ===============================================================================
//  struct GenerateFunctor
// ===============================================================================
template <typename FUNCTOR, typename RETURN_TYPE, typename ARG1, typename ARG2>
struct GenerateFunctor {
	enum {type = is_used<ARG1>::RET + is_used<ARG2>::RET};

	typedef CallFunctor0<FUNCTOR, RETURN_TYPE> f0;
	typedef CallFunctor1<FUNCTOR, RETURN_TYPE, ARG1> f1;
	typedef CallFunctor2<FUNCTOR, RETURN_TYPE, ARG1, ARG2> f2;

	typedef typename SWITCH<(type),
	        CASE<0, f0,
	        CASE<1, f1,
	        CASE<2, f2> > > >::RET RET;
};


// ===============================================================================
//  struct GenerateMethod
// ===============================================================================
template <typename RETURN_TYPE, typename OBJECT, typename METHOD, typename ARG1, typename ARG2>
struct GenerateMethod {
	enum {type = is_used<ARG1>::RET + is_used<ARG2>::RET};

	typedef method_functor0<RETURN_TYPE, OBJECT, METHOD> f0;
	typedef method_functor1<RETURN_TYPE, OBJECT, METHOD, ARG1> f1;
	typedef method_functor2<RETURN_TYPE, OBJECT, METHOD, ARG1, ARG2> f2;

	typedef typename SWITCH<type,
	        CASE<0, f0,
	        CASE<1, f1,
	        CASE<2, f2> > > >::RET RET;
};


}        // namespace details
}        // namespace mBoost

#endif   // BOOST_CALLBACKHELPERS_HPP

