#ifndef VECTOR2_HPP_INCLUDED
#define VECTOR2_HPP_INCLUDED


template<class T> class Vector2 {
public:
	inline Vector2();
	inline Vector2(const Vector2<T>&);
	inline Vector2(T, T);
	inline Vector2(const T*);

	inline Vector2& operator=(const Vector2<T>&);
	inline Vector2& operator=(const T*);
	inline void set(T, T);

	inline bool operator==(const Vector2<T>&) const;
	inline bool operator!=(const Vector2<T>&) const;

	inline const T& operator[](int x) const;
	inline T& operator[](int);
	inline operator const T*() const;
	inline operator T*();

	inline Vector2& operator+=(const Vector2<T>&);
	inline Vector2& operator-=(const Vector2<T>&);
	inline Vector2& operator*=(T);
	inline Vector2& operator/=(T);

	inline Vector2 operator-(const Vector2<T>&) const;
	inline Vector2 operator+(const Vector2<T>&) const;

	inline Vector2 operator-() const;
	inline Vector2 operator+() const;

	inline Vector2 operator^(T) const;
	inline Vector2 operator*(T) const;
	inline Vector2 operator/(T) const;


	inline T dot(const Vector2<T>&) const;
	inline T getSin(const Vector2<T>&)const;

	inline T length() const;
	inline T lengthSquared() const;
	inline void normalize();

	T v[2];
};





//! default constructor. Set all to 0.
template<class T> Vector2<T>::Vector2()
{
	v[0]=0;
	v[1]=0;
}

//! Constructor from an array. Data are copied.
//! @param a the array to copy data from.
template<class T> Vector2<T>::Vector2(const Vector2<T>& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
}

//! constructor from 2 values.
//! x first value.
//! y second value.
template<class T> Vector2<T>::Vector2(T x, T y)
{
	v[0]=x;
	v[1]=y;
}

//! Constructor from an array. Data are copied.
//! @param a the array to copy data from.
template<class T> Vector2<T>::Vector2(const T*a)
{
	v[0]=a[0];
	v[1]=a[1];
}

//! = operator.
//! @param a the vector to copy.
//! @return *this
template<class T> Vector2<T>& Vector2<T>::operator=(const Vector2<T>& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	return *this;
}

//! = operator.
//! @param a the vector to copy.
//! @return *this
template<class T> Vector2<T>& Vector2<T>::operator=(const T* a)
{
	v[0]=a[0];
	v[1]=a[1];
	return *this;
}

//! whole vector setter.
//! @param x the first component.
//! @param y the second component.
template<class T> void Vector2<T>::set(T x, T y)
{
	v[0]=x;
	v[1]=y;
}

//! == operator
//! @param a the vector to compare to.
//! @return true if equals, false otherwise.
template<class T> bool Vector2<T>::operator==(const Vector2<T>& a) const
{
	return (v[0] == a.v[0] && v[1] == a.v[1]);
}

//! =! operator
//! @param a the vector to compare to.
//! @return false if equals, true otherwise.
template<class T> bool Vector2<T>::operator!=(const Vector2<T>& a) const
{
	return (v[0] != a.v[0] || v[1] != a.v[1]);
}

//! [] operator
//! @param x index of the desired element.
//! @return the element.
template<class T> const T& Vector2<T>::operator[](int x) const
{
	return v[x];
}

//! [] operator
//! @param x index of the desired element.
//! @return the element.
template<class T> T& Vector2<T>::operator[](int x)
{
	return v[x];
}

//! *operator
//! @return a pointer to the vector array.
template<class T> Vector2<T>::operator const T*() const
{
	return v;
}

//! *operator
//! @return a pointer to the vector array.
template<class T> Vector2<T>::operator T*()
{
	return v;
}

//!  += operator
//! @param the other vector to addition.
//! @return *this
template<class T> Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& a)
{
	v[0] += a.v[0];
	v[1] += a.v[1];
	return *this;
}

//!  -= operator
//! @param the other vector to subtract.
//! @return *this
template<class T> Vector2<T>& Vector2<T>::operator-=(const Vector2<T>& a)
{
	v[0] -= a.v[0];
	v[1] -= a.v[1];
	return *this;
}

//!  *= operator
//! @param the other vector to multiply.
//! @return *this
template<class T> Vector2<T>& Vector2<T>::operator*=(T s)
{
	v[0] *= s;
	v[1] *= s;
	return *this;
}
//!  -= operator
//! @return - this vector.
template<class T> Vector2<T> Vector2<T>::operator-() const
{
	return Vector2<T>(-v[0], -v[1]);
}
//!  += operator
//! @return this vector.
template<class T> Vector2<T> Vector2<T>::operator+() const
{
	return *this;
}

//!  + operator
//! @param b the other vector to add
//! @return this + b.
template<class T> Vector2<T> Vector2<T>::operator+(const Vector2<T>& b) const
{
	return Vector2<T>(v[0] + b.v[0], v[1] + b.v[1]);
}

//!  - operator
//! @param b the other vector to subtract.
//! @return this - b.
template<class T> Vector2<T> Vector2<T>::operator-(const Vector2<T>& b) const
{
	return Vector2<T>(v[0] - b.v[0], v[1] - b.v[1]);
}
//! cross product
//! @param b the other vector to calaculate the cross product.
//!
template<class T> Vector2<T> Vector2<T>::operator^(T b) const
{
	Vector2<T>r(v[0]*b.v[1]-v[1]*b.v[0]);
	return r;
}
//!  * operator
//! @param b the other vector to multiply
//! @return this * b.
template<class T> Vector2<T> Vector2<T>::operator*(T s) const
{
	return Vector2<T>(s * v[0], s * v[1]);
}
//!  / operator
//! @param b the other vector to divide
//! @return this / b.
template<class T> Vector2<T> Vector2<T>::operator/(T s) const
{
	return Vector2<T>(v[0]/s, v[1]/s);
}

//! dot product.
//! @param b the other vector
//! @return the dot product.
template<class T> T Vector2<T>::dot(const Vector2<T>& b) const
{
	return v[0] * b.v[0] + v[1] * b.v[1];
}
//! Gives the sinus of the angle between two NORMALIZED vectors.
//! @param b the other vector
//! @return the sinus.
template<class T> T Vector2<T>::getSin(const Vector2<T>&b)const
{
	Vector2 cross= (*this)^b;
	return cross.length();
}

//! Gives the length of a vector.
//! @return the length.
template<class T> T Vector2<T>::length() const
{
	return (T) sqrt(v[0] * v[0] + v[1] * v[1]);
}
//! Gives the squared length of a vector.
//! This is useful when you don't want to use sqrt to compare distances.
//! @return the squared length.
template<class T> T Vector2<T>::lengthSquared() const
{
	return v[0] * v[0] + v[1] * v[1];
}
//! normalize this vector.
template<class T> void Vector2<T>::normalize()
{
	T s = (T) 1 / sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] *= s;
	v[1] *= s;
}


//! vector multiplication by an other vector.
//! @param a the first vector to use.
//! @param b the second vector to use.
//! @return a*b
template<class T> inline
T operator*(const Vector2<T>&a,const Vector2<T>&b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1];
}

//! vector multiplication by a scalar.
//! @param s the scalar to use.
//! @param v the vector to use.
//! @return s*v
template<class T> inline
Vector2<T> operator*(T s,const Vector2<T>&v)
{
	return Vector2<T>(s*v[0],s*v[1]);
}



#endif // VECTOR2_HPP_INCLUDED

































