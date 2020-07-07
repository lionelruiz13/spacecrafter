#ifndef MATRIX4_HPP_INCLUDED
#define MATRIX4_HPP_INCLUDED

#include "fmath.hpp"

// Column-major matrix compatible with openGL.
template<class T> class Matrix4 {
public:
	Matrix4();
	Matrix4(const Matrix4<T>& m);
	Matrix4(T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T);
	Matrix4(const T*);
	Matrix4(const Vector3<T>& v0, const Vector3<T>& v1,
	        const Vector3<T>& v2, const Vector3<T>& v3);
	Matrix4(const Vector4<T>& v0, const Vector4<T>& v1,
	        const Vector4<T>& v2, const Vector4<T>& v3);

	inline Matrix4& operator=(const Matrix4<T>&);
	inline Matrix4& operator=(const T*);
	inline void set(T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T);

	inline T* operator[](int);
	inline operator T*();
	inline operator const T*() const;

	inline Matrix4 operator-(const Matrix4<T>&) const;
	inline Matrix4 operator+(const Matrix4<T>&) const;
	inline Matrix4 operator*(const Matrix4<T>&) const;

	inline Vector3<T> operator*(const Vector3<T>&) const;
	inline Vector3<T> multiplyWithoutTranslation(const Vector3<T>& a) const;
	inline Vector4<T> operator*(const Vector4<T>&) const;

	static Matrix4<T> identity();
	static Matrix4<T> ortho(T, T, T, T, T, T);
	static Matrix4<T> ortho2D(T, T, T, T);
	static Matrix4<T> frustum( T, T, T, T, T, T);
	static Matrix4<T> perspective(T, T, T, T );
	static Matrix4<T> lookAt(T, T, T, T, T, T, T, T, T );
	static Matrix4<T> lookAtFromMatrix(const Matrix4<T>&m);

	static Matrix4<T> translation(const Vector3<T>&);

	//    static Matrix4<T> rotation(const Vector3<T>&)
	static const Matrix4<T> rotation(const Vector3<T>&a,const Vector3<T>&b);
	static const Matrix4<T> rotation(const Vector3<T>&, T);
	static const Matrix4<T> rotation(const T c,const T s,const Vector4<T>&axis);
	static const Matrix4<T> xrotation(T);
	static const Matrix4<T> yrotation(T);
	static const Matrix4<T> zrotation(T);
	static const Matrix4<T> zrotation(T cosinus,T sinus);
	static const Matrix4<T> scaling(const Vector3<T>&);
	static const Matrix4<T> scaling(T);


	Matrix4<T> transpose() const;
	Matrix4<T> inverse() const;
	Matrix4<float> convert() const;
	Matrix4<T> fastInverse() const;
	void setAsOrthonormalFromZ();
	const Vec4f getVector(int column)const;

	Matrix4<T> linearMix(const Matrix4<T> other,const T coef )const;

	void setVector(const Vector4<T>&v, unsigned int ind);

	inline void print(void) const;

	T r[16];
};




//! Basic constructor, set all to 0.
template<class T> Matrix4<T>::Matrix4()
{
	r[0]=0;
	r[1]=0;
	r[2]=0;
	r[3]=0;
	r[4]=0;
	r[5]=0;
	r[6]=0;
	r[7]=0;
	r[8]=0;
	r[9]=0;
	r[10]=0;
	r[11]=0;
	r[12]=0;
	r[13]=0;
	r[14]=0;
	r[15]=0;
}

//! Copy constructor.
//! @param m the matrix to copy.
template<class T> Matrix4<T>::Matrix4(const Matrix4<T>& m)
{
	memcpy(r,m.r,sizeof(m.r));
}

//! Constructor from array.
//! @param m an array to copy data from.
//! make sure it is large enough.
template<class T> Matrix4<T>::Matrix4(const T* m)
{
	memcpy(r,m,sizeof(T)*16);
}

//! = operator.
//! @param m the matrix to copy.
//! @return *this
template<class T> Matrix4<T>& Matrix4<T>::operator=(const Matrix4<T>& m)
{
	memcpy(r,m.r,sizeof(m.r));
	return (*this);
}

