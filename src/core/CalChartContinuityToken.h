#pragma once
/*
 * CalChartContinuityToken.h
 * Classes for ContinuityTokens
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

/**
 * CalChartContinuityToken
 *
 *  Continuities in CalChart are quite complicated as we want to provide a flexible data structure that can represent a number of
 *  different realizations.  To illustrate, we would want to have a data structure that could be used for the inuity "Mark Time 8
 *  in the direction of the last flow" as well as "Mark Time remaining facing East".  This is accomplished by having a data structure
 *  of "tokens" that can represent different parts of the inuity "sentance".
 *
 *  To accomplish this "abstract syntax tree" data structure we have the basic concept of a inuityToken, which is the basic
 *  parsing unit.  The inuityToken can have a "parent" which allows bidirectional searching of the tree (any node can then reach
 *  each other node).  We then have these inuity specialization "types"
 *
 *      procedure : A inuityToken that represents a inuity that a marcher will follow.
 *      value : A inuityToken that represents a specific Value, like a float
 *      function : A Value that has composes one or more Values for a new Value.
 *      direction : A Value that is specifically a Direction.
 *      steptype : A Value that is specifically a step type.
 *      point : A inuityToken that represents a Point on the field.
 *      unset : A special inuityToken that represents that the type has not yet been specified.
 *
 *  With these building blocks we can represent a large number of inuities.  For example, let's say that the inuity we would
 *  like to describe is "Mark Time East for the number of beats a reference point would take to reach this location", typical of a step drill.
 *  This would be represented as:
 *
 * ProcMT ( FuncDistFrom ( StartPoint , RefPoint(1) ) , Value ( E ) ) )
 *
 *  In a tree view this would look something like:
 *
 *               ProcMT
 *                 /              \
 *      FuncDistFrom       Value(E)
 *         /         \
 * StartPoint  RefPoint(1)
 *
 *  As each part of the tree is a Continuity::Token, creating drawing representation can be done in a straightforward recursive way, with
 *  each Continuity::Token specialization simply calling the drawing of each of it's nodes.
 *
 * Get(AnimationCompile):
 *  Point and Value need to be able to supply an actual value.  But because these are abstract representations of a point or value,
 *  they need actual state to act upon.  The AnimationCompile object represents the portion of the show that is being converted from an
 *  abstract concept (the StartPoint for example) to a specific value (the position of a specific marcher on the field.
 *
 * Memory Considerations:
 *  Memory ownership of each node is done by it's parent.  That means that when a new node is inserted, memory ownership should
 *  be transfered to the parent, which may require "setting" the parent node.  In addition, when a inuity needs to be "copied", it
 *  should be "cloned" into a new datastructure to preserve the runtime data structure.
 *
 * Serialization and Deserialization
 *  In order to be saved and restored from a file, the inuites need to be able to be serailized and deserialized.  Serialization is
 *  straight forward; each object can serialize itself and it's children into a vector of bytes.  Deserializtion is a little more complicated.
 *  Essentially we give the object a pointer to the beginning of a datablob and the end.  It will deserialize members and from the data,
 *  and return the data pointer where it ended the parse.  If at the conclusion of the process, if the data pointer end and the original end
 *  match, it was a good parse
 *
 * Style for this file
 *  There is a lot of redundancy on declarations in this file.  We should attempt to keep the file consistent in style and function layout
 *  to make reading and understanding easier.  Whenever it's possible to include implementions in the header file to simplify code, we
 *  will do so.  And similarly whenever we can use a base class implementation without losing clarity we should do so.
 */

#include "CalChartCoord.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace CalChart {

class Reader;
struct AnimationCompile;

}

