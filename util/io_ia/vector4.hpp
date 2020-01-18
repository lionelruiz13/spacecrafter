#ifndef VECTOR4_HPP_INCLUDED
#define VECTOR4_HPP_INCLUDED


template<class T> class Vector4 {
public:
	inline Vector4();
	inline Vector4(const T*);
	inline Vector4(const Vector4<T>&);
	inline Vector4(const Vector3<T>&);
	inline Vector4(T, T, T, T);
	inline Vector4(T, T, T);

	inline Vector4& operator=(const Vector4<T>&);
	inline Vector4& operator=(const Vector3<T>&);
	inline Vector4& operator=(const T*);
	inline void set(T, T, T, T);

	inline bool operator==(const Vector4<T>&) const;
	inline bool operator!=(const Vector4<T>&) const;

	inline T& operator[](int);
	inline const T& operator[](int) const;
	inline operator T*();
	inline operator const T*() const;

	inline Vector4& operator+=(const Vector4<T>&);
	inline Vector4& operator-=(const Vector4<T>&);
	inline Vector4& operator*=(T);
	inline Vector4& operator/=(T);

	inline Vector4 operator-(const Vector4<T>&) const;
	inline Vector4 operator+(const Vector4<T>&) const;

	inline Vector4 operator-() const;
	inline Vector4 operator+() const;

	inline Vector4 operator*(T) const;
	inline Vector4 operator/(T) const;

	static Vector4 null();
	static Vector4 nullW();

	inline const T            dot     (const Vector4<T>&)const;
	inline const Vector4<T>   cross   (const Vector4<T>&)const;
	inline const T            getSin  (const Vector4<T>&)const;
	inline void toPlane(const Vector3<T>&);
	inline void toPlane(const Vector4<T>&);

	inline T length() const;
	inline T lengthSquared() const;
	inline void normalize();
	void correctW();


	inline void transfo4d(const Mat4d&);
	Vector4<float> convert() const;
	T v[4];		// The 4 values
};


//! Convert to a float vector
template<class T> Vector4<float> Vector4<T>::convert() const
{
	return Vector4<float>(v[0], v[1], v[2] , v[3]);
}





//! default constructor. Set all to 0.
template<class T> Vector4<T>::Vector4()
{
	v[0]=0;
	v[1]=0;
	v[2]=0;
	v[3]=0;
}

//! Constructor from an array. Data are copied.
//! @param a the array to copy data from.
template<class T> Vector4<T>::Vector4(const T*a)
{
	this->operator=(a);
}

//! Copy constructor.
//! @param a the vector to copy.
template<class T> Vector4<T>::Vector4(const Vector4<T>& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
	v[3]=a.v[3];
}

//! constructor from vector3. Automatically set w to 1.0.
//! @param a the vector to copy.
template<class T> Vector4<T>::Vector4(const Vector3<T>& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
	v[3]=1.0;
}

//! constructor from 4 values.
//! x first value.
//! y second value.
//! z third value.
//! a fourth value (w).
template<class T> Vector4<T>::Vector4(T x, T y, T z, T a)
{
	v[0]=x;
	v[1]=y;
	v[2]=z;
	v[3]=a;
}

//! constructor from 3 values.
//! Automatically set w to 1.0.
//! x first value.
//! y second value.
//! z third value.
template<class T> Vector4<T>::Vector4(T x, T y, T z)
{
	v[0]=x;
	v[1]=y;
	v[2]=z;
	v[3]=1;
}

//! = operator.
//! @param a the vector to copy.
//! @return *this
template<class T> Vector4<T>& Vector4<T>::operator=(const Vector4<T>& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
	v[3]=a.v[3];
	return *this;
}

//! = operator.
//! Automatically set w to 1.0.
//! @param a the vector to copy.
//! @return *this
template<class T> Vector4<T>& Vector4<T>::operator=(const Vector3<T>& a)
{
	v[0]=a.v[0];
	v[1]=a.v[1];
	v[2]=a.v[2];
	v[3]=1;
	return *this;
}

