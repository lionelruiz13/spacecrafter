#ifndef VECTOR3_HPP_INCLUDED
#define VECTOR3_HPP_INCLUDED


template<class T> class Vector3 {
public:
	inline Vector3();
	inline Vector3(const Vector3&);
	inline Vector3(const Vector4<T>&);
	template <class T2> inline Vector3(const Vector3<T2>&);
	inline Vector3(T, T, T);

	inline Vector3& operator=(const Vector3&);
	inline Vector3& operator=(const T*);
	template <class T2> inline Vector3& operator=(const Vector3<T2>&);
	inline void set(T, T, T);

	inline bool operator==(const Vector3<T>&) const;
	inline bool operator!=(const Vector3<T>&) const;

	inline T& operator[](int);
	inline const T& operator[](int) const;
	inline operator const T*() const;
	inline operator T*();

	inline Vector3& operator+=(const Vector3<T>&);
	inline Vector3& operator-=(const Vector3<T>&);
	inline Vector3& operator*=(T);
	inline Vector3& operator/=(T);

	inline Vector3 operator-(const Vector3<T>&) const;
	inline Vector3 operator+(const Vector3<T>&) const;

	inline Vector3 operator-() const;
	inline Vector3 operator+() const;

	inline Vector3 operator*(T) const;
	inline Vector3 operator/(T) const;

	static Vector3 null();

	inline T dot(const Vector3<T>&) const;
	inline Vector3 operator^(const Vector3<T>&) const;
	Vector3<float> convert() const;
	// Distance in radian between two
	inline T angle(const Vector3<T>&) const;

	inline T length() const;
	inline T lengthSquared() const;
	inline void normalize();
	inline void toPlane(const Vector4<T>&);
	inline void toPlane(const Vector3<T>&);

	inline void transfo4d(const Mat4d&);
	inline void transfo4d(const Mat4f&);
	T v[3];		// The 3 values
};


//! Convert to a float vector
template<class T> Vector3<float> Vector3<T>::convert() const
{
	return Vector3<float>(v[0], v[1], v[2] );
}




//! default constructor. Set all to 0.
template<class T> Vector3<T>::Vector3()
{
	v[0]=0;
	v[1]=0;
	v[2]=0;
}

//! Constructor from an array. Data are copied.
//! @param a the array to copy data from.
template<class T> Vector3<T>::Vector3(const Vector3& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
}

//! Copy constructor.
//! @param a the vector to copy.
template<class T> Vector3<T>::Vector3(const Vector4<T>&a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
}

//! Copy constructor.
//! @param a the vector to copy.
template<class T> template<class T2> Vector3<T>::Vector3(const Vector3<T2>& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
}

//! constructor from 3 values.
//! x first value.
//! y second value.
//! z third value.
template<class T> Vector3<T>::Vector3(T x, T y, T z)
{
	v[0]=x;
	v[1]=y;
	v[2]=z;
}

//! = operator.
//! @param a the vector to copy.
//! @return *this
template<class T> Vector3<T>& Vector3<T>::operator=(const Vector3& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
	return *this;
}

//! = operator.
//! @param a the vector to copy.
//! @return *this
template<class T> template <class T2> Vector3<T>& Vector3<T>::operator=(const Vector3<T2>& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
	return *this;
}

//! = operator from array.
//! @param a the array to copy.
//! @return *this
template<class T> Vector3<T>& Vector3<T>::operator=(const T* a)
{
	v[0]=a[0];
	v[1]=a[1];
	v[2]=a[2];
	return *this;
}

//! whole vector setter.
//! @param x the first component.
//! @param y the second component.
//! @param z the third component.
template<class T> void Vector3<T>::set(T x, T y, T z)
{
	v[0]=x;
	v[1]=y;
	v[2]=z;
}

//! == operator
//! @param a the vector to compare to.
//! @return true if equals, false otherwise.
template<class T> bool Vector3<T>::operator==(const Vector3<T>& a) const
{
	return (v[0] == a.v[0] && v[1] == a.v[1] && v[2] == a.v[2]);
}

//! =! operator
//! @param a the vector to compare to.
//! @return false if equals, true otherwise.
template<class T> bool Vector3<T>::operator!=(const Vector3<T>& a) const
{
	return (v[0] != a.v[0] || v[1] != a.v[1] || v[2] != a.v[2]);
}

//! [] operator
//! @param x index of the desired element.
//! @return the element.
template<class T> T& Vector3<T>::operator[](int x)
{
	return v[x];
}

//! [] operator
//! @param x index of the desired element.
//! @return the element.
template<class T> const T& Vector3<T>::operator[](int x) const
{
	return v[x];
}

//! *operator
//! @return a pointer to the vector array.
template<class T> Vector3<T>::operator const T*() const
{
	return v;
}

//! *operator
//! @return a pointer to the vector array.
template<class T> Vector3<T>::operator T*()
{
	return v;
}

//!  += operator
//! @param the other vector to addition.
//! @return *this
template<class T> Vector3<T>& Vector3<T>::operator+=(const Vector3<T>& a)
{
	v[0] += a.v[0];
	v[1] += a.v[1];
	v[2] += a.v[2];
	return *this;
}

