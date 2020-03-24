/*
cartotype_arithmetic.h
Copyright (C) 2004-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_ARITHMETIC_H__
#define CARTOTYPE_ARITHMETIC_H__

#include <cartotype_base.h>
#include <cartotype_errors.h>

#include <math.h>

namespace CartoType
{

// forward declarations
class TFixedSmall;

/** The intersection place of a line segment on another. */
enum class TIntersectionPlace
    {
    /** Lines are parallel or coincident. */
    None = 0,
    /** The intersection is before the start of the segment. */
    Before,
    /** The intersection is on the segment. */
    On,
    /** The intersection is after the segment. */
    After
    };

/** The type of intersection of two line segments. */
class TIntersectionType
    {
    public:
    /** Returns true if the lines, extended to infinity, do not intersect: that is, they are coincident or parallel. */
    bool None() const { return iFirstSegment == TIntersectionPlace::None && iSecondSegment == TIntersectionPlace::None; }
    /** Returns true if the line segments intersect each other within their lengths. */
    bool Both() const { return iFirstSegment == TIntersectionPlace::On && iSecondSegment == TIntersectionPlace::On; }

    /** The intersection place of the first line segment. */
    TIntersectionPlace iFirstSegment = TIntersectionPlace::None;
    /** The intersection place of the second line segment. */
    TIntersectionPlace iSecondSegment = TIntersectionPlace::None;
    };

/**
Rounds a floating-point value to the nearest integer.
Does not use floor() because it is said to be slow on some platforms.
*/
inline int32_t Round(double aValue)
    {
    return int32_t(aValue < 0.0 ? aValue - 0.5 : aValue + 0.5);
    }

/**
A non-locale-dependent version of the standard library strtod function.

Converts a string to a floating-point number and optionally returns a pointer to the position
after the portion of the string converted. The decimal point is always a full stop,
unlike the standard library function, which uses that of the current locale (e.g., a comma
in the German locale.)

If aLength is UINT32_MAX the string must be null-terminated.
*/
double Strtod(const char* aString,size_t aLength = UINT32_MAX,const char** aEndPtr = nullptr) noexcept;

/**
A non-locale-dependent version of the standard library strtod function, but
for UTF-16 characters.

Converts a string to a floating-point number and optionally returns a pointer to the position
after the portion of the string converted. The decimal point is always a full stop,
unlike the standard library function, which uses that of the current locale (e.g., a comma
in the German locale.)

If aLength is UINT32_MAX the string must be null-terminated.
*/
double Strtod(const uint16_t* aString,size_t aLength = UINT32_MAX,const uint16** aEndPtr = nullptr) noexcept;

/** A constant for pi/2 in 3.29 fixed-point format.*/
constexpr int32 KHalfPi3_29 = 843314856UL;
/** A constant for pi in 3.29 fixed-point format.*/
constexpr int32 KPi3_29 = 1686629713UL;