namespace CalChart::Cont {

enum DefinedValue {
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

// Number of variables in continuity language (A B C D X Y Z DOF DOH)
enum class Variable {
    A,
    B,
    C,
    D,
    X,
    Y,
    Z,
    DOF,
    DOH,
};

enum class DefinedDirection {
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW
};

constexpr auto kNumVariables = static_cast<std::underlying_type_t<Variable>>(Variable::DOH) + 1;

enum class Type {
    procedure,
    value,
    function,
    direction,
    steptype,
    point,
    unset,
};

class Token;

// Drawable is a structure that describes the inuity for drawing
struct Drawable {
    Token const* self_ptr;
    Token* parent_ptr;
    Type type = Type::procedure;
    std::string description;
    std::string short_description;
    std::vector<Drawable> args;
};

// helper to set the parent
template <typename P, typename T, typename... Ts>
void SetParentPtr_helper(P parent, T& t, Ts&... ts);

template <typename T>
auto uniquify(T* p) { return std::unique_ptr<std::remove_pointer_t<decltype(p)>>(p); }

template <typename T>
auto mv(T&& p) { return std::move(p); }

class Token {
public:
    Token();
    virtual ~Token() = default;
    virtual std::ostream& Print(std::ostream&) const;
    void SetParentPtr(Token* p) { parent_ptr = p; }
    virtual void replace(Token const* which, std::unique_ptr<Token> v);

    [[nodiscard]] virtual auto Serialize() const -> std::vector<std::byte>;
    virtual Reader Deserialize(Reader);

    uint32_t line, col;

protected:
    Token* parent_ptr = nullptr;

    friend bool operator==(Token const& lhs, Token const& rhs);
    virtual bool is_equal(Token const& other) const
    {
        return line == other.line && col == other.col;
    }

private:
    static constexpr auto NumParts = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Token& c)
{
    return c.Print(os);
}

inline bool operator==(Token const& lhs, Token const& rhs)
{
    return (typeid(lhs) == typeid(rhs)) && lhs.is_equal(rhs);
}

class Point : public Token {
    using super = Token;

public:
    Point() = default;
    virtual std::unique_ptr<Point> clone() const { return std::make_unique<Point>(); }

