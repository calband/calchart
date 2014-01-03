/*
 * linmath.h
 * Definitions for linear algebra classes
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _LINMATH_H_
#define _LINMATH_H_

#include <math.h>

typedef float LinElem;

/**
 * A four-dimensional vector.
 * This is used often for two-dimensional manipulations by zeroing the z component
 * and maintaining the default 1.0 value for the w component. 
 * Matrices (which, in CalChart, always have 4x4 dimensions) are composed of four of
 * these vectors, and those matrices can be used for a large number of applications,
 * like rotating, scaling, translating, etc.
 */
class Vector
{
public:

	/** 
	 * Creates a default vector: (0, 0, 0, 1).
	 */
	Vector()
	{
		elem[0] = elem[1] = elem[2] = 0.0;
		elem[3] = 1.0;
	}

	/**
	 * Creates a vector: (x, y, z, w), where x, y, z, and w
	 * are provided in the parameters.
	 * @param x The x component of the vector.
	 * @param y The y component of the vector.
	 * @param z The z component of the vector.
	 * @param w The w component of the vector.
	 */
	Vector(LinElem x, LinElem y, LinElem z, LinElem w = 1.0)
	{
		elem[0] = x;
		elem[1] = y;
		elem[2] = z;
		elem[3] = w;
	}

	/**
	 * Makes a copy of another vector.
	 * @param ref The vector to copy.
	 */
	Vector(const Vector& ref)
	{
		*this = ref;
	}

	/**
	 * Returns a component of the vector.
	 * @param i The index of the component that will be
	 * returned.
	 * @return The component of the vector identified by
	 * the index passed as a parameter.
	 */
	inline LinElem Get(unsigned i) const { return elem[i]; }

	/**
	 * Returns the x component of the vector.
	 * @return The x component of the vector.
	 */
	inline LinElem GetX() const { return elem[0]; }

	/**
	* Returns the x component of the vector.
	* @return The y component of the vector.
	*/
	inline LinElem GetY() const { return elem[1]; }

	/**
	* Returns the x component of the vector.
	* @return The z component of the vector.
	*/
	inline LinElem GetZ() const { return elem[2]; }

	/**
	* Returns the x component of the vector.
	* @return The w component of the vector.
	*/
	inline LinElem GetW() const { return elem[3]; }

	/**
	 * Sets a particular component of the vector to a particular
	 * value.
	 * @param i The index of the component to modify.
	 * @param val The new value to assign to the component.
	 */
	inline void Set(int i, LinElem val) { elem[i] = val; }

	/**
	 * Sets the X component of the vector.
	 * @param val The value to assign to the X component of the vector.
	 */
	inline void SetX(LinElem val) { elem[0] = val; }

	/**
	* Sets the Y component of the vector.
	* @param val The value to assign to the Y component of the vector.
	*/
	inline void SetY(LinElem val) { elem[1] = val; }

	/**
	* Sets the Z component of the vector.
	* @param val The value to assign to the Z component of the vector.
	*/
	inline void SetZ(LinElem val) { elem[2] = val; }

	/**
	* Sets the W component of the vector.
	* @param val The value to assign to the W component of the vector.
	*/
	inline void SetW(LinElem val) { elem[3] = val; }

	/** 
	 * Homogenizes the vector (that is, divides each component by
	 * the last component).
	 */
	inline void Homogenize()
	{
		elem[0] /= elem[3];
		elem[1] /= elem[3];
		elem[2] /= elem[3];
		elem[3] = 1;
	}

	/**
	* Makes this vector equal to another, such that all of its
	* components will be made identical to those of the other
	* vector.
	* @param v The vector to which this vector should be made
	* identical.
	* @return A reference to this vector, now modified.
	*/
	inline const Vector& operator = (const Vector& v)
	{
		elem[0] = v.elem[0];
		elem[1] = v.elem[1];
		elem[2] = v.elem[2];
		elem[3] = v.elem[3];
		return *this;
	}

	/**
	 * Performs vector-scalar multiplication.
	 * @param s The scalar to multiply the vector by.
	 * @return A reference to this vector, now modified.
	 */
	inline const Vector& operator *= (LinElem s)
	{
		elem[0] *= s;
		elem[1] *= s;
		elem[2] *= s;
		elem[3] *= s;
		return *this;
	}

	/**
	* Performs vector-scalar division.
	* @param s The scalar to divide the vector by.
	* @return A reference to this vector, now modified.
	*/
	inline const Vector& operator /= (LinElem s)
	{
		elem[0] /= s;
		elem[1] /= s;
		elem[2] /= s;
		elem[3] /= s;
		return *this;
	}