//! Constructor from four 3 dimensions vectors.
//! Last line will be 0.0 0.0 0.0 1.0
//! @param v0 the 1st column
//! @param v1 the 2nd column
//! @param v2 the 3rd column
//! @param v3 the 4th column
template<class T> Matrix4<T>::Matrix4(const Vector3<T>& v0,
                                      const Vector3<T>& v1,
                                      const Vector3<T>& v2,
                                      const Vector3<T>& v3)
{
	r[0] = v0.v[0];
	r[1] = v0.v[1];
	r[2] = v0.v[2];
	r[3] = 0.0;
	r[4] = v1.v[0];
	r[5] = v1.v[1];
	r[6] = v1.v[2];
	r[7] = 0.0;
	r[8] = v2.v[0];
	r[9] = v2.v[1];
	r[10] = v2.v[2];
	r[11] = 0.0;
	r[12] = v3.v[0];
	r[13] = v3.v[1];
	r[14] = v3.v[2];
	r[15] = 1.0;
}


//! Constructor from four 4 dimensions vectors.
//! @param v0 the 1st column
//! @param v1 the 2nd column
//! @param v2 the 3rd column
//! @param v3 the 4th column
template<class T> Matrix4<T>::Matrix4(const Vector4<T>& v0,
                                      const Vector4<T>& v1,
                                      const Vector4<T>& v2,
                                      const Vector4<T>& v3)
{
	r[0] = v0.v[0];
	r[1] = v0.v[1];
	r[2] = v0.v[2];
	r[3] = v0.v[3];
	r[4] = v1.v[0];
	r[5] = v1.v[1];
	r[6] = v1.v[2];
	r[7] = v1.v[3];
	r[8] = v2.v[0];
	r[9] = v2.v[1];
	r[10] = v2.v[2];
	r[11] = v2.v[3];
	r[12] = v3.v[0];
	r[13] = v3.v[1];
	r[14] = v3.v[2];
	r[15] = v3.v[3];
}

//! Constructor from 16 values (column by column).
template<class T> Matrix4<T>::Matrix4(T a, T b, T c, T d, T e, T f, T g, T h, T i, T j, T k, T l, T m, T n, T o, T p)
{
	r[0]=a;
	r[1]=b;
	r[2]=c;
	r[3]=d;
	r[4]=e;
	r[5]=f;
	r[6]=g;
	r[7]=h;
	r[8]=i;
	r[9]=j;
	r[10]=k;
	r[11]=l;
	r[12]=m;
	r[13]=n;
	r[14]=o;
	r[15]=p;
}

//! Set the matrix with the 16 given values (column by column).
template<class T> void Matrix4<T>::set(T a, T b, T c, T d, T e, T f, T g, T h, T i, T j, T k, T l, T m, T n, T o, T p)
{
	r[0]=a;
	r[1]=b;
	r[2]=c;
	r[3]=d;
	r[4]=e;
	r[5]=f;
	r[6]=g;
	r[7]=h;
	r[8]=i;
	r[9]=j;
	r[10]=k;
	r[11]=l;
	r[12]=m;
	r[13]=n;
	r[14]=o;
	r[15]=p;
}

//! [] operator.
//! n the index of the wanted column.
template<class T> T* Matrix4<T>::operator[](int n)
{
	return &(r[n*4]);
}

//! * operator
//! @return the address of the 16 values array.
template<class T> Matrix4<T>::operator T*()
{
	return r;
}

//! const * operator
//! @return the address of the 16 values array.
template<class T> Matrix4<T>::operator const T*() const
{
	return r;
}

//! Static function to get a identity matrix.
//! @return a identity matrix.
template<class T> Matrix4<T> Matrix4<T>::identity()
{
	return Matrix4<T>(	1, 0, 0, 0,
	                    0, 1, 0, 0,
	                    0, 0, 1, 0,
	                    0, 0, 0, 1  );
}

