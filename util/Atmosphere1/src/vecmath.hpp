//!
//!
//! Copyright (C) 2003 Fabien Chereau
//! Copyright (C) 2009 Digitalis Education Solutions, Inc.
//! Copyright (C) 2015 Olivier Nivoix
//! Copyright (C) 2016 Jérôme Lartillot
//!
//! This program is free software; you can redistribute it and/or
//! modify it under the terms of the GNU General Public License
//! as published by the Free Software Foundation; either version 3
//! of the License, or (at your option) any later version.
//!
//! This program is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//! GNU General Public License for more details.
//!
//! You should have received a copy of the GNU General Public License
//! along with this program; if not, write to the Free Software
//! Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//!
//! Stellarium360 is a free open project of of LSS team
//! See the TRADEMARKS file for free open project usage requirements.
//!
//!
//! @file vecmath.hpp vector2.hpp vector3.hpp vector4.hpp matrix4.hpp
//! @author Jérôme Lartillot
//! @date 04/05/2016
//!
//! @section description Description
//!
//! Template vector and matrix library.
//! Use OpenGL compatible ordering ie. you can pass a matrix or vector to
//! openGL functions without changes in the ordering
//!
//! @section modifications Last modifications:
//!
//! Jérôme Lartillot: added rotation functions, vector constructors and basic functions.
//! also added toPlane functions and some other related stuff..
//!

#ifndef _VECMATH_HPP_INCLUDED
#define _VECMATH_HPP_INCLUDED

//#include "main.hpp"

#include <cmath>
#include <cstring>
#include <cstdio>
#include <iostream>

template<class T> class Vector2;
template<class T> class Vector3;
template<class T> class Vector4;
template<class T> class Matrix4;

typedef Vector2<float>	Vec2f;
typedef Vector2<double>	Vec2d;
typedef Vector2<int>	Vec2i;
typedef Vector2<size_t> Vec2size_t;

typedef Vector3<float>	Vec3f;
typedef Vector3<double>	Vec3d;
typedef Vector3<int>	Vec3i;

typedef Vector4<double>	Vec4d;
typedef Vector4<float>	Vec4f;
typedef Vector4<int>	Vec4i;

typedef Matrix4<float>	Mat4f;
typedef Matrix4<double>	Mat4d;

#include "vector2.hpp"

#include "vector3.hpp"

#include "vector4.hpp"

#include "matrix4.hpp"


#endif // _VECMATH_HPP_INCLUDED