	/**
	 * Calculates the dot product between two vectors.
	 * @param a The first vector.
	 * @param b The second vector.
	 * @return The dot product between vectors a and b.
	 */
	inline friend LinElem operator * (const Vector& a, const Vector& b)
	{
		return a.elem[0]*b.elem[0] + a.elem[1]*b.elem[1] + a.elem[2]*b.elem[2] +
			a.elem[3]*b.elem[3];
	}

	/**
	 * Performs component-wise multiplication between vectors a and b.
	 * @param a The first vector.
	 * @param b The second vector.
	 * @return The result of performing the component-wise multiplication
	 * of vectors a and b.
	 */
	inline friend Vector operator | (const Vector& a, const Vector& b)
	{
		return Vector(a.elem[0]*b.elem[0], a.elem[1]*b.elem[1],
			a.elem[2]*b.elem[2], a.elem[3]*b.elem[3]);
	}

	/**
	 * Performs vector addition, and returns the result.
	 * @param a The first vector to add.
	 * @param b The second vector to add.
	 * @return The result of adding vectors a and b.
	 */
	inline friend Vector operator + (const Vector& a, const Vector& b)
	{
		return Vector(a.elem[0]+b.elem[0], a.elem[1]+b.elem[1],
			a.elem[2]+b.elem[2]);
	}

	/**
	 * Performs vector subtraction, and returns the result.
	 * @param a The first vector.
	 * @param b A second vector to subtract from the first.
	 * @return The result of subtracting vector b from vector a.
	 */
	inline friend Vector operator - (const Vector& a, const Vector& b)
	{
		return Vector(a.elem[0]-b.elem[0], a.elem[1]-b.elem[1],
			a.elem[2]-b.elem[2]);
	}

	/**
	 * Negates a vector.
	 * @param v The vector to negate.
	 * @return The result of negating vector v.
	 */
	inline friend Vector operator - (const Vector& v)
	{
		return Vector(-v.elem[0], -v.elem[1], -v.elem[2], v.elem[3]);
	}

protected:

	/**
	 * Records the components of the vector.
	 * The x, y, z, and w components are stored under array indices
	 * 0, 1, 2, and 3, respectively.
	 */
	LinElem elem[4];
};

/**
 * A 4x4 matrix.
 * The row vectors are made up of Vector objects.
 */
class Matrix
{
public:

	/**
	 * Creates an identity matrix.
	 */
	Matrix()
	{
		vect[0] = Vector(1.0, 0.0, 0.0, 0.0);
		vect[1] = Vector(0.0, 1.0, 0.0, 0.0);
		vect[2] = Vector(0.0, 0.0, 1.0, 0.0);
		vect[3] = Vector(0.0, 0.0, 0.0, 1.0);
	}

	/**
	 * Copies a matrix.
	 * @param ref The matrix to copy.
	 */
	Matrix(const Matrix& ref)
	{
		*this = ref;
	}

	/**
	 * Constructs a Matrix from four Vector objects.
	 * @param v0 First row vector.
	 * @param v1 Second row vector.
	 * @param v2 Third row vector.
	 * @param v3 Fourth row vector.
	 */
	Matrix(const Vector& v0, const Vector& v1,
		const Vector& v2, const Vector& v3)
	{
		vect[0] = v0;
		vect[1] = v1;
		vect[2] = v2;
		vect[3] = v3;
	}

	/** 
	 * Returns an element from a particular position in the matrix.
	 * @param u The row of the element.
	 * @param v The column of the element.
	 * @return The element at position (row,column) = (u,v) in the matrix.
	 */
	inline LinElem GetElem(int u, int v) const { return vect[u].Get(v); }

	/**
	 * Places a value into the matrix at a particular location.
	 * @param u The row at which to insert the element.
	 * @param v The column at which to insert the element.
	 * @param val The val to insert into the matrix.
	 */
	inline void SetElem(int u, int v, LinElem val) { vect[u].Set(v, val); }

	/**
	 * Returns the transpose of the matrix.
	 * @return The transpose of the matrix.
	 */
	inline Matrix GetTranspose() const
	{
		return Matrix(Vector(vect[0].Get(0), vect[1].Get(0),
			vect[2].Get(0), vect[3].Get(0)),
			Vector(vect[0].Get(1), vect[1].Get(1),
			vect[2].Get(1), vect[3].Get(1)),
			Vector(vect[0].Get(2), vect[1].Get(2),
			vect[2].Get(2), vect[3].Get(2)),
			Vector(vect[0].Get(3), vect[1].Get(3),
			vect[2].Get(3), vect[3].Get(3)));
	}