//! Static function to get a translation matrix.
//! @param a the vector that describes the translation.
//! @return the corresponding translation matrix.
template<class T> Matrix4<T> Matrix4<T>::translation(const Vector3<T>& a)
{
	return Matrix4<T>(	1, 0, 0, 0,
	                    0, 1, 0, 0,
	                    0, 0, 1, 0,
	                    a.v[0], a.v[1], a.v[2], 1);
}

//! Static function to get a rotation matrix from 2 vectors.
//! The axis will be cross(a,b).
//! The rotation will describe how to get b from a.
//! WARNING: both a and b must be normalized.
//! @param a first vector.
//! @param b second vector.
//! @return the corresponding rotation matrix.
template<class T>const Matrix4<T> Matrix4<T>::rotation(const Vector3<T>&a,const Vector3<T>&b)
{
	static T c,s;
	static Vector3<T> axis;
	c= a.dot(b);
	axis=a^b;
	//cout<<axis<<endl;
	s=axis.length();
	axis.normalize();
	//cout<<axis<<endl;
	//cout<<"cos: "<<c<<endl<<"sin: "<<s<<endl;
	return Matrix4<T>::rotation(c,s,axis);
}

//! Static function to get a rotation matrix.
//! WARNING axis must be normalized.
//! @param axis the axis vector.
//! @param angle the rotation angle, in radians.
//! @return the corresponding rotation matrix.
template<class T> const Matrix4<T> Matrix4<T>::rotation(const Vector3<T>& axis,T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);
	return Matrix4<T>::rotation(c,s,axis);
}

//! Static function to get a rotation matrix.
//! WARNING axis must be normalized.
//! @param c cosine of the angle.
//! @param s sinus of the angle.
//! @param angle the rotation angle, in radians.
//! @return the corresponding rotation matrix.
template<class T> const Matrix4<T> Matrix4<T>::rotation(const T c,const T s,const Vector4<T>&axis)
{
	static T t;
	t = 1 - c;
	return Matrix4<T>(Vector4<T>(t * axis.v[0] * axis.v[0] + c,
	                             t * axis.v[0] * axis.v[1] - s * axis.v[2],
	                             t * axis.v[0] * axis.v[2] + s * axis.v[1],
	                             0),
	                  Vector4<T>(t * axis.v[0] * axis.v[1] + s * axis.v[2],
	                             t * axis.v[1] * axis.v[1] + c,
	                             t * axis.v[1] * axis.v[2] - s * axis.v[0],
	                             0),
	                  Vector4<T>(t * axis.v[0] * axis.v[2] - s * axis.v[1],
	                             t * axis.v[1] * axis.v[2] + s * axis.v[0],
	                             t * axis.v[2] * axis.v[2] + c,
	                             0),
	                  Vector4<T>(0, 0, 0, 1));
}


//! gives a orthographic projection matrix.
//! @param left Specify the coordinates for the left vertical clipping planes.
//! @param right Specify the coordinates for the right vertical clipping planes.
//! @param bottom Specify the coordinates for the bottom horizontal clipping planes.
//! @param top Specify the coordinates for the top horizontal clipping planes.
//! @param nearVal Specify the distances to the nearer depth clipping planes. Negative if the plane is to be behind the viewer.
//! @param farVal Specify the distances to the farther depth clipping planes. Negative if the plane is to be behind the viewer.
//! @return the projection matrix.
template<class T> Matrix4<T> Matrix4<T>::ortho(T left, T right, T bottom, T top, T nearVal, T farVal)
{
	// r_m_l that mean right_minus_left
	T r_m_l = right-left;
	T t_m_b = top - bottom;
	T f_m_n = farVal - nearVal;
	// TODO no warning if left= right of bottom = top or near = far
	return Matrix4<T>(		2/r_m_l,    				 0,   					    0,    0,
	                        0, 			   2/t_m_b,  					    0,    0,
	                        0,   			 	     0, 				 -2/f_m_n,    0,
	                        -(right+left)/r_m_l , -(top + bottom)/t_m_b, -(farVal + nearVal)/f_m_n,    1 );
}