/**
A fixed-point number consisting of a 16-bit integer plus 16 fractional bits.
This type and the routines used are based on FreeType's FT_FIXED
type and the FT_MulFix and FT_DivFix functions.
*/
class TFixed
    {
    public:
    /** Constructs a TFixed with the value zero. */
    TFixed(): iValue(0) { }
    /** Constructs a TFixed from an integer. */
    TFixed(int aValue): iValue(aValue << 16) { }
    /** Constructs a TFixed from a value with 0...16 fractional bits. */
    TFixed(int32_t aValue,int32_t aFractionalBits)
        { assert(aFractionalBits >= 0 && aFractionalBits <= 16); iValue = aValue << (16 - aFractionalBits); }
    /** Constructs a TFixed from a double-precision floating-point number.*/
    TFixed(double aValue) { iValue = Round(aValue * 65536.0); }
    /** A dummy type allowing construction of a TFixed from a raw value. */
    enum class TRaw { Value };
    /** Constructs a TFixed from a raw value. */
    TFixed(int32_t aValue,TRaw): iValue(aValue) { }
    /** A dummy type allowing construction of a TFixed from 64ths. */
    enum class T64ths { Value };
    /** Constructs a TFixed from 64ths. */
    TFixed(int32_t aValue,T64ths): iValue(aValue << 10) { }
    /** Constructs a TFixed from a TFixedSmall. */
    inline TFixed(const TFixedSmall& aFixedSmall);
    /** Returns the value in 65536ths. */
    int32_t RawValue() const { return iValue; }
    /** Returns the value as a double-precision floating-point number. */
    double FpValue() const { return (double)iValue / 65536.0; }
    /** Sets the value in 65536ths. */
    void SetRawValue(int32_t aRawValue) { iValue = aRawValue; }
    /** The equality operator. */
    bool operator==(TFixed aFixed) const { return iValue == aFixed.iValue; }
    /** The inequality operator. */
    bool operator!=(TFixed aFixed) const { return iValue != aFixed.iValue; }
    /** The less-than operator. */
    bool operator<(TFixed aFixed) const { return iValue < aFixed.iValue; }
    /** The less-than-or-equal operator. */
    bool operator<=(TFixed aFixed) const { return iValue <= aFixed.iValue; }
    /** The greater-than operator. */
    bool operator>(TFixed aFixed) const { return iValue > aFixed.iValue; }
    /** The greater-than-or-equal operator. */
    bool operator>=(TFixed aFixed) const { return iValue >= aFixed.iValue; }
    /** Returns the value rounded to the nearest unit. */
    int32_t Rounded() const { return (iValue + 32768) >> 16; }
    /** Returns the nearest integer value at or below the actual value. */
    int32_t Floor() const { return iValue >> 16; }
    /** Returns the nearest integer value at or above the actual value. */
    int32_t Ceiling() const { return (iValue + 65535) >> 16; }
    /** Returns the value rounded to the nearest 64th. */
    int32_t Rounded64ths() const { return (iValue + 512) >> 10; }
    /** Adds a fixed-point value. */
    TFixed operator+(TFixed aFixed) const { TFixed f = *this; f.iValue += aFixed.iValue; return f; }
    /** Increments by a fixed-point value. */
    void operator+=(TFixed aFixed) { iValue += aFixed.iValue; }
    /** Subtracts a fixed-point value. */
    TFixed operator-(TFixed aFixed) const { TFixed f = *this; f.iValue -= aFixed.iValue; return f; }
    /** Decrements by a fixed-point value. */
    void operator-=(TFixed aFixed) { iValue -= aFixed.iValue; }
    /** An assignment operator to multiply two fixed-point values. */
    void operator*=(TFixed aB)
        {
        if (iValue == 0 || aB.iValue == 0x10000)
            return;
        
        // CartoType testing showed that aB was zero in about 8% of cases.
        if (aB.iValue == 0)
            {
            iValue = 0;
            return;
            }

        int s = 1;
        if (iValue < 0)
            {
            iValue = -iValue;
            s = -1;
            }
        if (aB.iValue < 0)
            {
            aB.iValue = -aB.iValue;
            s = -s;
            }
        if (s > 0)
            iValue = int32_t((int64_t(iValue) * int64_t(aB.iValue) + 0x8000) >> 16);
        else
            iValue = int32_t(-((int64_t(iValue) * int64_t(aB.iValue) + 0x8000) >> 16));
        }
    /** Multiplies by a fixed-point value. */
    TFixed operator*(TFixed aFixed) const { TFixed f = *this; f *= aFixed; return f; }
    /** An assignment operator to multiply by an integer value. */
    void operator*=(int32_t aInt) { iValue *= aInt; }
    /** Multiplies by an integer. */
    TFixed operator*(int32_t aInt) const { TFixed f = *this; f.iValue *= aInt; return f; }
    /** An assignment operator to divide one fixed-point value by another. */
    void operator/=(TFixed aB)
        {
        if (iValue == 0 || aB.iValue == 0x10000)
            return;

        int s = 1;
        if (iValue < 0)
            {
            iValue = -iValue;
            s = -1;
            }
        if (aB.iValue < 0)
            {
            aB.iValue = -aB.iValue;
            s = -s;
            }

        uint32_t q;
        if (aB.iValue == 0)
            /* check for division by 0 */
            q = 0x7FFFFFFFL;
        else
            /* compute result directly */
            q = (uint32_t)((((int64_t)iValue << 16) + (aB.iValue >> 1)) / aB.iValue);

        iValue = s < 0 ? -(int32_t)q : (int32_t)q;
        }

    /** Divides by a fixed-point value. */
    TFixed operator/(TFixed aFixed) const { TFixed f = *this; f /= aFixed; return f; }
    /** An assignment operator to divide by an integer. */
    void operator/=(int32_t aInt) { iValue /= aInt; }
    /** Divides by an integer. */
    TFixed operator/(int32_t aInt) const { TFixed f = *this; f.iValue /= aInt; return f; }
    /** The unary negation operator. */
    TFixed operator-() const { TFixed f; f.iValue = -iValue; return f; }
    /**
    Returns the square root of a fixed-point value.
    Returns 0 if the argument is less than or equal to zero.
    */
    TFixed Sqrt() const;
    /** Interprets a value as degrees and converts it to radians. */
    TFixedSmall ToRadians() const;
    /**
    Returns the difference between two angles.
    The result, expressed in radians, is the angular distance swept out when moving the
    angle to aAngle by the shortest route, and thus is in the range -pi ... pi.
    */
    TFixed AngleDiff(TFixed aAngle) const;
    /** Returns the constant pi as a TFixed value. */
    static TFixed Pi() { return TFixed(KPi3_29 >> 13,TRaw::Value); }
    /** Returns the constant pi/2 as a TFixed value. */
    static TFixed HalfPi() { return TFixed(KHalfPi3_29 >> 13,TRaw::Value); }
    /** Returns the integer part, rounding down. The integer part of 3.6 is 3; the integer part of -0.2 is -1. */
    int32_t IntegerPart() const { return iValue >> 16; }
    /** Returns the fractional part, rounding down. The fractional part of 3.6 is 0.6; the fractional part of -0.2 is 0.8. */
    TFixed FractionalPart() const { TFixed f = *this; f.iValue &= 0x0000FFFF; return f; }
    /** Returns the absolute value of the fixed number. */
    TFixed Abs() const
        {
        if (iValue >= 0)
            return *this;
        return -*this;
        }
    /** Returns true if the number is zero. */
    bool IsZero() const { return iValue == 0; }
    /** Returns true if the number is non-zero. */
    bool NonZero() const { return iValue != 0; }

    private:
    int32_t iValue;
    };

