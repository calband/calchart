#pragma once
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

#include <math.h>

template <typename LinElem>
class Vector {
public:
    Vector()
    {
        elem[0] = elem[1] = elem[2] = 0.0;
        elem[3] = 1.0;
    }
    Vector(LinElem x, LinElem y, LinElem z, LinElem w = 1.0)
    {
        elem[0] = x;
        elem[1] = y;
        elem[2] = z;
        elem[3] = w;
    }
    Vector(const Vector& ref) { *this = ref; }
    inline LinElem Get(unsigned i) const { return elem[i]; }
    inline LinElem GetX() const { return elem[0]; }
    inline LinElem GetY() const { return elem[1]; }
    inline LinElem GetZ() const { return elem[2]; }
    inline LinElem GetW() const { return elem[3]; }

    inline void Set(int i, LinElem val) { elem[i] = val; }
    inline void SetX(LinElem val) { elem[0] = val; }
    inline void SetY(LinElem val) { elem[1] = val; }
    inline void SetZ(LinElem val) { elem[2] = val; }
    inline void SetW(LinElem val) { elem[3] = val; }

    inline void Homogenize()
    {
        elem[0] /= elem[3];
        elem[1] /= elem[3];
        elem[2] /= elem[3];
        elem[3] = 1;
    }

    inline const Vector& operator=(const Vector& v)
    {
        elem[0] = v.elem[0];
        elem[1] = v.elem[1];
        elem[2] = v.elem[2];
        elem[3] = v.elem[3];
        return *this;
    }
    inline const Vector& operator*=(LinElem s)
    {
        elem[0] *= s;
        elem[1] *= s;
        elem[2] *= s;
        elem[3] *= s;
        return *this;
    }
    inline const Vector& operator/=(LinElem s)
    {
        elem[0] /= s;
        elem[1] /= s;
        elem[2] /= s;
        elem[3] /= s;
        return *this;
    }
    // dot product
    inline friend LinElem operator*(const Vector& a, const Vector& b)
    {
        return a.elem[0] * b.elem[0] + a.elem[1] * b.elem[1] + a.elem[2] * b.elem[2] + a.elem[3] * b.elem[3];
    }
    // Component-wise multiplication
    inline friend Vector operator|(const Vector& a, const Vector& b)
    {
        return Vector(a.elem[0] * b.elem[0], a.elem[1] * b.elem[1],
            a.elem[2] * b.elem[2], a.elem[3] * b.elem[3]);
    }
    inline friend Vector operator+(const Vector& a, const Vector& b)
    {
        return Vector(a.elem[0] + b.elem[0], a.elem[1] + b.elem[1],
            a.elem[2] + b.elem[2]);
    }
    inline friend Vector operator-(const Vector& a, const Vector& b)
    {
        return Vector(a.elem[0] - b.elem[0], a.elem[1] - b.elem[1],
            a.elem[2] - b.elem[2]);
    }
    inline friend Vector operator-(const Vector& v)
    {
        return Vector(-v.elem[0], -v.elem[1], -v.elem[2], v.elem[3]);
    }

protected:
    LinElem elem[4];
};

template <typename LinElem>
class Matrix {
public:
    Matrix()
    {
        vect[0] = Vector<LinElem>(1.0, 0.0, 0.0, 0.0);
        vect[1] = Vector<LinElem>(0.0, 1.0, 0.0, 0.0);
        vect[2] = Vector<LinElem>(0.0, 0.0, 1.0, 0.0);
        vect[3] = Vector<LinElem>(0.0, 0.0, 0.0, 1.0);
    }
    Matrix(const Matrix& ref) { *this = ref; }
    Matrix(const Vector<LinElem>& v0, const Vector<LinElem>& v1, const Vector<LinElem>& v2,
        const Vector<LinElem>& v3)
    {
        vect[0] = v0;
        vect[1] = v1;
        vect[2] = v2;
        vect[3] = v3;
    }
    inline LinElem GetElem(int u, int v) const { return vect[u].Get(v); }
    inline void SetElem(int u, int v, LinElem val) { vect[u].Set(v, val); }

    inline Matrix GetTranspose() const
    {
        return Matrix(
            Vector(vect[0].Get(0), vect[1].Get(0), vect[2].Get(0), vect[3].Get(0)),
            Vector(vect[0].Get(1), vect[1].Get(1), vect[2].Get(1), vect[3].Get(1)),
            Vector(vect[0].Get(2), vect[1].Get(2), vect[2].Get(2), vect[3].Get(2)),
            Vector(vect[0].Get(3), vect[1].Get(3), vect[2].Get(3), vect[3].Get(3)));
    }
    inline const Matrix& operator=(const Matrix& m)
    {
        vect[0] = m.vect[0];
        vect[1] = m.vect[1];
        vect[2] = m.vect[2];
        vect[3] = m.vect[3];
        return *this;
    }
    inline const Matrix& operator*=(const Matrix& m)
    {
        *this = *this * m;
        return *this;
    }
    inline const Matrix& operator*=(LinElem s)
    {
        vect[0] *= s;
        vect[1] *= s;
        vect[2] *= s;
        vect[3] *= s;
        return *this;
    }
    inline const Matrix& operator/=(LinElem s)
    {
        vect[0] /= s;
        vect[1] /= s;
        vect[2] /= s;
        vect[3] /= s;
        return *this;
    }
    inline friend Matrix operator*(const Matrix& a, const Matrix& b)
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
    inline friend Vector<LinElem> operator*(const Matrix& a, const Vector<LinElem>& b)
    {
        return Vector(a.vect[0] * b, a.vect[1] * b, a.vect[2] * b, a.vect[3] * b);
    }

protected:
    Vector<LinElem> vect[4];
};