//! gives a orthogonal projection matrix.
//! @see gluOrtho2D
//! @param left Specify the coordinates for the left vertical clipping planes.
//! @param right Specify the coordinates for the right vertical clipping planes.
//! @param bottom Specify the coordinates for the bottom horizontal clipping planes.
//! @param top Specify the coordinates for the top horizontal clipping planes.
//! @return the projection matrix.
template<class T> Matrix4<T> Matrix4<T>::ortho2D(T left, T right, T bottom, T top)
{
	// r_m_l that mean right_minus_left
	T r_m_l = right-left;
	T t_m_b = top - bottom;
	// TODO no warning if left= right of bottom = top
	return Matrix4<T>(		    2/r_m_l,     				  0, 	0,	0,
	                            0, 					2/t_m_b,	0,	0,
	                            0,     					  0,	-1,	0 ,
	                            -(right+left)/r_m_l , -(top + bottom)/t_m_b ,	0,	1 );
}

//! gives a perspective projection matrix.
//! @param left Specify the coordinates for the left vertical clipping planes.
//! @param right Specify the coordinates for the right vertical clipping planes.
//! @param bottom Specify the coordinates for the bottom horizontal clipping planes.
//! @param top Specify the coordinates for the top horizontal clipping planes.
//! @param nearVal Specify the distances to the nearer and farther depth clipping planes. Must be positive!
//! @param farVal Specify the distances to the nearer and farther depth clipping planes. Must be positive!
//! @return the projection matrix.
template<class T> Matrix4<T> Matrix4<T>::frustum(T left, T right, T bottom, T top, T znear, T zfar)
{
	T tmp1 = 2.0 * znear;
	T tmp2 = right - left;
	T tmp3 = top - bottom;
	T tmp4 = zfar - znear;
	// TODO no warning
	return Matrix4<T>(   tmp1 /tmp2,           0,    0,           0 ,
	                     0,   tmp1/tmp3,    0,           0 ,
	                     (right+left)/tmp2, (top+bottom)/tmp3,   (-zfar-znear)/tmp4,  -1,
	                     0,           0,      (-tmp1*zfar)/tmp4 ,           0 );
}

//! Gives a orthogonal projection matrix.
//! @see gluPerspective
//! @param fovyInDegrees the field of view width.
//! @param aspectRation the ration y/x of the screen size.
//! @param znear Nearest draw distance. Try to have the farthest as possible.
//! @param zfar Farthest draw distance. Try to have the nearest as possible.
template<class T> Matrix4<T> Matrix4<T>::perspective(T fovyInDegrees, T aspectRatio, T znear, T zfar)
{
	T ymax, xmax;
	ymax = znear * tanf(fovyInDegrees * C_PI / 360.0);
	xmax = ymax * aspectRatio;
	return frustum(-xmax, xmax, -ymax, ymax, znear, zfar);
}

//! Gives a view matrix.
//! @param eyeXYZ the position of the camera.
//! @param centerXYZ a point where the camera is looking at.
//! @param upXYZ a vector which vertically point the top of your scene.
template<class T> Matrix4<T> Matrix4<T>::lookAt(T eyeX, T eyeY, T eyeZ, T centerX, T centerY, T centerZ, T upX, T upY, T upZ)
{
	Vector3<T> F= Vector3<T>(centerX - eyeX,centerY - eyeY, centerZ - eyeZ);
	Vector3<T> UP= Vector3<T>(upX, upY, upZ);
	F.normalize();
	UP.normalize();
	Vector3<T> S = F ^ UP;
	S.normalize();
	Vector3<T> U = S  ^ F;

	//~ Matrix4<T> tmp1= Matrix4<T>( S[0], U[0], -F[0], 0,
	//~ S[1], U[1], -F[1], 0,
	//~ S[2], U[2], -F[2], 0,
	//~ 0,    0,    0 , 1 );
//~
	//~ Matrix4<T> tmp2=translation(Vector3<T>(-eyeX, -eyeY, -eyeZ));
//~
	//~ return tmp1 * tmp2;
	return Matrix4<T> ( S[0], U[0], -F[0], 0,
	                    S[1], U[1], -F[1], 0,
	                    S[2], U[2], -F[2], 0,
	                    0,    0,    0 , 1 )*translation(Vector3<T>(-eyeX, -eyeY, -eyeZ));
}