	/**
	 * Modifies this matrix to make it identical to another.
	 * @param m The matrix to which this matrix should be made identical.
	 * @return A reference to this matrix, now modified.
	 */
	inline const Matrix& operator = (const Matrix& m)
	{
		vect[0] = m.vect[0];
		vect[1] = m.vect[1];
		vect[2] = m.vect[2];
		vect[3] = m.vect[3];
		return *this;
	}

	/**
	 * Performs matrix multiplication, and puts the result in this matrix.
	 * @param m The matrix to multiply with this object.
	 * @return A reference to this matrix, now modified.
	 */
	inline const Matrix& operator *= (const Matrix& m)
	{
		*this = *this * m;
		return *this;
	}

	/**
	 * Performs matrix-scalar multiplication, storing the result in this matrix.
	 * @param s The scalar to multiply this matrix by.
	 * @return A reference to this matrix, now modified.
	 */
	inline const Matrix& operator *= (LinElem s)
	{
		vect[0] *= s;
		vect[1] *= s;
		vect[2] *= s;
		vect[3] *= s;
		return *this;
	}

	/**
	 * Performs matrix-scalar division, storing the result in this matrix.
	 * @param s The scalar to divide this matrix by.
	 * @return A reference to this matrix, now modified.
	 */
	inline const Matrix& operator /= (LinElem s)
	{
		vect[0] /= s;
		vect[1] /= s;
		vect[2] /= s;
		vect[3] /= s;
		return *this;
	}

	/**
	 * Performs matrix multiplication.
	 * @param a The first matrix.
	 * @param b The second matrix.
	 * @return The 4x4 matrix resulting from the product of 4x4 matrices
	 * a and b.
	 */
	inline friend Matrix operator * (const Matrix& a, const Matrix& b)
	{
		Matrix at = a.GetTranspose();
		return Matrix(Vector(b.vect[0] * at.vect[0], b.vect[0] * at.vect[1],
			b.vect[0] * at.vect[2], b.vect[0] * at.vect[3]),
			Vector(b.vect[1] * at.vect[0], b.vect[1] * at.vect[1],
			b.vect[1] * at.vect[2], b.vect[1] * at.vect[3]),
			Vector(b.vect[2] * at.vect[0], b.vect[2] * at.vect[1],
			b.vect[2] * at.vect[2], b.vect[2] * at.vect[3]),
			Vector(b.vect[3] * at.vect[0], b.vect[3] * at.vect[1],
			b.vect[3] * at.vect[2], b.vect[3] * at.vect[3]));
	}

	/**
	 * Multiplies a matrix by a vector.
	 * @param a The matrix.
	 * @param b The vector.
	 * @return The vector that results from multiplying matrix a by
	 * vector b.
	 */
	inline friend Vector operator * (const Matrix& a, const Vector& b)
	{
		return Vector(a.vect[0]*b, a.vect[1]*b,
			a.vect[2]*b, a.vect[3]*b);
	}

protected:

	/** 
	 * The vector rows of the matrix. Each vector represents a row of the
	 * matrix, where the vector at the Nth index represents the Nth row
	 * of the matrix.
	 */
	Vector vect[4];
};

/**
* A matrix that can be used to translate points on the x, y, and z axes
* (local or absolute).
* If multiplied by a vector, the matrix will translate that vector
* on the absolute x, y, and z axes.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will translate that vector on the transformation matrix's local
* x, y, and z axes.
*/
class TranslationMatrix: public Matrix
{
public:

	/** 
	 * Makes a translation matrix that will translate points
	 * by a specified amount along the x, y, and z axes.
	 * @param v A vector whose x, y, and z components specify
	 * how many units the translation matrix will translate
	 * points along the x, y, and z axes, respectively.
	 */
	TranslationMatrix(const Vector& v):
	Matrix()
	{
		vect[0].SetW(v.GetX());
		vect[1].SetW(v.GetY());
		vect[2].SetW(v.GetZ());
	}
};

/**
* A matrix that can be used to scale a point's location along the
* x, y, and z axes (local or absolute).
* If multiplied by a vector, the matrix will scale that vector
* along the absolute x, y, and z axes.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will scale that vector along the transformation matrix's local
* x, y, and z axes.
*/
class ScaleMatrix: public Matrix
{
public:
	