//!  -= operator
//! @param the other vector to subtract.
//! @return *this
template<class T> Vector3<T>& Vector3<T>::operator-=(const Vector3<T>& a)
{
	v[0] -= a.v[0];
	v[1] -= a.v[1];
	v[2] -= a.v[2];
	return *this;
}

//!  *= operator
//! @param the other vector to multiply.
//! @return *this
template<class T> Vector3<T>& Vector3<T>::operator*=(T s)
{
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
	return *this;
}
//!  -= operator
//! @return - this vector.
template<class T> Vector3<T> Vector3<T>::operator-() const
{
	return Vector3<T>(-v[0], -v[1], -v[2]);
}
//!  += operator
//! @return this vector.
template<class T> Vector3<T> Vector3<T>::operator+() const
{
	return *this;
}

//!  + operator
//! @param b the other vector to add
//! @return this + b.
template<class T> Vector3<T> Vector3<T>::operator+(const Vector3<T>& b) const
{
	return Vector3<T>(v[0] + b.v[0], v[1] + b.v[1], v[2] + b.v[2]);
}

//!  - operator
//! @param b the other vector to subtract.
//! @return this - b.
template<class T> Vector3<T> Vector3<T>::operator-(const Vector3<T>& b) const
{
	return Vector3<T>(v[0] - b.v[0], v[1] - b.v[1], v[2] - b.v[2]);
}
//!  * operator
//! @param b the other vector to multiply
//! @return this * b.
template<class T> Vector3<T> Vector3<T>::operator*(T s) const
{
	return Vector3<T>(s * v[0], s * v[1], s * v[2]);
}
//!  / operator
//! @param b the other vector to divide
//! @return this / b.
template<class T> Vector3<T> Vector3<T>::operator/(T s) const
{
	return Vector3<T>(v[0]/s, v[1]/s, v[2]/s);
}

//! Null vector getter.
//! @return a nill vector (0.0,0.0,0.0).
template<class T> Vector3<T> Vector3<T>::null()
{
	return Vector3<T>(0.0,0.0,0.0);
}

//! Angle in radian between two normalized vectors.
//! @param b the other vector to calculate the angle.
//! @return Angle in radian between two normalized vectors.
template<class T> T Vector3<T>::angle(const Vector3<T>& b) const
{
	return std::acos(dot(b)/sqrt(lengthSquared()*b.lengthSquared()));
}
//! dot product.
//! @param b the other vector
//! @return the dot product.
template<class T> T Vector3<T>::dot(const Vector3<T>& b) const
{
	return v[0] * b.v[0] + v[1] * b.v[1] + v[2] * b.v[2];
}


//! cross product.
//! @param b the other vector
//! @return the cross product.
template<class T> Vector3<T> Vector3<T>::operator^(const Vector3<T>& b) const
{
	Vector3<T> result(v[1] * b.v[2] - v[2] * b.v[1],
	                  v[2] * b.v[0] - v[0] * b.v[2],
	                  v[0] * b.v[1] - v[1] * b.v[0]);
	return result;
}

//! Gives the length of a vector.
//! @return the length.
template<class T> T Vector3<T>::length() const
{
	return (T) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
//! Gives the squared length of a vector.
//! This is useful when you don't want to use sqrt to compare distances.
//! @return the squared length.
template<class T> T Vector3<T>::lengthSquared() const
{
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}
//! normalize this vector.
template<class T> void Vector3<T>::normalize()
{
	T s = (T) (1. / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));
	if(s!=0) {
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
	}
}
//! Puts this point to a 3D plane.
//! @param a the 3D plane equation.
template<class T> inline void Vector3<T>::toPlane(const Vector4<T>&a)
{
	static T distance;
	distance= v[0]*a.v[0]+
	          v[1]*a.v[1]+
	          v[2]*a.v[2]+
	          a.v[3];
	this->operator-=(a*distance);
}
//! Puts this point to a 3D plane.
//! @param a the 3D plane equation (without d, origin will belong to the plane.)
template<class T> inline void Vector3<T>::toPlane(const Vector3<T>&a)
{
	static T distance;
	distance= v[0]*a.v[0]+
	          v[1]*a.v[1]+
	          v[2]*a.v[2];
	this->operator-=(a*distance);
}

//! apply a matrix to this vector.
//! @param m the matrix to apply.
template<class T> void Vector3<T>::transfo4d(const Mat4d& m)
{
	(*this)=m*(*this);
}


template<class T> void Vector3<T>::transfo4d(const Mat4f& m)
{
	(*this)=m*(*this);
}

//! print function.
//! @param o the stream to write in.
//! @param v the vector to write.
//! @return the new o stream.
template<class T>
std::ostream& operator<<(std::ostream &o,const Vector3<T> &v)
{
	return o << '[' << v[0] << ',' << v[1] << ',' << v[2] << ']';
}


//! vector multiplication by an other vector.
//! @param a the first vector to use.
//! @param b the second vector to use.
//! @return a*b
template<class T> inline
T operator*(const Vector3<T>&a,const Vector3<T>&b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

//! vector multiplication by a scalar.
//! @param s the scalar to use.
//! @param v the vector to use.
//! @return s*v
template<class T> inline
Vector3<T> operator*(T s,const Vector3<T>&v)
{
	return Vector3<T>(s*v[0],s*v[1],s*v[2]);
}


#endif // VECTOR3_HPP_INCLUDED






