//! Gives a rotation matrix around the x axis.
//! WARNING this rotation is unconventionally clockwise!
//! @param angle the angle in radians.
//! @return the corresponding rotation matrix.
template<class T> const Matrix4<T> Matrix4<T>::xrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return Matrix4<T>(1, 0, 0, 0,
	                  0, c, s, 0,
	                  0,-s, c, 0,
	                  0, 0, 0, 1 );
}

template<class T> Matrix4<T> Matrix4<T>::lookAtFromMatrix(const Matrix4<T>&m)
{
	return Matrix4<T> ( m.r[4], m.r[8], -m.r[0], 0, // Warning! It appears transposed!
	                    m.r[5], m.r[9], -m.r[1], 0,
	                    m.r[6], m.r[10],-m.r[2], 0,
	                    0,      0,        0,     1 )

	       *translation(Vector3<T>(-m.r[12], -m.r[13], -m.r[14]));
}


//! Gives a rotation matrix around the y axis.
//! WARNING this rotation is unconventionally clockwise!
//! @param angle the angle in radians.
//! @return the corresponding rotation matrix.
template<class T> const Matrix4<T> Matrix4<T>::yrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return Matrix4<T>( c, 0,-s, 0,
	                   0, 1, 0, 0,
	                   s, 0, c, 0,
	                   0, 0, 0, 1 );
}

//! Gives a rotation matrix around the z axis.
//! WARNING this rotation is unconventionally clockwise!
//! @param angle the angle in radians.
//! @return the corresponding rotation matrix.
template<class T> const Matrix4<T> Matrix4<T>::zrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return Matrix4<T>(c, s, 0, 0,
	                  -s, c, 0, 0,
	                  0, 0, 1, 0,
	                  0, 0, 0, 1 );
}

//! Gives a rotation matrix around the y axis.
//! WARNING this rotation is unconventionally clockwise!
//! @param cos the cosine in radians.
//! @param sin the sinus in radians.
//! @return the corresponding rotation matrix.
template<class T> const Matrix4<T> Matrix4<T>::zrotation(T cos,T sin)
{
	// writted by Jerome LARTILLOT while zrotation is unconventionnally CW instead of CCW.

	return Matrix4<T>(cos, -sin, 0, 0,
	                  sin, cos, 0, 0,
	                  0, 0, 1, 0,
	                  0, 0, 0, 1 );
}

//! Gives a scaling matrix.
//! @param s a vector which contains the 3 scale factors.
//! @return the corresponding scaling matrix.
template<class T> const Matrix4<T> Matrix4<T>::scaling(const Vector3<T>& s)
{
	return Matrix4<T>(s[0], 0  , 0  , 0,
	                  0   ,s[1], 0  , 0,
	                  0   , 0  ,s[2], 0,
	                  0   , 0  , 0  , 1);
}

//! Gives a scaling matrix.
//! @param scale the scale amount for the 3 dimensions.
//! @return the corresponding scaling matrix.
template<class T> const Matrix4<T> Matrix4<T>::scaling(T scale)
{
	return scaling(Vector3<T>(scale, scale, scale));
}

//! Multiplication operator with Vector3
//! @see correctW
//! multiply a vector by this matrix.
//! @param a the vector to multiply
//! @return The result.
// multiply column vector by a 4x4 matrix in homogeneous coordinate (use a[3]=1)
template<class T> Vector3<T> Matrix4<T>::operator*(const Vector3<T>& a) const
{
	return Vector3<T>(	r[0]*a.v[0] + r[4]*a.v[1] +  r[8]*a.v[2] + r[12],
	                    r[1]*a.v[0] + r[5]*a.v[1] +  r[9]*a.v[2] + r[13],
	                    r[2]*a.v[0] + r[6]*a.v[1] + r[10]*a.v[2] + r[14] );
}

