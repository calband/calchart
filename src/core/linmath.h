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

typedef float LinElem;
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

class Matrix {
public:
    Matrix()
    {
        vect[0] = Vector(1.0, 0.0, 0.0, 0.0);
        vect[1] = Vector(0.0, 1.0, 0.0, 0.0);
        vect[2] = Vector(0.0, 0.0, 1.0, 0.0);
        vect[3] = Vector(0.0, 0.0, 0.0, 1.0);
    }
    Matrix(const Matrix& ref) { *this = ref; }
    Matrix(const Vector& v0, const Vector& v1, const Vector& v2,
        const Vector& v3)
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
    inline friend Vector operator*(const Matrix& a, const Vector& b)
    {
        return Vector(a.vect[0] * b, a.vect[1] * b, a.vect[2] * b, a.vect[3] * b);
    }

protected:
    Vector vect[4];
};

class TranslationMatrix : public Matrix {
public:
    TranslationMatrix(const Vector& v)
        : Matrix()
    {
        vect[0].SetW(v.GetX());
        vect[1].SetW(v.GetY());
        vect[2].SetW(v.GetZ());
    }
};

class ScaleMatrix : public Matrix {
public:
    ScaleMatrix(const Vector& v)
    {
        vect[0] = Vector(v.GetX(), 0.0, 0.0, 0.0);
        vect[1] = Vector(0.0, v.GetY(), 0.0, 0.0);
        vect[2] = Vector(0.0, 0.0, v.GetZ(), 0.0);
        vect[3] = Vector(0.0, 0.0, 0.0, 1.0);
    }
};

class XReflectionMatrix : public Matrix {
public:
    XReflectionMatrix()
        : Matrix()
    {
        vect[0].SetX(-1);
    }
};

class YReflectionMatrix : public Matrix {
public:
    YReflectionMatrix()
        : Matrix()
    {
        vect[1].SetY(-1);
    }
};

class ZReflectionMatrix : public Matrix {
public:
    ZReflectionMatrix()
        : Matrix()
    {
        vect[2].SetZ(-1);
    }
};

class XYShearMatrix : public Matrix {
public:
    XYShearMatrix(LinElem amount)
        : Matrix()
    {
        vect[0].SetY(amount);
    }
};

class XZShearMatrix : public Matrix {
public:
    XZShearMatrix(LinElem amount)
        : Matrix()
    {
        vect[0].SetZ(amount);
    }
};

class YXShearMatrix : public Matrix {
public:
    YXShearMatrix(LinElem amount)
        : Matrix()
    {
        vect[1].SetX(amount);
    }
};

class YZShearMatrix : public Matrix {
public:
    YZShearMatrix(LinElem amount)
        : Matrix()
    {
        vect[1].SetZ(amount);
    }
};

class ZXShearMatrix : public Matrix {
public:
    ZXShearMatrix(LinElem amount)
        : Matrix()
    {
        vect[2].SetX(amount);
    }
};

class ZYShearMatrix : public Matrix {
public:
    ZYShearMatrix(LinElem amount)
        : Matrix()
    {
        vect[2].SetY(amount);
    }
};

class XRotationMatrix : public Matrix {
public:
    XRotationMatrix(LinElem ang)
        : Matrix()
    {
        LinElem c = cos(ang);
        LinElem s = sin(ang);
        vect[1].SetY(c);
        vect[1].SetZ(-s);
        vect[2].SetY(s);
        vect[2].SetZ(c);
    }
};

class YRotationMatrix : public Matrix {
public:
    YRotationMatrix(LinElem ang)
        : Matrix()
    {
        LinElem c = cos(ang);
        LinElem s = sin(ang);
        vect[0].SetX(c);
        vect[0].SetZ(s);
        vect[2].SetX(-s);
        vect[2].SetZ(c);
    }
};

class ZRotationMatrix : public Matrix {
public:
    ZRotationMatrix(LinElem ang)
        : Matrix()
    {
        LinElem c = cos(ang);
        LinElem s = sin(ang);
        vect[0].SetX(c);
        vect[0].SetY(-s);
        vect[1].SetX(s);
        vect[1].SetY(c);
    }
};

class ProjectionMatrix : public Matrix {
public:
    ProjectionMatrix(LinElem d)
        : Matrix()
    {
        vect[2].SetZ(0.0);
        vect[3].SetZ(1.0 / d);
    }
};
