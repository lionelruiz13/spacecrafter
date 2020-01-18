#ifndef BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
#define BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
//
// This file was created from the source of the boost library
//
//  intrusive_ptr.hpp
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/intrusive_ptr.html for documentation.
//


#include <functional>
#include <iosfwd>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <cassert>


template<class T> class IntrusivePtr {

public:
	IntrusivePtr() : px( 0 ) {
	}

	IntrusivePtr( T * p, bool add_ref = true ): px( p ) {
		if( px != 0 && add_ref ) intrusivePtrAddRef( px );
	}

	IntrusivePtr(IntrusivePtr const & rhs): px( rhs.px ) {
		if( px != 0 ) intrusivePtrAddRef( px );
	}

	~IntrusivePtr() {
		if( px != 0 ) intrusivePtrRelease( px );
	}

	IntrusivePtr & operator=(IntrusivePtr const & rhs) {
		IntrusivePtr(rhs).swap(*this);
		return *this;
	}

	IntrusivePtr & operator=(T * rhs) {
		IntrusivePtr(rhs).swap(*this);
		return *this;
	}

	void reset() {
		IntrusivePtr().swap( *this );
	}

	void reset( T * rhs ) {
		IntrusivePtr( rhs ).swap( *this );
	}

	T * get() const {
		return px;
	}

	T & operator*() const {
		((px != 0) ? static_cast<void> (0) : assert(true));
		return *px;
	}

	T * operator->() const {
		((px != 0) ? static_cast<void> (0) : assert(true));
		return px;
	}

	bool operator! () const {
		return px == 0;
	}

	void swap(IntrusivePtr & rhs) {
		T * tmp = px;
		px = rhs.px;
		rhs.px = tmp;
	}

private:
	T * px;
};

#endif  // #ifndef BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