//! Multiply a vector by the 3x3 part of this matrix.
//! This only apply rotation and scaling. Not translation nor projection effects.
template<class T> Vector3<T> Matrix4<T>::multiplyWithoutTranslation(const Vector3<T>& a) const
{
	return Vector3<T>(	r[0]*a.v[0] + r[4]*a.v[1] +  r[8]*a.v[2],
	                    r[1]*a.v[0] + r[5]*a.v[1] +  r[9]*a.v[2],
	                    r[2]*a.v[0] + r[6]*a.v[1] + r[10]*a.v[2] );
}

//! Multiplication operator with Vector4
//! WARNING: ignore 4th line!
//! TODO: correct this function.
//! @see correctW
//! multiply a vector by this matrix.
//! @param a the vector to multiply
//! @return The result.
// multiply column vector by a 4x4 matrix in homogeneous coordinate (considere a[3]=1)
template<class T> Vector4<T> Matrix4<T>::operator*(const Vector4<T>& a) const
{
	return Vector4<T>(	r[0]*a.v[0] + r[4]*a.v[1] +  r[8]*a.v[2] + r[12]*a.v[3],
	                    r[1]*a.v[0] + r[5]*a.v[1] +  r[9]*a.v[2] + r[13]*a.v[3],
	                    r[2]*a.v[0] + r[6]*a.v[1] + r[10]*a.v[2] + r[14]*a.v[3] );
}

//! Transpose this matrix.
//! @return The transpose of this matrix.
template<class T> Matrix4<T> Matrix4<T>::transpose() const
{
	return Matrix4<T>(	r[0], r[4], r[8],  r[12],
	                    r[1], r[5], r[9],  r[13],
	                    r[2], r[6], r[10], r[14],
	                    r[3], r[7], r[11], r[15]);
}

//! Multiplication of 2 matrix.
//! @param a the right operand.
//! @return this*a.
template<class T> Matrix4<T> Matrix4<T>::operator*(const Matrix4<T>& a) const
{
#define MATMUL(R, C) (r[R] * a.r[C] + r[R+4] * a.r[C+1] + r[R+8] * a.r[C+2] + r[R+12] * a.r[C+3])
	return Matrix4<T>(	MATMUL(0,0), MATMUL(1,0), MATMUL(2,0), MATMUL(3,0),
	                    MATMUL(0,4), MATMUL(1,4), MATMUL(2,4), MATMUL(3,4),
	                    MATMUL(0,8), MATMUL(1,8), MATMUL(2,8), MATMUL(3,8),
	                    MATMUL(0,12), MATMUL(1,12), MATMUL(2,12), MATMUL(3,12) );
#undef MATMUL
}

//! Matrix addition.
//! @param a the matrix to addition.
//! @return this+a.
template<class T> Matrix4<T> Matrix4<T>::operator+(const Matrix4<T>& a) const
{
	return Matrix4<T>(	r[0]+a.r[0], r[1]+a.r[1], r[2]+a.r[2], r[3]+a.r[3],
	                    r[4]+a.r[4], r[5]+a.r[5], r[6]+a.r[6], r[7]+a.r[7],
	                    r[8]+a.r[8], r[9]+a.r[9], r[10]+a.r[10], r[11]+a.r[11],
	                    r[12]+a.r[12], r[13]+a.r[13], r[14]+a.r[14], r[15]+a.r[15] );
}

//! Matrix subtraction.
//! @param a the matrix to subtract.
//! @return this-a.
template<class T> Matrix4<T> Matrix4<T>::operator-(const Matrix4<T>& a) const
{
	return Matrix4<T>(	r[0]-a.r[0], r[1]-a.r[1], r[2]-a.r[2], r[3]-a.r[3],
	                    r[4]-a.r[4], r[5]-a.r[5], r[6]-a.r[6], r[7]-a.r[7],
	                    r[8]-a.r[8], r[9]-a.r[9], r[10]-a.r[10], r[11]-a.r[11],
	                    r[12]-a.r[12], r[13]-a.r[13], r[14]-a.r[14], r[15]-a.r[15] );
}