    virtual Coord Get(AnimationCompile const& anim) const;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class PointUnset : public Point {
    using super = Point;

public:
    virtual std::unique_ptr<Point> clone() const override { return std::make_unique<PointUnset>(); }

    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class StartPoint : public Point {
    using super = Point;

public:
    StartPoint() = default;
    virtual std::unique_ptr<Point> clone() const override { return std::make_unique<StartPoint>(); }

    virtual Coord Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class NextPoint : public Point {
    using super = Point;

public:
    NextPoint() = default;
    virtual std::unique_ptr<Point> clone() const override { return std::make_unique<NextPoint>(); }

    virtual Coord Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class RefPoint : public Point {
    using super = Point;

public:
    RefPoint() = default;
    RefPoint(unsigned n);
    virtual std::unique_ptr<Point> clone() const override { return std::make_unique<RefPoint>(refnum); }

    virtual Coord Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && refnum == dynamic_cast<RefPoint const&>(other).refnum;
    }

private:
    uint32_t refnum{};

private:
    static constexpr auto NumParts = 0;
};

class Value : public Token {
    using super = Token;

public:
    Value() = default;
    virtual std::unique_ptr<Value> clone() const = 0;

    virtual float Get(AnimationCompile const& anim) const = 0;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const = 0;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class ValueUnset : public Value {
    using super = Value;

public:
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueUnset>(); }

    virtual float Get(AnimationCompile const&) const override { return 0; }
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class ValueFloat : public Value {
    using super = Value;

public:
    ValueFloat() = default;
    ValueFloat(float v);
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueFloat>(val); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && val == dynamic_cast<ValueFloat const&>(other).val;
    }

private:
    float val{};
    static constexpr auto NumParts = 0;
};

class ValueDefined : public Value {
    using super = Value;

public:
    ValueDefined() = default;
    ValueDefined(DefinedValue v);
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueDefined>(val); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && val == dynamic_cast<ValueDefined const&>(other).val;
    }

private:
    DefinedValue val{ CC_N };
    static constexpr auto NumParts = 0;
};

class ValueAdd : public Value {
    using super = Value;

public:
    ValueAdd() = default;
    ValueAdd(Value* v1, Value* v2)
        : ValueAdd(uniquify(v1), uniquify(v2))
    {
    }
    ValueAdd(std::unique_ptr<Value> v1, std::unique_ptr<Value> v2)
        : val1(mv(v1))
        , val2(mv(v2))
    {
        SetParentPtr_helper(this, val1, val2);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueAdd>(val1->clone(), val2->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ValueAdd const&>(other);
        return super::is_equal(other) && (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Value> val1, val2;
};

class ValueSub : public Value {
    using super = Value;

public:
    ValueSub() = default;
    ValueSub(Value* v1, Value* v2)
        : ValueSub(uniquify(v1), uniquify(v2))
    {
    }
    ValueSub(std::unique_ptr<Value> v1, std::unique_ptr<Value> v2)
        : val1(mv(v1))
        , val2(mv(v2))
    {
        SetParentPtr_helper(this, val1, val2);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueSub>(val1->clone(), val2->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ValueSub const&>(other);
        return super::is_equal(other) && (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Value> val1, val2;
};

class ValueMult : public Value {
    using super = Value;

public:
    ValueMult() = default;
    ValueMult(Value* v1, Value* v2)
        : ValueMult(uniquify(v1), uniquify(v2))
    {
    }
    ValueMult(std::unique_ptr<Value> v1, std::unique_ptr<Value> v2)
        : val1(mv(v1))
        , val2(mv(v2))
    {
        SetParentPtr_helper(this, val1, val2);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueMult>(val1->clone(), val2->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ValueMult const&>(other);
        return super::is_equal(other) && (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Value> val1, val2;
};

class ValueDiv : public Value {
    using super = Value;

public:
    ValueDiv() = default;
    ValueDiv(Value* v1, Value* v2)
        : ValueDiv(uniquify(v1), uniquify(v2))
    {
    }
    ValueDiv(std::unique_ptr<Value> v1, std::unique_ptr<Value> v2)
        : val1(mv(v1))
        , val2(mv(v2))
    {
        SetParentPtr_helper(this, val1, val2);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueDiv>(val1->clone(), val2->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ValueDiv const&>(other);
        return super::is_equal(other) && (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Value> val1, val2;
};

class ValueNeg : public Value {
    using super = Value;

public:
    ValueNeg() = default;
    ValueNeg(Value* v)
        : ValueNeg(uniquify(v))
    {
    }
    ValueNeg(std::unique_ptr<Value> v)
        : val(mv(v))
    {
        SetParentPtr_helper(this, val);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueNeg>(val->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && (*val == *dynamic_cast<ValueNeg const&>(other).val);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Value> val;
};

class ValueREM : public Value {
    using super = Value;

public:
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueREM>(); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class ValueVar : public Value {
    using super = Value;

public:
    ValueVar() = default;
    ValueVar(Variable num);
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueVar>(varnum); }

    virtual float Get(AnimationCompile const& anim) const override;
    void Set(AnimationCompile& anim, float v);
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && (varnum == dynamic_cast<ValueVar const&>(other).varnum);
    }

private:
    Variable varnum{};
    static constexpr auto NumParts = 0;
};

class ValueVarUnset : public ValueVar {
    using super = ValueVar;

public:
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<ValueVarUnset>(); }

    virtual float Get(AnimationCompile const&) const override { return 0; }
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class FuncDir : public Value {
    using super = Value;

public:
    FuncDir() = default;
    FuncDir(Point* p)
        : FuncDir(std::unique_ptr<Point>(p))
    {
    }
    FuncDir(std::unique_ptr<Point> p)
        : pnt(mv(p))
    {
        SetParentPtr_helper(this, pnt);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<FuncDir>(pnt->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && (*pnt == *dynamic_cast<FuncDir const&>(other).pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Point> pnt;
};

class FuncDirFrom : public Value {
    using super = Value;

public:
    FuncDirFrom() = default;
    FuncDirFrom(Point* start, Point* end)
        : FuncDirFrom(uniquify(start), uniquify(end))
    {
    }
    FuncDirFrom(std::unique_ptr<Point> start, std::unique_ptr<Point> end)
        : pnt_start(mv(start))
        , pnt_end(mv(end))
    {
        SetParentPtr_helper(this, pnt_start, pnt_end);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<FuncDirFrom>(pnt_start->clone(), pnt_end->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<FuncDirFrom const&>(other);
        return super::is_equal(other) && (*pnt_start == *der_other.pnt_start) && (*pnt_end == *der_other.pnt_end);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Point> pnt_start, pnt_end;
};

class FuncDist : public Value {
    using super = Value;

public:
    FuncDist() = default;
    FuncDist(Point* p)
        : FuncDist(uniquify(p))
    {
    }
    FuncDist(std::unique_ptr<Point> p)
        : pnt(mv(p))
    {
        SetParentPtr_helper(this, pnt);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<FuncDist>(pnt->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && (*pnt == *dynamic_cast<FuncDist const&>(other).pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Point> pnt;
};

class FuncDistFrom : public Value {
    using super = Value;

public:
    FuncDistFrom() = default;
    FuncDistFrom(Point* start, Point* end)
        : FuncDistFrom(uniquify(start), uniquify(end))
    {
    }
    FuncDistFrom(std::unique_ptr<Point> start, std::unique_ptr<Point> end)
        : pnt_start(mv(start))
        , pnt_end(mv(end))
    {
        SetParentPtr_helper(this, pnt_start, pnt_end);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<FuncDistFrom>(pnt_start->clone(), pnt_end->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<FuncDistFrom const&>(other);
        return super::is_equal(other) && (*pnt_start == *der_other.pnt_start) && (*pnt_end == *der_other.pnt_end);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Point> pnt_start, pnt_end;
};

class FuncEither : public Value {
    using super = Value;

public:
    FuncEither() = default;
    FuncEither(Value* d1, Value* d2, Point* p)
        : FuncEither(uniquify(d1), uniquify(d2), uniquify(p))
    {
    }
    FuncEither(std::unique_ptr<Value> d1, std::unique_ptr<Value> d2, std::unique_ptr<Point> p)
        : dir1(mv(d1))
        , dir2(mv(d2))
        , pnt(mv(p))
    {
        SetParentPtr_helper(this, dir1, dir2, pnt);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<FuncEither>(dir1->clone(), dir2->clone(), pnt->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<FuncEither const&>(other);
        return super::is_equal(other) && (*dir1 == *der_other.dir1) && (*dir2 == *der_other.dir2) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<Value> dir1, dir2;
    std::unique_ptr<Point> pnt;
};

class FuncOpp : public Value {
    using super = Value;

public:
    FuncOpp() = default;
    FuncOpp(Value* d)
        : FuncOpp(uniquify(d))
    {
    }
    FuncOpp(std::unique_ptr<Value> d)
        : dir(mv(d))
    {
        SetParentPtr_helper(this, dir);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<FuncOpp>(dir->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && (*dir == *dynamic_cast<FuncOpp const&>(other).dir);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Value> dir;
};

class FuncStep : public Value {
    using super = Value;

public:
    FuncStep() = default;
    FuncStep(Value* beats, Value* blocksize, Point* p)
        : FuncStep(uniquify(beats), uniquify(blocksize), uniquify(p))
    {
    }
    FuncStep(std::unique_ptr<Value> beats, std::unique_ptr<Value> blocksize, std::unique_ptr<Point> p)
        : numbeats(mv(beats))
        , blksize(mv(blocksize))
        , pnt(mv(p))
    {
        SetParentPtr_helper(this, numbeats, blksize, pnt);
    }
    virtual std::unique_ptr<Value> clone() const override { return std::make_unique<FuncStep>(numbeats->clone(), blksize->clone(), pnt->clone()); }

    virtual float Get(AnimationCompile const& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<FuncStep const&>(other);
        return super::is_equal(other) && (*numbeats == *der_other.numbeats) && (*blksize == *der_other.blksize) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<Value> numbeats, blksize;
    std::unique_ptr<Point> pnt;
};

class Procedure : public Token {
    using super = Token;

public:
    Procedure() = default;
    virtual std::unique_ptr<Procedure> clone() const = 0;

    virtual void Compile(AnimationCompile& anim) = 0;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const = 0;
    virtual bool IsValid() const { return true; }

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;
};

class ProcUnset : public Procedure {
    using super = Procedure;

public:
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcUnset>(); }
    virtual void Compile(AnimationCompile&) override { }
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual bool IsValid() const override { return false; }

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

private:
    static constexpr auto NumParts = 0;
};

class ProcSet : public Procedure {
    using super = Procedure;

public:
    ProcSet() = default;
    ProcSet(ValueVar* vr, Value* v)
        : ProcSet(uniquify(vr), uniquify(v))
    {
    }
    ProcSet(std::unique_ptr<ValueVar> vr, std::unique_ptr<Value> v)
        : var(mv(vr))
        , val(mv(v))
    {
        SetParentPtr_helper(this, var, val);
    }
    virtual std::unique_ptr<Procedure> clone() const override;

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

    struct ReplaceError_NotAVar : std::exception {
    };

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcSet const&>(other);
        return super::is_equal(other) && (*var == *der_other.var) && (*val == *der_other.val);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<ValueVar> var;
    std::unique_ptr<Value> val;
};

class ProcBlam : public Procedure {
    using super = Procedure;

public:
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcBlam>(); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(Token const& other) const override
    {
        return super::is_equal(other) && true;
    }

private:
    static constexpr auto NumParts = 0;
};

class ProcCM : public Procedure {
    using super = Procedure;

public:
    ProcCM() = default;
    ProcCM(Point* p1, Point* p2, Value* steps, Value* d1, Value* d2, Value* beats)
        : ProcCM(uniquify(p1), uniquify(p2), uniquify(steps), uniquify(d1), uniquify(d2), uniquify(beats))
    {
    }
    ProcCM(std::unique_ptr<Point> p1, std::unique_ptr<Point> p2, std::unique_ptr<Value> steps, std::unique_ptr<Value> d1,
        std::unique_ptr<Value> d2, std::unique_ptr<Value> beats)
        : pnt1(mv(p1))
        , pnt2(mv(p2))
        , stps(mv(steps))
        , dir1(mv(d1))
        , dir2(mv(d2))
        , numbeats(mv(beats))
    {
        SetParentPtr_helper(this, pnt1, pnt2, stps, dir1, dir2, numbeats);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcCM>(pnt1->clone(), pnt2->clone(), stps->clone(), dir1->clone(), dir2->clone(), numbeats->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcCM const&>(other);
        return super::is_equal(other) && (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*stps == *der_other.stps) && (*dir1 == *der_other.dir1) && (*dir2 == *der_other.dir2) && (*numbeats == *der_other.numbeats);
    }

private:
    static constexpr auto NumParts = 6;
    std::unique_ptr<Point> pnt1, pnt2;
    std::unique_ptr<Value> stps, dir1, dir2, numbeats;
};

class ProcDMCM : public Procedure {
    using super = Procedure;

public:
    ProcDMCM() = default;
    ProcDMCM(Point* p1, Point* p2, Value* beats)
        : ProcDMCM(uniquify(p1), uniquify(p2), uniquify(beats))
    {
    }
    ProcDMCM(std::unique_ptr<Point> p1, std::unique_ptr<Point> p2, std::unique_ptr<Value> beats)
        : pnt1(mv(p1))
        , pnt2(mv(p2))
        , numbeats(mv(beats))
    {
        SetParentPtr_helper(this, pnt1, pnt2, numbeats);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcDMCM>(pnt1->clone(), pnt2->clone(), numbeats->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcDMCM const&>(other);
        return super::is_equal(other) && (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*numbeats == *der_other.numbeats);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<Point> pnt1, pnt2;
    std::unique_ptr<Value> numbeats;
};

class ProcDMHS : public Procedure {
    using super = Procedure;

public:
    ProcDMHS() = default;
    ProcDMHS(Point* p)
        : ProcDMHS(uniquify(p))
    {
    }
    ProcDMHS(std::unique_ptr<Point> p)
        : pnt(mv(p))
    {
        SetParentPtr_helper(this, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcDMHS>(pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcDMHS const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Point> pnt;
};

class ProcEven : public Procedure {
    using super = Procedure;

public:
    ProcEven() = default;
    ProcEven(Value* steps, Point* p)
        : ProcEven(uniquify(steps), uniquify(p))
    {
    }
    ProcEven(std::unique_ptr<Value> steps, std::unique_ptr<Point> p)
        : stps(mv(steps))
        , pnt(mv(p))
    {
        SetParentPtr_helper(this, stps, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcEven>(stps->clone(), pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcEven const&>(other);
        return super::is_equal(other) && (*stps == *der_other.stps) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Value> stps;
    std::unique_ptr<Point> pnt;
};

class ProcEWNS : public Procedure {
    using super = Procedure;

public:
    ProcEWNS() = default;
    ProcEWNS(Point* p)
        : ProcEWNS(uniquify(p))
    {
    }
    ProcEWNS(std::unique_ptr<Point> p)
        : pnt(mv(p))
    {
        SetParentPtr_helper(this, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcEWNS>(pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcEWNS const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Point> pnt;
};

class ProcFountain : public Procedure {
    using super = Procedure;

public:
    ProcFountain() = default;
    ProcFountain(Value* d1, Value* d2, Value* s1, Value* s2, Point* p)
        : ProcFountain(uniquify(d1), uniquify(d2), uniquify(s1), uniquify(s2), uniquify(p))
    {
    }
    ProcFountain(std::unique_ptr<Value> d1, std::unique_ptr<Value> d2, std::unique_ptr<Value> s1, std::unique_ptr<Value> s2, std::unique_ptr<Point> p)
        : dir1(mv(d1))
        , dir2(mv(d2))
        , stepsize1(mv(s1))
        , stepsize2(mv(s2))
        , pnt(mv(p))
    {
        SetParentPtr_helper(this, dir1, dir2, stepsize1, stepsize2, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcFountain>(dir1->clone(), dir2->clone(), stepsize1 ? stepsize1->clone() : nullptr, stepsize2 ? stepsize2->clone() : nullptr, pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        if (!super::is_equal(other)) {
            return false;
        }

        auto&& der_other = dynamic_cast<ProcFountain const&>(other);
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
    std::unique_ptr<Value> dir1, dir2;
    std::unique_ptr<Value> stepsize1, stepsize2;
    std::unique_ptr<Point> pnt;
};

class ProcFM : public Procedure {
    using super = Procedure;

public:
    ProcFM() = default;
    ProcFM(Value* steps, Value* d)
        : ProcFM(uniquify(steps), uniquify(d))
    {
    }
    ProcFM(std::unique_ptr<Value> steps, std::unique_ptr<Value> d)
        : stps(mv(steps))
        , dir(mv(d))
    {
        SetParentPtr_helper(this, stps, dir);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcFM>(stps->clone(), dir->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcFM const&>(other);
        return super::is_equal(other) && (*stps == *der_other.stps) && (*dir == *der_other.dir);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Value> stps, dir;
};

class ProcFMTO : public Procedure {
    using super = Procedure;

public:
    ProcFMTO() = default;
    ProcFMTO(Point* p)
        : ProcFMTO(uniquify(p))
    {
    }
    ProcFMTO(std::unique_ptr<Point> p)
        : pnt(mv(p))
    {
        SetParentPtr_helper(this, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcFMTO>(pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcFMTO const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Point> pnt;
};

class ProcGrid : public Procedure {
    using super = Procedure;

public:
    ProcGrid() = default;
    ProcGrid(Value* g)
        : ProcGrid(uniquify(g))
    {
    }
    ProcGrid(std::unique_ptr<Value> g)
        : grid(mv(g))
    {
        SetParentPtr_helper(this, grid);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcGrid>(grid->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcGrid const&>(other);
        return super::is_equal(other) && (*grid == *der_other.grid);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Value> grid;
};

class ProcHSCM : public Procedure {
    using super = Procedure;

public:
    ProcHSCM() = default;
    ProcHSCM(Point* p1, Point* p2, Value* beats)
        : ProcHSCM(uniquify(p1), uniquify(p2), uniquify(beats))
    {
    }
    ProcHSCM(std::unique_ptr<Point> p1, std::unique_ptr<Point> p2, std::unique_ptr<Value> beats)
        : pnt1(mv(p1))
        , pnt2(mv(p2))
        , numbeats(mv(beats))
    {
        SetParentPtr_helper(this, pnt1, pnt2, numbeats);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcHSCM>(pnt1->clone(), pnt2->clone(), numbeats->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcHSCM const&>(other);
        return super::is_equal(other) && (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*numbeats == *der_other.numbeats);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<Point> pnt1, pnt2;
    std::unique_ptr<Value> numbeats;
};

class ProcHSDM : public Procedure {
    using super = Procedure;

public:
    ProcHSDM() = default;
    ProcHSDM(Point* p)
        : ProcHSDM(uniquify(p))
    {
    }
    ProcHSDM(std::unique_ptr<Point> p)
        : pnt(mv(p))
    {
        SetParentPtr_helper(this, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcHSDM>(pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcHSDM const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Point> pnt;
};

class ProcMagic : public Procedure {
    using super = Procedure;

public:
    ProcMagic() = default;
    ProcMagic(Point* p)
        : ProcMagic(uniquify(p))
    {
    }
    ProcMagic(std::unique_ptr<Point> p)
        : pnt(mv(p))
    {
        SetParentPtr_helper(this, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcMagic>(pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcMagic const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Point> pnt;
};

class ProcMarch : public Procedure {
    using super = Procedure;

public:
    ProcMarch() = default;
    ProcMarch(Value* stepsize, Value* steps, Value* d, Value* face)
        : ProcMarch(uniquify(stepsize), uniquify(steps), uniquify(d), uniquify(face))
    {
    }
    ProcMarch(std::unique_ptr<Value> stepsize, std::unique_ptr<Value> steps, std::unique_ptr<Value> d,
        std::unique_ptr<Value> face)
        : stpsize(mv(stepsize))
        , stps(mv(steps))
        , dir(mv(d))
        , facedir(mv(face))
    {
        SetParentPtr_helper(this, stpsize, stps, dir, facedir);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcMarch>(stpsize->clone(), stps->clone(), dir->clone(), (facedir) ? facedir->clone() : nullptr); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        if (!super::is_equal(other)) {
            return false;
        }
        auto&& der_other = dynamic_cast<ProcMarch const&>(other);
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
    std::unique_ptr<Value> stpsize, stps, dir, facedir;
};

class ProcMT : public Procedure {
    using super = Procedure;

public:
    ProcMT() = default;
    ProcMT(Value* beats, Value* d)
        : ProcMT(uniquify(beats), uniquify(d))
    {
    }
    ProcMT(std::unique_ptr<Value> beats, std::unique_ptr<Value> d)
        : numbeats(mv(beats))
        , dir(mv(d))
    {
        SetParentPtr_helper(this, numbeats, dir);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcMT>(numbeats->clone(), dir->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcMT const&>(other);
        return super::is_equal(other) && (*numbeats == *der_other.numbeats) && (*dir == *der_other.dir);
    }

private:
    static constexpr auto NumParts = 2;
    std::unique_ptr<Value> numbeats, dir;
};

class ProcMTRM : public Procedure {
    using super = Procedure;

public:
    ProcMTRM() = default;
    ProcMTRM(Value* d)
        : ProcMTRM(uniquify(d))
    {
    }
    ProcMTRM(std::unique_ptr<Value> d)
        : dir(mv(d))
    {
        SetParentPtr_helper(this, dir);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcMTRM>(dir->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcMTRM const&>(other);
        return super::is_equal(other) && (*dir == *der_other.dir);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Value> dir;
};

class ProcNSEW : public Procedure {
    using super = Procedure;

public:
    ProcNSEW() = default;
    ProcNSEW(Point* p)
        : ProcNSEW(uniquify(p))
    {
    }
    ProcNSEW(std::unique_ptr<Point> p)
        : pnt(mv(p))
    {
        SetParentPtr_helper(this, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcNSEW>(pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcNSEW const&>(other);
        return super::is_equal(other) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 1;
    std::unique_ptr<Point> pnt;
};

class ProcRotate : public Procedure {
    using super = Procedure;

public:
    ProcRotate() = default;
    ProcRotate(Value* angle, Value* steps, Point* p)
        : ProcRotate(uniquify(angle), uniquify(steps), uniquify(p))
    {
    }
    ProcRotate(std::unique_ptr<Value> angle, std::unique_ptr<Value> steps, std::unique_ptr<Point> p)
        : ang(mv(angle))
        , stps(mv(steps))
        , pnt(mv(p))
    {
        SetParentPtr_helper(this, ang, stps, pnt);
    }
    virtual std::unique_ptr<Procedure> clone() const override { return std::make_unique<ProcRotate>(ang->clone(), stps->clone(), pnt->clone()); }

    virtual void Compile(AnimationCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual Drawable GetDrawable() const override;
    virtual void replace(Token const* which, std::unique_ptr<Token> v) override;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte> override;
    virtual Reader Deserialize(Reader) override;

protected:
    virtual bool is_equal(Token const& other) const override
    {
        auto&& der_other = dynamic_cast<ProcRotate const&>(other);
        return super::is_equal(other) && (*ang == *der_other.ang) && (*stps == *der_other.stps) && (*pnt == *der_other.pnt);
    }

private:
    static constexpr auto NumParts = 3;
    std::unique_ptr<Value> ang, stps;
    std::unique_ptr<Point> pnt;
};

// this is the top level Deserialization function
std::tuple<std::unique_ptr<Procedure>, Reader> DeserializeProcedure(Reader);

// helper for setting the parent pointer
template <typename P>
void SetParentPtr_helper(P)
{
}

template <typename P, typename T>
void SetParentPtr_helper(P parent, T& t)
{
    if (t) {
        t->SetParentPtr(parent);
    }
}

template <typename P, typename T, typename... Ts>
void SetParentPtr_helper(P parent, T& t, Ts&... ts)
{
    SetParentPtr_helper(parent, t);
    SetParentPtr_helper(parent, ts...);
}

// helper for some dynamic casting
template <typename Derived, typename Base>
std::unique_ptr<Derived>
dynamic_unique_ptr_cast(std::unique_ptr<Base>&& p)
{
    if (Derived* result = dynamic_cast<Derived*>(p.get())) {
        p.release();
        return std::unique_ptr<Derived>(result);
    }
    return std::unique_ptr<Derived>(nullptr);
}

// helper function that examines each pointer to see if it shuold be replaced, and then replace it.
// making sure to clean things up a the end.
template <typename R, typename UP, typename T>
void replace_helper2(R replace, UP& new_value, T& t)
{
    if (t.get() == replace) {
        using Derived = typename T::element_type;
        auto result = dynamic_cast<Derived*>(new_value.get());
        if (!result) {
            throw std::runtime_error("Invalid value in replace");
        }
        auto&& casted = dynamic_unique_ptr_cast<Derived>(std::move(new_value));
        if (!casted) {
            throw std::runtime_error("Invalid value in replace");
        }
        t = std::move(casted);
    }
}

template <typename R, typename UP, typename T, typename... Ts>
void replace_helper2(R replace, UP& new_value, T& t, Ts&... ts)
{
    replace_helper2(replace, new_value, t);
    replace_helper2(replace, new_value, ts...);
}

template <int N, typename P, typename R, typename UP, typename T, typename... Ts>
void replace_helper(P parent, R replace, UP& new_value, T& t, Ts&... ts)
{
    static_assert(N == (sizeof...(Ts) + 1));
    replace_helper2(replace, new_value, t, ts...);
    SetParentPtr_helper(parent, t, ts...);
}

}