	/** 
	 * Makes a scale matrix that will scale vectors along the
	 * x, y, and z axes.
	 * @param v A vector whose x, y, and z components specify
	 * the amount that vectors which are transformed by this
	 * matrix will be scaled along the x, y, and z axes,
	 * respectively.
	 */
	ScaleMatrix(const Vector& v)
	{
		vect[0] = Vector(v.GetX(), 0.0, 0.0, 0.0);
		vect[1] = Vector(0.0, v.GetY(), 0.0, 0.0);
		vect[2] = Vector(0.0, 0.0, v.GetZ(), 0.0);
		vect[3] = Vector(0.0, 0.0, 0.0, 1.0);
	}
};

/**
* A matrix that can be used to reflect a point across an x axis (local
* or absolute).
* If multiplied by a vector, the matrix will reflect that vector
* across the absolute x-axis.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will reflect that vector across the transformation matrix's local
* x axis.
*/
class XReflectionMatrix: public Matrix
{
public:

	/** 
	 * Makes a reflection matrix that will reflect vectors across
	 * the x axis.
	 */
	XReflectionMatrix():
	Matrix()
	{
		vect[0].SetX(-1);
	}
};

/**
* A matrix that can be used to reflect a point across a y axis (local
* or absolute).
* If multiplied by a vector, the matrix will reflect that vector
* across the absolute y-axis.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will reflect that vector across the transformation matrix's local
* y axis.
*/
class YReflectionMatrix: public Matrix
{
public:

	/**
	* Makes a reflection matrix that will reflect vectors across
	* the y axis.
	*/
	YReflectionMatrix():
	Matrix()
	{
		vect[1].SetY(-1);
	}
};

/**
* A matrix that can be used to reflect a point across a z axis (local
* or absolute).
* If multiplied by a vector, the matrix will reflect that vector
* across the absolute z-axis.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will reflect that vector across the transformation matrix's local
* z axis.
*/
class ZReflectionMatrix: public Matrix
{
public:

	/**
	* Makes a reflection matrix that will reflect vectors across
	* the z axis.
	*/
	ZReflectionMatrix():
	Matrix()
	{
		vect[2].SetZ(-1);
	}
};

/**
* A matrix that can be used to shear vectors in the XY plane (local
* or absolute), such that their displacements along the x axis is
* proportional to their y components.
* If multiplied by a vector, the matrix will shear that vector
* in the absolute XY plane.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will shear that vector in the matrix's local XY plane.
*/
class XYShearMatrix: public Matrix
{
public:

	/**
	* Makes a shear matrix which will displace vectors along the x
	* axis such that their displacement is proportional to their
	* y components.
	* @param amount The proportionality constant that relates a vector's
	* displacement along the x axis to its y component.
	*/
	XYShearMatrix(LinElem amount):
	Matrix()
	{
		vect[0].SetY(amount);
	}
};

/**
* A matrix that can be used to shear vectors in the XZ plane (local
* or absolute), such that their displacements along the x axis is
* proportional to their z components.
* If multiplied by a vector, the matrix will shear that vector
* in the absolute XZ plane.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will shear that vector in the matrix's local XZ plane.
*/
class XZShearMatrix: public Matrix
{
public:

	/**
	* Makes a shear matrix which will displace vectors along the x
	* axis such that their displacement is proportional to their
	* z components.
	* @param amount The proportionality constant that relates a vector's
	* displacement along the x axis to its z component.
	*/
	XZShearMatrix(LinElem amount):
	Matrix()
	{
		vect[0].SetZ(amount);
	}
};

/**
* A matrix that can be used to shear vectors in the XY plane (local
* or absolute), such that their displacements along the y axis is
* proportional to their x components.
* If multiplied by a vector, the matrix will shear that vector
* in the absolute XY plane.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will shear that vector in the matrix's local XY plane.
*/
class YXShearMatrix: public Matrix
{
public:

	/**
	* Makes a shear matrix which will displace vectors along the y
	* axis such that their displacement is proportional to their
	* x components.
	* @param amount The proportionality constant that relates a vector's
	* displacement along the y axis to its x component.
	*/
	YXShearMatrix(LinElem amount):
	Matrix()
	{
		vect[1].SetX(amount);
	}
};

/**
* A matrix that can be used to shear vectors in the YZ plane (local
* or absolute), such that their displacements along the y axis is
* proportional to their z components.
* If multiplied by a vector, the matrix will shear that vector
* in the absolute YZ plane.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will shear that vector in the matrix's local YZ plane.
*/
class YZShearMatrix: public Matrix
{
public:

