#pragma once
/*
 * cont.h
 * Classes for continuity
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

#include "CalChartCoord.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

// what follows it effectively the Abstract Syntax Tree for CalChart continuities.
//
// Everything derives from a ContToken, which is the basic parsing unit.
// To make the objects more "value" type, there is a clone and is_equal function they
// all implement.
// We use boost::serialize to archive and restore these objects.
// We have both raw pointer constructors and unique_ptr constructors.
// The raw pointer constructors come from the cont parser, and implies that the newly
// formed objects owns the pointers it was given

namespace CalChart {

enum ContDefinedValue {
    CC_N,
    CC_NW,
    CC_W,
    CC_SW,
    CC_S,
    CC_SE,
    CC_E,
    CC_NE,
    CC_HS,
    CC_MM,
    CC_SH,
    CC_JS,
    CC_GV,
    CC_M,
    CC_DM
};

enum class ContType {
    procedure,
    value,
    function,
    direction,
    steptype,
    point,
    unset,
};

class ContToken;
class AnimationCompile;

// DrawableCont is a structure that describes the continuity for drawing
struct DrawableCont {
    ContToken const* self_ptr;
    ContToken* parent_ptr;
    ContType type = ContType::procedure;
    std::string description;
    std::string short_description;
    std::vector<DrawableCont> args;
};

// A note about serialization/deserialation.
// Serialization is straight forward; each object can serialize itself, members
// and parents into a vector of bytes.
// Deserializtion is a little more complicated.  Essentially we give the
// object a pointer to the beginning of a datablob and the end.  It will deserialize
// members and super objects into it.  It returns where it ended.  If at the end
// where it ends is the actual end, it was a good parse
//

class ContToken {
public:
    ContToken();
    virtual ~ContToken() = default;
    virtual std::ostream& Print(std::ostream&) const;
    void SetParentPtr(ContToken* p) { parent_ptr = p; }
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v);

    virtual std::vector<uint8_t> Serialize() const;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end);

    uint32_t line, col;

protected:
    ContToken* parent_ptr = nullptr;

    friend bool operator==(ContToken const& lhs, ContToken const& rhs);
    virtual bool is_equal(ContToken const& other) const
    {
        return line == other.line && col == other.col;
    }
};

inline std::ostream& operator<<(std::ostream& os, const ContToken& c)
{
    return c.Print(os);
}

inline bool operator==(ContToken const& lhs, ContToken const& rhs)
{
    return (typeid(lhs) == typeid(rhs)) && lhs.is_equal(rhs);
}

class ContPoint : public ContToken {
    using super = ContToken;

public:
    ContPoint() = default;
    virtual Coord Get(AnimationCompile& anim) const;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const;
    virtual std::unique_ptr<ContPoint> clone() const;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other);
    }
};

class ContPointUnset : public ContPoint {
    using super = ContPoint;

public:
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContPoint> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;
};

class ContStartPoint : public ContPoint {
    using super = ContPoint;

public:
    ContStartPoint() = default;
    virtual Coord Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContPoint> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other);
    }
};

class ContNextPoint : public ContPoint {
    using super = ContPoint;

public:
    ContNextPoint() = default;
    virtual Coord Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContPoint> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other);
    }
};

class ContRefPoint : public ContPoint {
    using super = ContPoint;

public:
    ContRefPoint() = default;
    ContRefPoint(unsigned n);
    virtual Coord Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContPoint> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && refnum == dynamic_cast<ContRefPoint const&>(other).refnum;
    }

private:
    uint32_t refnum{};
};

class ContValue : public ContToken {
    using super = ContToken;

public:
    ContValue() = default;
    virtual float Get(AnimationCompile& anim) const = 0;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const = 0;
    virtual std::unique_ptr<ContValue> clone() const = 0;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;
};

class ContValueUnset : public ContValue {
    using super = ContValue;

public:
    virtual float Get(AnimationCompile&) const override { return 0; }
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;
};

class ContValueFloat : public ContValue {
    using super = ContValue;

public:
    ContValueFloat() = default;
    ContValueFloat(float v);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && val == dynamic_cast<ContValueFloat const&>(other).val;
    }

private:
    float val{};
};

class ContValueDefined : public ContValue {
    using super = ContValue;

public:
    ContValueDefined() = default;
    ContValueDefined(ContDefinedValue v);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && val == dynamic_cast<ContValueDefined const&>(other).val;
    }

private:
    ContDefinedValue val{ CC_N };
};

class ContValueAdd : public ContValue {
    using super = ContValue;

public:
    ContValueAdd() = default;
    ContValueAdd(ContValue* v1, ContValue* v2);
    ContValueAdd(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContValueAdd const&>(other);
        return super::is_equal(other) && (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContValue> val1, val2;
};

class ContValueSub : public ContValue {
    using super = ContValue;

public:
    ContValueSub() = default;
    ContValueSub(ContValue* v1, ContValue* v2);
    ContValueSub(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContValueSub const&>(other);
        return super::is_equal(other) && (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContValue> val1, val2;
};

class ContValueMult : public ContValue {
    using super = ContValue;

public:
    ContValueMult() = default;
    ContValueMult(ContValue* v1, ContValue* v2);
    ContValueMult(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContValueMult const&>(other);
        return super::is_equal(other) && (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContValue> val1, val2;
};

class ContValueDiv : public ContValue {
    using super = ContValue;

public:
    ContValueDiv() = default;
    ContValueDiv(ContValue* v1, ContValue* v2);
    ContValueDiv(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContValueDiv const&>(other);
        return super::is_equal(other) && (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContValue> val1, val2;
};

class ContValueNeg : public ContValue {
    using super = ContValue;

public:
    ContValueNeg() = default;
    ContValueNeg(ContValue* v);
    ContValueNeg(std::unique_ptr<ContValue> v);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && (*val == *dynamic_cast<ContValueNeg const&>(other).val);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContValue> val;
};

class ContValueREM : public ContValue {
    using super = ContValue;

public:
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && true;
    }
};

class ContValueVar : public ContValue {
    using super = ContValue;

public:
    ContValueVar() = default;
    ContValueVar(unsigned num);
    virtual float Get(AnimationCompile& anim) const override;
    void Set(AnimationCompile& anim, float v);
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && (varnum == dynamic_cast<ContValueVar const&>(other).varnum);
    }

private:
    uint8_t varnum{};
};

class ContValueVarUnset : public ContValueVar {
    using super = ContValueVar;

public:
    virtual float Get(AnimationCompile&) const override { return 0; }
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;
};

class ContFuncDir : public ContValue {
    using super = ContValue;

public:
    ContFuncDir() = default;
    ContFuncDir(ContPoint* p);
    ContFuncDir(std::unique_ptr<ContPoint> p);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && (*pnt == *dynamic_cast<ContFuncDir const&>(other).pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContPoint> pnt;
};

class ContFuncDirFrom : public ContValue {
    using super = ContValue;

public:
    ContFuncDirFrom() = default;
    ContFuncDirFrom(ContPoint* start, ContPoint* end);
    ContFuncDirFrom(std::unique_ptr<ContPoint> start, std::unique_ptr<ContPoint> end);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContFuncDirFrom const&>(other);
        return super::is_equal(other) && (*pnt_start == *der_other.pnt_start) && (*pnt_end == *der_other.pnt_end);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContPoint> pnt_start, pnt_end;
};

class ContFuncDist : public ContValue {
    using super = ContValue;

public:
    ContFuncDist() = default;
    ContFuncDist(ContPoint* p);
    ContFuncDist(std::unique_ptr<ContPoint> p);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && (*pnt == *dynamic_cast<ContFuncDist const&>(other).pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContPoint> pnt;
};

class ContFuncDistFrom : public ContValue {
    using super = ContValue;

public:
    ContFuncDistFrom() = default;
    ContFuncDistFrom(ContPoint* start, ContPoint* end);
    ContFuncDistFrom(std::unique_ptr<ContPoint> start, std::unique_ptr<ContPoint> end);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContFuncDistFrom const&>(other);
        return super::is_equal(other) && (*pnt_start == *der_other.pnt_start) && (*pnt_end == *der_other.pnt_end);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContPoint> pnt_start, pnt_end;
};

class ContFuncEither : public ContValue {
    using super = ContValue;

public:
    ContFuncEither() = default;
    ContFuncEither(ContValue* d1, ContValue* d2, ContPoint* p);
    ContFuncEither(std::unique_ptr<ContValue> d1, std::unique_ptr<ContValue> d2, std::unique_ptr<ContPoint> p);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContFuncEither const&>(other);
        return super::is_equal(other) && (*dir1 == *der_other.dir1) && (*dir2 == *der_other.dir2) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<ContValue> dir1, dir2;
    std::unique_ptr<ContPoint> pnt;
};

class ContFuncOpp : public ContValue {
    using super = ContValue;

public:
    ContFuncOpp() = default;
    ContFuncOpp(ContValue* d);
    ContFuncOpp(std::unique_ptr<ContValue> d);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && (*dir == *dynamic_cast<ContFuncOpp const&>(other).dir);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContValue> dir;
};

class ContFuncStep : public ContValue {
    using super = ContValue;

public:
    ContFuncStep() = default;
    ContFuncStep(ContValue* beats, ContValue* blocksize, ContPoint* p);
    ContFuncStep(std::unique_ptr<ContValue> beats, std::unique_ptr<ContValue> blocksize, std::unique_ptr<ContPoint> p);
    virtual float Get(AnimationCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContFuncStep const&>(other);
        return super::is_equal(other) && (*numbeats == *der_other.numbeats) && (*blksize == *der_other.blksize) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<ContValue> numbeats, blksize;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcedure : public ContToken {
    using super = ContToken;

public:
    ContProcedure() = default;
    virtual void Compile(AnimationCompile& anim) = 0;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const = 0;
    virtual std::unique_ptr<ContProcedure> clone() const = 0;
    virtual bool IsValid() const { return true; }

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;
};

class ContProcUnset : public ContProcedure {
    using super = ContProcedure;

public:
    virtual void Compile(AnimationCompile&) override { }
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual bool IsValid() const override { return false; }

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;
};

class ContProcSet : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcSet() = default;
    ContProcSet(ContValueVar* vr, ContValue* v);
    ContProcSet(std::unique_ptr<ContValueVar> vr, std::unique_ptr<ContValue> v);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    struct ReplaceError_NotAVar : public std::exception {
    };
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcSet const&>(other);
        return super::is_equal(other) && (*var == *der_other.var) && (*val == *der_other.val);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContValueVar> var;
    std::unique_ptr<ContValue> val;
};

class ContProcBlam : public ContProcedure {
    using super = ContProcedure;

public:
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return super::is_equal(other) && true;
    }
};

class ContProcCM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcCM() = default;
    ContProcCM(ContPoint* p1, ContPoint* p2, ContValue* steps, ContValue* d1,
        ContValue* d2, ContValue* beats);
    ContProcCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d1,
        std::unique_ptr<ContValue> d2, std::unique_ptr<ContValue> beats);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcCM const&>(other);
        return super::is_equal(other) && (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*stps == *der_other.stps) && (*dir1 == *der_other.dir1) && (*dir2 == *der_other.dir2) && (*numbeats == *der_other.numbeats);
    }

private:
    static constexpr auto NumParts = 6;
    std::unique_ptr<ContPoint> pnt1, pnt2;
    std::unique_ptr<ContValue> stps, dir1, dir2, numbeats;
};

class ContProcDMCM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcDMCM() = default;
    ContProcDMCM(ContPoint* p1, ContPoint* p2, ContValue* beats);
    ContProcDMCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> beats);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcDMCM const&>(other);
        return super::is_equal(other) && (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*numbeats == *der_other.numbeats);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<ContPoint> pnt1, pnt2;
    std::unique_ptr<ContValue> numbeats;
};

class ContProcDMHS : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcDMHS() = default;
    ContProcDMHS(ContPoint* p);
    ContProcDMHS(std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcDMHS const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcEven : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcEven() = default;
    ContProcEven(ContValue* steps, ContPoint* p);
    ContProcEven(std::unique_ptr<ContValue> steps, std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcEven const&>(other);
        return super::is_equal(other) && (*stps == *der_other.stps) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContValue> stps;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcEWNS : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcEWNS() = default;
    ContProcEWNS(ContPoint* p);
    ContProcEWNS(std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcEWNS const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcFountain : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcFountain() = default;
    ContProcFountain(ContValue* d1, ContValue* d2, ContValue* s1, ContValue* s2,
        ContPoint* p);
    ContProcFountain(std::unique_ptr<ContValue> d1, std::unique_ptr<ContValue> d2, std::unique_ptr<ContValue> s1, std::unique_ptr<ContValue> s2,
        std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        if (!super::is_equal(other)) {
            return false;
        }

        auto&& der_other = dynamic_cast<ContProcFountain const&>(other);
        // stepsize1, 2 can be null.  So this gets complicated
        if (!((*dir1 == *der_other.dir1) && (*dir2 == *der_other.dir2) && (*pnt == *der_other.pnt))) {
            return false;
        }
        if (stepsize1 || der_other.stepsize1) {
            return (stepsize1 && der_other.stepsize1) ? (*stepsize1 == *der_other.stepsize1) : false;
        }
        if (stepsize2 || der_other.stepsize2) {
            return (stepsize2 && der_other.stepsize2) ? (*stepsize2 == *der_other.stepsize2) : false;
        }
        return true;
    }

private:
    static constexpr auto NumParts = 5;
    std::unique_ptr<ContValue> dir1, dir2;
    std::unique_ptr<ContValue> stepsize1, stepsize2;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcFM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcFM() = default;
    ContProcFM(ContValue* steps, ContValue* d);
    ContProcFM(std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcFM const&>(other);
        return super::is_equal(other) && (*stps == *der_other.stps) && (*dir == *der_other.dir);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContValue> stps, dir;
};

class ContProcFMTO : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcFMTO() = default;
    ContProcFMTO(ContPoint* p);
    ContProcFMTO(std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcFMTO const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcGrid : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcGrid() = default;
    ContProcGrid(ContValue* g);
    ContProcGrid(std::unique_ptr<ContValue> g);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcGrid const&>(other);
        return super::is_equal(other) && (*grid == *der_other.grid);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContValue> grid;
};

class ContProcHSCM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcHSCM() = default;
    ContProcHSCM(ContPoint* p1, ContPoint* p2, ContValue* beats);
    ContProcHSCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> beats);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcHSCM const&>(other);
        return super::is_equal(other) && (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*numbeats == *der_other.numbeats);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<ContPoint> pnt1, pnt2;
    std::unique_ptr<ContValue> numbeats;
};

class ContProcHSDM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcHSDM() = default;
    ContProcHSDM(ContPoint* p);
    ContProcHSDM(std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcHSDM const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcMagic : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcMagic() = default;
    ContProcMagic(ContPoint* p);
    ContProcMagic(std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcMagic const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcMarch : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcMarch() = default;
    ContProcMarch(ContValue* stepsize, ContValue* steps, ContValue* d,
        ContValue* face);
    ContProcMarch(std::unique_ptr<ContValue> stepsize, std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d,
        std::unique_ptr<ContValue> face);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        if (!super::is_equal(other)) {
            return false;
        }
        auto&& der_other = dynamic_cast<ContProcMarch const&>(other);
        // facedir can be null.  So this gets complicated
        if (!((*stpsize == *der_other.stpsize) && (*stps == *der_other.stps) && (*dir == *der_other.dir))) {
            return false;
        }
        if (facedir || der_other.facedir) {
            return (facedir && der_other.facedir) ? (*facedir == *der_other.facedir) : false;
        }
        return true;
    }

private:
    static constexpr auto NumParts = 4;
    std::unique_ptr<ContValue> stpsize, stps, dir, facedir;
};

class ContProcMT : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcMT() = default;
    ContProcMT(ContValue* beats, ContValue* d);
    ContProcMT(std::unique_ptr<ContValue> beats, std::unique_ptr<ContValue> d);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcMT const&>(other);
        return super::is_equal(other) && (*numbeats == *der_other.numbeats) && (*dir == *der_other.dir);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ContValue> numbeats, dir;
};

class ContProcMTRM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcMTRM() = default;
    ContProcMTRM(ContValue* d);
    ContProcMTRM(std::unique_ptr<ContValue> d);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcMTRM const&>(other);
        return super::is_equal(other) && (*dir == *der_other.dir);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContValue> dir;
};

class ContProcNSEW : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcNSEW() = default;
    ContProcNSEW(ContPoint* p);
    ContProcNSEW(std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcNSEW const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcRotate : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcRotate() = default;
    ContProcRotate(ContValue* angle, ContValue* steps, ContPoint* p);
    ContProcRotate(std::unique_ptr<ContValue> angle, std::unique_ptr<ContValue> steps, std::unique_ptr<ContPoint> p);
    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;
    virtual void replace(ContToken const* which, std::unique_ptr<ContToken> v) override;

    virtual std::vector<uint8_t> Serialize() const override;
    virtual uint8_t const* Deserialize(uint8_t const* begin, uint8_t const* end) override;

protected:
    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcRotate const&>(other);
        return super::is_equal(other) && (*ang == *der_other.ang) && (*stps == *der_other.stps) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<ContValue> ang, stps;
    std::unique_ptr<ContPoint> pnt;
};

// this is the top level Deserialization function
std::tuple<std::unique_ptr<ContProcedure>, uint8_t const*> DeserializeContProcedure(uint8_t const* begin, uint8_t const* end);

}