//! Matrix raw inversion.
//! This function won't use any shortcut.
//! Don't forget that you can invert:
//! -rotation matrix by transposing.
//! -scaling matrix by calculating its inverses.
//! -translation matrix by multiplying last column (except w) by -1.
//! @return the inverse of this matrix, or a null matrix when it's not possible.
/*
 * Code ripped from the GLU library
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy jle@star.be
 * Return zero matrix on failure (singular matrix)
 */
template<class T> Matrix4<T> Matrix4<T>::inverse() const
{
	const T * m = r;
	T out[16];

	/* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS(a, b) { T *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

	T wtmp[4][8];
	T m0, m1, m2, m3, s;
	T *r0, *r1, *r2, *r3;

	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

	r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
	                              r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
	                                      r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
	                                              r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
	                                                      r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
	                                                              r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
	                                                                      r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
	                                                                              r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
	                                                                                      r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
	                                                                                              r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
	                                                                                                      r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
	                                                                                                              r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

	/* choose pivot - or die */
	if (fabs(r3[0]) > fabs(r2[0]))
		SWAP_ROWS(r3, r2);
	if (fabs(r2[0]) > fabs(r1[0]))
		SWAP_ROWS(r2, r1);
	if (fabs(r1[0]) > fabs(r0[0]))
		SWAP_ROWS(r1, r0);
	if (0.0 == r0[0])
		return Matrix4<T>();

	/* eliminate first variable     */
	m1 = r1[0] / r0[0];
	m2 = r2[0] / r0[0];
	m3 = r3[0] / r0[0];
	s = r0[1];
	r1[1] -= m1 * s;
	r2[1] -= m2 * s;
	r3[1] -= m3 * s;
	s = r0[2];
	r1[2] -= m1 * s;
	r2[2] -= m2 * s;
	r3[2] -= m3 * s;
	s = r0[3];
	r1[3] -= m1 * s;
	r2[3] -= m2 * s;
	r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0) {
		r1[4] -= m1 * s;
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r0[5];
	if (s != 0.0) {
		r1[5] -= m1 * s;
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r0[6];
	if (s != 0.0) {
		r1[6] -= m1 * s;
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r0[7];
	if (s != 0.0) {
		r1[7] -= m1 * s;
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}

	/* choose pivot - or die */
	if (fabs(r3[1]) > fabs(r2[1]))
		SWAP_ROWS(r3, r2);
	if (fabs(r2[1]) > fabs(r1[1]))
		SWAP_ROWS(r2, r1);
	if (0.0 == r1[1])
		return Matrix4<T>();

	/* eliminate second variable */
	m2 = r2[1] / r1[1];
	m3 = r3[1] / r1[1];
	r2[2] -= m2 * r1[2];
	r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3];
	r3[3] -= m3 * r1[3];
	s = r1[4];
	if (0.0 != s) {
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r1[5];
	if (0.0 != s) {
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r1[6];
	if (0.0 != s) {
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r1[7];
	if (0.0 != s) {
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}

	/* choose pivot - or die */
	if (fabs(r3[2]) > fabs(r2[2]))
		SWAP_ROWS(r3, r2);
	if (0.0 == r2[2])
		return Matrix4<T>();

	/* eliminate third variable */
	m3 = r3[2] / r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
	                              r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];

	/* last check */
	if (0.0 == r3[3])
		return Matrix4<T>();

	s = 1.0 / r3[3];		/* now back substitute row 3 */
	r3[4] *= s;
	r3[5] *= s;
	r3[6] *= s;
	r3[7] *= s;

	m2 = r2[3];			/* now back substitute row 2 */
	s = 1.0 / r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
	        r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
	                              r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
	                              r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

	m1 = r1[2];			/* now back substitute row 1 */
	s = 1.0 / r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
	        r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
	                              r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

	m0 = r0[1];			/* now back substitute row 0 */
	s = 1.0 / r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
	        r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

	MAT(out, 0, 0) = r0[4];
	MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
	MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
	MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
	MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
	MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
	MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
	MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
	MAT(out, 3, 3) = r3[7];

	return Matrix4<T>(out);

#undef MAT
#undef SWAP_ROWS
}


//! Makes a matrix orthonormal.
//! Uses the z vector first, then the x vector, end finally retrieve y.
template<class T> void Matrix4<T>::setAsOrthonormalFromZ()
{
	static Vector4<T> f,s,t; // first second third, order.
	f=r+2*4,
	s=r+0*4,
	t=r+1*4;

	f.normalize();
	s.toPlane(f.v);
	s.normalize();
	t=s.cross(f);
	setVector(f,2);
	setVector(s,0);
	setVector(t,1);
}

//! Get a vector of the matrix.
//! @param column the desired column in [0-3].
//! @return the asked column as a 4 components vector.
template<class T> const Vec4f Matrix4<T>::getVector(int column)const
{
	Vec4f result=r+column*4;
	return result;
}

#define APPLY_TO_LINE(x) result.r[x]=r[x]*(1.0-coef)+coef*other.r[x];

//! Makes a linear mix between two matrix.
//! @param other the other matrix to mix with this matrix.
//! @param coef the mix amount. 0 will result in this, 1 will result in other.
//! don't forget to clamp coef between 0 and 1.
//! @return the mixed matrix.
template<class T> Matrix4<T> Matrix4<T>::linearMix(const Matrix4<T> other,const T coef )const
{
	Matrix4<T> result;
	APPLY_TO_LINE(0)
	APPLY_TO_LINE(1)
	APPLY_TO_LINE(2)
	APPLY_TO_LINE(3)
	APPLY_TO_LINE(4)
	APPLY_TO_LINE(5)
	APPLY_TO_LINE(6)
	APPLY_TO_LINE(7)
	APPLY_TO_LINE(8)
	APPLY_TO_LINE(9)
	APPLY_TO_LINE(10)
	APPLY_TO_LINE(11)
	APPLY_TO_LINE(12)
	APPLY_TO_LINE(13)
	APPLY_TO_LINE(14)
	APPLY_TO_LINE(15)
	return result;
}
#undef APPLY_TO_LINE

//! Set just one vector (column) of the matrix.
//! @param v the vector to supply values.
//! @param ind the index of the column to set.
template<class T> void Matrix4<T>::setVector(const Vector4<T>&v,unsigned int ind)
{
	ind*=4;
	r[ind+0]=v.v[0];
	r[ind+1]=v.v[1];
	r[ind+2]=v.v[2];
	r[ind+3]=ind==12;
}

//! Print this matrix on 4 lines with 2 end-of-line at the end.
template<class T> void Matrix4<T>::print(void) const
{
	printf("[%5.3lf %5.3lf %5.3lf %5.3lf]\n"
	       "[%5.3lf %5.3lf %5.3lf %5.3lf]\n"
	       "[%5.3lf %5.3lf %5.3lf %5.3lf]\n"
	       "[%5.3lf %5.3lf %5.3lf %5.3lf]\n\n",
	       r[0],r[4],r[8],r[12],
	       r[1],r[5],r[9],r[13],
	       r[2],r[6],r[10],r[14],
	       r[3],r[7],r[11],r[15]);
}


//! Convert to a float matrix
template<class T> Matrix4<float> Matrix4<T>::convert() const
{
	return Matrix4<float>(	r[0], r[1], r[2],  r[3],
	                        r[4], r[5], r[6],  r[7],
	                        r[8], r[9], r[10], r[11],
	                        r[12], r[13], r[14], r[15]);
}


template<class T> Matrix4<T> Matrix4<T>::fastInverse() const
{
	return Matrix4<T>(	r[0], r[4], r[8],  -r[3],
	                    r[1], r[5], r[9],  -r[7],
	                    r[2], r[6], r[10], -r[11],
	                    r[12], r[13], r[14], r[15]);
}







#endif // MATRIX4_HPP_INCLUDED