	/**
	* Makes a shear matrix which will displace vectors along the y
	* axis such that their displacement is proportional to their
	* z components.
	* @param amount The proportionality constant that relates a vector's
	* displacement along the y axis to its z component.
	*/
	YZShearMatrix(LinElem amount):
	Matrix()
	{
		vect[1].SetZ(amount);
	}
};

/**
* A matrix that can be used to shear vectors in the XZ plane (local
* or absolute), such that their displacements along the x axis is
* proportional to their z components.
* If multiplied by a vector, the matrix will shear that vector
* in the absolute XZ plane.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will shear that vector in the matrix's local XZ plane.
*/
class ZXShearMatrix: public Matrix
{
public:

	/**
	* Makes a shear matrix which will displace vectors along the z
	* axis such that their displacement is proportional to their
	* x components.
	* @param amount The proportionality constant that relates a vector's
	* displacement along the z axis to its x component.
	*/
	ZXShearMatrix(LinElem amount):
	Matrix()
	{
		vect[2].SetX(amount);
	}
};

/**
* A matrix that can be used to shear vectors in the YZ plane (local
* or absolute), such that their displacements along the z axis is
* proportional to their y components.
* If multiplied by a vector, the matrix will shear that vector
* in the absolute YZ plane.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will shear that vector in the matrix's local YZ plane.
*/
class ZYShearMatrix: public Matrix
{
public:

	/**
	* Makes a shear matrix which will displace vectors along the z
	* axis such that their displacement is proportional to their
	* y components.
	* @param amount The proportionality constant that relates a vector's
	* displacement along the z axis to its y component.
	*/
	ZYShearMatrix(LinElem amount):
	Matrix()
	{
		vect[2].SetY(amount);
	}
};

/**
* A matrix that can be used to rotate a point around an X Axis (local
* or absolute).
* If multiplied by a vector, the matrix will rotate that vector
* around the absolute X-Axis.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will rotate that vector around the transformation matrix's local
* x axis.
*/
class XRotationMatrix: public Matrix
{
public:
	/** Creates a matrix that can be used to rotate a vector
	* around a x-axis by a certain angle.
	* @param ang The angle by which to rotate around the
	* x-axis, in radians.
	*/
	XRotationMatrix(LinElem ang):
	Matrix()
	{
		LinElem c = cos(ang);
		LinElem s = sin(ang);
		vect[1].SetY(c);
		vect[1].SetZ(-s);
		vect[2].SetY(s);
		vect[2].SetZ(c);
	}
};

/**
* A matrix that can be used to rotate a point around a Y Axis (local
* or absolute).
* If multiplied by a vector, the matrix will rotate that vector
* around the absolute Y-Axis.
* If this matrix is multiplied by another matrix, it will produce
* a new transformation matrix which, when multiplied by a vector,
* will rotate that vector around the transformation matrix's local
* Y axis.
*/
class YRotationMatrix: public Matrix
{
public:
	/** Creates a matrix that can be used to rotate a vector
	 * around a y-axis by a certain angle.
	 * @param ang The angle by which to rotate around the
	 * y-axis, in radians.
	 */
	YRotationMatrix(LinElem ang):
	Matrix()
	{
		LinElem c = cos(ang);
		LinElem s = sin(ang);
		vect[0].SetX(c);
		vect[0].SetZ(s);
		vect[2].SetX(-s);
		vect[2].SetZ(c);
	}
};

/** 
 * A matrix that can be used to rotate a point around a Z Axis (local
 * or absolute).
 * If multiplied by a vector, the matrix will rotate that vector
 * around the absolute Z-Axis.
 * If this matrix is multiplied by another matrix, it will produce
 * a new transformation matrix which, when multiplied by a vector,
 * will rotate that vector around the transformation matrix's local
 * z axis.
 */
class ZRotationMatrix: public Matrix
{
public:
	/** Creates a matrix that can be used to rotate a vector
	* around a z-axis by a certain angle.
	* @param ang The angle by which to rotate around the
	* z-axis, in radians.
	*/
	ZRotationMatrix(LinElem ang):
	Matrix()
	{
		LinElem c = cos(ang);
		LinElem s = sin(ang);
		vect[0].SetX(c);
		vect[0].SetY(-s);
		vect[1].SetX(s);
		vect[1].SetY(c);
	}
};

//TODO
class ProjectionMatrix: public Matrix
{
public:
	ProjectionMatrix(LinElem d):
	Matrix()
	{
		vect[2].SetZ(0.0);
		vect[3].SetZ(1.0/d);
	}
};
#endif