//! = operator.
//! @param a the vector to copy.
//! @return *this
template<class T> Vector4<T>& Vector4<T>::operator=(const T* a)
{
	v[0]=a[0];
	v[1]=a[1];
	v[2]=a[2];
	v[3]=a[3];
	return *this;
}
//! whole vector setter.
//! @param x the first component.
//! @param y the second component.
//! @param z the third component.
//! @param a the fourth component (w).
template<class T> void Vector4<T>::set(T x, T y, T z, T a)
{
	v[0]=x;
	v[1]=y;
	v[2]=z;
	v[3]=a;
}

//! == operator
//! @param a the vector to compare to.
//! @return true if equals, false otherwise.
template<class T> bool Vector4<T>::operator==(const Vector4<T>& a) const
{
	return (v[0] == a.v[0] && v[1] == a.v[1] && v[2] == a.v[2] && v[3] == a.v[3]);
}

//! =! operator
//! @param a the vector to compare to.
//! @return false if equals, true otherwise.
template<class T> bool Vector4<T>::operator!=(const Vector4<T>& a) const
{
	return (v[0] != a.v[0] || v[1] != a.v[1] || v[2] != a.v[2] || v[3] != a.v[3]);
}

//! [] operator
//! @param x index of the desired element.
//! @return the element.
template<class T> T& Vector4<T>::operator[](int x)
{
	return v[x];
}

//! [] operator
//! @param x index of the desired element.
//! @return the element.
template<class T> const T& Vector4<T>::operator[](int x) const
{
	return v[x];
}

//! *operator
//! @return a pointer to the vector array.
template<class T> Vector4<T>::operator T*()
{
	return v;
}
//! *operator
//! @return a pointer to the vector array.
template<class T> Vector4<T>::operator const T*() const
{
	return v;
}
//!  += operator
//! @param the other vector to addition.
//! @return *this
template<class T> Vector4<T>& Vector4<T>::operator+=(const Vector4<T>& a)
{
	v[0] += a.v[0];
	v[1] += a.v[1];
	v[2] += a.v[2];
	v[3] += a.v[3];
	return *this;
}

//!  -= operator
//! @param the other vector to subtract.
//! @return *this
template<class T> Vector4<T>& Vector4<T>::operator-=(const Vector4<T>& a)
{
	v[0] -= a.v[0];
	v[1] -= a.v[1];
	v[2] -= a.v[2];
	v[3] -= a.v[3];
	return *this;
}
//!  *= operator
//! @param the other vector to multiply.
//! @return *this
template<class T> Vector4<T>& Vector4<T>::operator*=(T s)
{
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
	v[3] *= s;
	return *this;
}
//!  -= operator
//! @return - this vector.
template<class T> Vector4<T> Vector4<T>::operator-() const
{
	return Vector4<T>(-v[0], -v[1], -v[2], -v[3]);
}
//!  += operator
//! @return this vector.
template<class T> Vector4<T> Vector4<T>::operator+() const
{
	return *this;
}

//!  + operator
//! @param b the other vector to add
//! @return this + b.
template<class T> Vector4<T> Vector4<T>::operator+(const Vector4<T>& b) const
{
	return Vector4<T>(v[0] + b.v[0], v[1] + b.v[1], v[2] + b.v[2], v[3] + b.v[3]);
}
//!  - operator
//! @param b the other vector to subtract.
//! @return this - b.
template<class T> Vector4<T> Vector4<T>::operator-(const Vector4<T>& b) const
{
	return Vector4<T>(v[0] - b.v[0], v[1] - b.v[1], v[2] - b.v[2], v[3] - b.v[3]);
}
//!  * operator
//! @param b the other vector to multiply
//! @return this * b.
template<class T> Vector4<T> Vector4<T>::operator*(T s) const
{
	return Vector4<T>(s * v[0], s * v[1], s * v[2], s * v[3]);
}
//!  / operator
//! @param b the other vector to divide
//! @return this / b.
template<class T> Vector4<T> Vector4<T>::operator/(T s) const
{
	return Vector4<T>(v[0]/s, v[1]/s, v[2]/s, v[3]/s);
}