/** A point class containing two fixed-point numbers. */
class TPointFixed
    {
    public:
    /** Creates a TPointFixed with coordinates (0,0). */
    TPointFixed() { }
    /** Creates a TPointFixed from fixed-point X and Y coordinates. */
    TPointFixed(TFixed aX,TFixed aY): iX(aX), iY(aY) { }
    /** Creates a TPointFixed from integer X and Y coordinates. */
    TPointFixed(int32_t aX,int32_t aY): iX(aX), iY(aY) { }
    /** Creates a TPointFixed from an integer point. */
    TPointFixed(const TPoint& aPoint): iX(aPoint.iX), iY(aPoint.iY) { }
    /** Creates a TPointFixed from a point with 0...16 fractional bits. */
    TPointFixed(const TPoint& aPoint,int32_t aFractionalBits): iX(aPoint.iX,aFractionalBits), iY(aPoint.iY,aFractionalBits) { }
    /** Creates a TPointFixed from a point containing raw values (with 16 fractional bits). */
    TPointFixed(const TPoint& aPoint,TFixed::TRaw): iX(aPoint.iX,TFixed::TRaw::Value), iY(aPoint.iY,TFixed::TRaw::Value) { }
    /** The equality operator. */
    bool operator==(const TPointFixed& aPoint) const { return iX == aPoint.iX && iY == aPoint.iY; }
    /** The inequality operator. */
    bool operator!=(const TPointFixed& aPoint) const { return !(*this == aPoint); }
    /** Offsets a point by another point, treated as a positive vector. */
    void operator+=(const TPointFixed& aPoint) { iX += aPoint.iX; iY += aPoint.iY; }
    /** Offsets a point by another point, treated as a negative vector. */
    void operator-=(const TPointFixed& aPoint) { iX -= aPoint.iX; iY -= aPoint.iY; }
    /** Multiplies the vector represented by the point. */
    void operator*=(const TFixed& aFixed) { iX *= aFixed; iY *= aFixed; }
    /** Returns the length of the vector represented by the point. */
    TFixed VectorLength() const
        {
        if (iX.RawValue() == 0)
            return iY.RawValue() >= 0 ? iY : -iY;
        if (iY.RawValue() == 0)
            return iX.RawValue() >= 0 ? iX : -iX;
        return VectorLengthHelper();
        }
    /** Returns an integer point in rounded 64ths of the values in the TPointFixed object. */
    TPoint Rounded64ths() const
        {
        int32_t x = iX.Rounded64ths();
        int32_t y = iY.Rounded64ths();
        return TPoint(x,y);
        }
    /** Returns an integer point, rounding the values in the TPointFixed object to units. */
    TPoint Rounded() const
        {
        int32_t x = iX.Rounded();
        int32_t y = iY.Rounded();
        return TPoint(x,y);
        }
    /**
    Converts a point to polar coordinates.
    The radius is stored in iX and the angle in radians in iY.
    */
    void Polarize();
    /** Rotates a point treated as a vector by an angle in radians. */
    void Rotate(TFixedSmall aAngle);
    /** Returns the arc-tangent of this point: that is, the angle in radians of a line from the origin to this point. */
    TFixedSmall Atan2() const;
    
    /** The x coordinate. */
    TFixed iX;
    /** The y coordinate. */
    TFixed iY;

    private:
    TFixed VectorLengthHelper() const;
    };