template <typename LinElem>
class TranslationMatrix : public Matrix<LinElem> {
public:
    TranslationMatrix(const Vector<LinElem>& v)
        : Matrix<LinElem>()
    {
        // yes, the this is necessary here:
        // https://yunmingzhang.wordpress.com/2019/01/26/accessing-template-base-class-members-in-c/
        this->vect[0].SetW(v.GetX());
        this->vect[1].SetW(v.GetY());
        this->vect[2].SetW(v.GetZ());
    }
};

template <typename LinElem>
class ScaleMatrix : public Matrix<LinElem> {
public:
    ScaleMatrix(const Vector<LinElem>& v)
    {
        this->vect[0] = Vector<LinElem>(v.GetX(), 0.0, 0.0, 0.0);
        this->vect[1] = Vector<LinElem>(0.0, v.GetY(), 0.0, 0.0);
        this->vect[2] = Vector<LinElem>(0.0, 0.0, v.GetZ(), 0.0);
        this->vect[3] = Vector<LinElem>(0.0, 0.0, 0.0, 1.0);
    }
};

template <typename LinElem>
class XReflectionMatrix : public Matrix<LinElem> {
public:
    XReflectionMatrix()
        : Matrix<LinElem>()
    {
        this->vect[0].SetX(-1);
    }
};

template <typename LinElem>
class YReflectionMatrix : public Matrix<LinElem> {
public:
    YReflectionMatrix()
        : Matrix<LinElem>()
    {
        this->vect[1].SetY(-1);
    }
};

template <typename LinElem>
class ZReflectionMatrix : public Matrix<LinElem> {
public:
    ZReflectionMatrix()
        : Matrix<LinElem>()
    {
        this->vect[2].SetZ(-1);
    }
};

template <typename LinElem>
class XYShearMatrix : public Matrix<LinElem> {
public:
    XYShearMatrix(LinElem amount)
        : Matrix<LinElem>()
    {
        this->vect[0].SetY(amount);
    }
};

template <typename LinElem>
class XZShearMatrix : public Matrix<LinElem> {
public:
    XZShearMatrix(LinElem amount)
        : Matrix<LinElem>()
    {
        this->vect[0].SetZ(amount);
    }
};

template <typename LinElem>
class YXShearMatrix : public Matrix<LinElem> {
public:
    YXShearMatrix(LinElem amount)
        : Matrix<LinElem>()
    {
        this->vect[1].SetX(amount);
    }
};

template <typename LinElem>
class YZShearMatrix : public Matrix<LinElem> {
public:
    YZShearMatrix(LinElem amount)
        : Matrix<LinElem>()
    {
        this->vect[1].SetZ(amount);
    }
};

template <typename LinElem>
class ZXShearMatrix : public Matrix<LinElem> {
public:
    ZXShearMatrix(LinElem amount)
        : Matrix<LinElem>()
    {
        this->vect[2].SetX(amount);
    }
};

template <typename LinElem>
class ZYShearMatrix : public Matrix<LinElem> {
public:
    ZYShearMatrix(LinElem amount)
        : Matrix<LinElem>()
    {
        this->vect[2].SetY(amount);
    }
};

template <typename LinElem>
class XRotationMatrix : public Matrix<LinElem> {
public:
    XRotationMatrix(LinElem ang)
        : Matrix<LinElem>()
    {
        LinElem c = cos(ang);
        LinElem s = sin(ang);
        this->vect[1].SetY(c);
        this->vect[1].SetZ(-s);
        this->vect[2].SetY(s);
        this->vect[2].SetZ(c);
    }
};

template <typename LinElem>
class YRotationMatrix : public Matrix<LinElem> {
public:
    YRotationMatrix(LinElem ang)
        : Matrix<LinElem>()
    {
        LinElem c = cos(ang);
        LinElem s = sin(ang);
        this->vect[0].SetX(c);
        this->vect[0].SetZ(s);
        this->vect[2].SetX(-s);
        this->vect[2].SetZ(c);
    }
};

template <typename LinElem>
class ZRotationMatrix : public Matrix<LinElem> {
public:
    ZRotationMatrix(LinElem ang)
        : Matrix<LinElem>()
    {
        LinElem c = cos(ang);
        LinElem s = sin(ang);
        this->vect[0].SetX(c);
        this->vect[0].SetY(-s);
        this->vect[1].SetX(s);
        this->vect[1].SetY(c);
    }
};

template <typename LinElem>
class ProjectionMatrix : public Matrix<LinElem> {
public:
    ProjectionMatrix(LinElem d)
        : Matrix<LinElem>()
    {
        this->vect[2].SetZ(0.0);
        this->vect[3].SetZ(1.0 / d);
    }
};