//! null vector getter
//! @return a null vector.
template<class T> Vector4<T> Vector4<T>::null()
{
	return Vector4<T>(0.0,0.0,0.0,0.0);
}

//! null vector getter
//! @return a null vector with w at 1.0.
template<class T> Vector4<T> Vector4<T>::nullW()
{
	return Vector4<T>(0.0,0.0,0.0,1.0);
}

//! dot product.
//! @param b the other vector
//! @return the dot product.
template<class T> const T Vector4<T>::dot(const Vector4<T>& b) const
{
	return v[0] * b.v[0] + v[1] * b.v[1] + v[2] * b.v[2] + v[3] * b.v[3];
}
//! cross product.
//! @param b the other vector
//! @return the cross product.
template<class T> const Vector4<T>   Vector4<T>::cross   (const Vector4<T>&a)const
{
	Vector4<T> result(v[1]*a.v[2]-v[2]*a.v[1],
	                  v[2]*a.v[0]-v[0]*a.v[2],
	                  v[0]*a.v[1]-v[1]*a.v[0]);
	return result;
}

//! Gives the sinus of the angle between two NORMALIZED vectors.
//! @param b the other vector
//! @return the sinus.
template<class T> const T            Vector4<T>::getSin  (const Vector4<T>&a)const
{
	return this->cross(a).length();
}

//! Puts this point to a 3D plane.
//! @param a the 3D plane equation (without d, origin will belong to the plane.)
template<class T> void Vector4<T>::toPlane(const Vector3<T>&a)
{
	static T distance;
	distance= v[0]*a.v[0]+
	          v[1]*a.v[1]+
	          v[2]*a.v[2];
	this->operator-=(a*distance);
}

//! Puts this point to a 3D plane.
//! @param a the 3D plane equation.
template<class T> void Vector4<T>::toPlane(const Vector4<T>&a)
{
	static T distance;
	distance= v[0]*a.v[0]+
	          v[1]*a.v[1]+
	          v[2]*a.v[2]+
	          a.v[3];
	this->operator-=(a*distance);
}

//! Gives the length of a vector.
//! @return the length.
template<class T> T Vector4<T>::length() const
{
	return (T) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
}
//! Gives the squared length of a vector.
//! This is useful when you don't want to use sqrt to compare distances.
//! @return the squared length.
template<class T> T Vector4<T>::lengthSquared() const
{
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
}

//! normalize this vector.
template<class T> void Vector4<T>::normalize()
{
	T s = (T) (1. / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]));
	if(s!=0.0) {
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
		v[3] *= s;
	}
}
//! Set the fourth component of the vector at 1.0.
//! This is useful after a true matrix multiplication.
template<class T> void Vector4<T>::correctW()
{
	v[3]=1.0;
}

//! apply some matrix transformations to this vector.
//! @param m the matrix which contains transformations.
template<class T> void Vector4<T>::transfo4d(const Mat4d& m)
{
	(*this)=m*(*this);
}

template<class T>
std::ostream& operator<<(std::ostream &o,const Vector4<T> &v)
{
	return o << '[' << v[0] << ',' << v[1] << ',' << v[2] << ',' << v[3] << ']';
}



//! vector multiplication by an other vector.
//! @param a the first vector to use.
//! @param b the second vector to use.
//! @return a*b
template<class T> inline
T operator*(const Vector4<T>&a,const Vector4<T>&b)
{
	return a.v[0]*b.v[0] + a.v[1]*b.v[1] + a.v[2]*b.v[2] + a.v[3]*b.v[3];
}

//! vector multiplication by a scalar.
//! @param s the scalar to use.
//! @param v the vector to use.
//! @return s*v
template<class T> inline
Vector4<T> operator*(T s,const Vector4<T>&v)
{
	return Vector4<T>(s*v[0],s*v[1],s*v[2],s*v[3]);
}






#endif // VECTOR4_HPP_INCLUDED













