/** A straight line or line segment defined using fixed-point numbers. */
class TLineFixed
    {
    public:
    TLineFixed() { }
    /** Creates a line from aStart to aEnd. */
    TLineFixed(const TPointFixed& aStart,const TPointFixed& aEnd):
        iStart(aStart),
        iEnd(aEnd),
        iLength(-1,TFixed::TRaw::Value)
        {
        }
    /** Returns the length of the line segment. */
    TFixed Length() const
        {
        if (iLength.RawValue() < 0)
            {
            TPointFixed p(iEnd); p -= iStart; iLength = p.VectorLength();
            }
        return iLength;
        }
    /** Returns the start of the line segment. */
    const TPointFixed& Start() const { return iStart; }
    /** Returns the end of the line segment. */
    const TPointFixed& End() const { return iEnd; }
    /**
    Returns true if the distance from the line segment of either of the one or two supplied points
    exceeds aMaxDistance. This function is used when flattening curved paths, for finding
    whether the path is flat enough, which is when all the control points are
    near enough to the line segments.
    */
    bool DistanceExceeds(double aX1,double aY1,double aX2,double aY2,int32_t aPoints,double aMaxDistance) const;
    /**
    Returns a point at a certain distance along a line in aPoint.
    aDistance can be negative, or beyond the end of the line.
    In these cases the line is extended in the same direction.
    */
    void GetTangent(TFixed aDistance,TPointFixed& aPoint) const;
    /** Returns a reversed copy of this line. */
    TLineFixed Reverse() const { TLineFixed l; l.iStart = iEnd; l.iEnd = iStart; l.iLength = iLength; return l; }

    private:
    TPointFixed iStart;
    TPointFixed iEnd;
    mutable TFixed iLength;
    };

/**
An arctangent function which checks for two zero arguments and returns zero in that case.
In the standard library atan2(0,0) is undefined, and on Embarcadero C++ Builder it throws an exception.
*/
inline double Atan2(double aY,double aX)
    {
    if (aY == 0 && aX == 0)
        return 0;
    return atan2(aY,aX);
    }

} // namespace CartoType

#endif
