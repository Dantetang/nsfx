/**
 * @file
 *
 * @brief Chrono support for Network Simulation Frameworks.
 *
 * @version 1.2
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2015-02-16
 *
 * @copyright Copyright (c) 2015.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#ifndef DURATION_H__E90A15BC_96A8_4FA8_99A1_E3DC5B5A2A66
#define DURATION_H__E90A15BC_96A8_4FA8_99A1_E3DC5B5A2A66


#include <nsfx/chrono/config.h>
#include <nsfx/utility/ratio.h>
#include <nsfx/utility/rounding.h>
#include <boost/functional/hash.hpp>
#include <boost/core/swap.hpp>
#include <boost/ratio.hpp>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath> // ceil, floor
#include <boost/math/special_functions/trunc.hpp>
#include <boost/math/special_functions/round.hpp>
#include <type_traits> // true_type, false_type, is_same, conditional


NSFX_CHRONO_OPEN_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
namespace aux { /*{{{*/


////////////////////////////////////////
// Tags.
struct OverflowTag  {};
struct UnderflowTag {};


////////////////////////////////////////
// Precondition: Unit is a ratio type.
template<class Unit, intmax_t factor,
         bool unitGtOne = ratio_greater<Unit, ratio<1>>::value>
struct MakeSmallerUnitCheckSize
{
    typedef typename ratio_multiply<Unit, ratio<1, factor>>::type  type;
};

// Precondition: Unit is a ratio type.
template<class Unit, intmax_t factor>
struct MakeSmallerUnitCheckSize<Unit, factor, false>
{
    typedef UnderflowTag  type;
};

// Unit is UnderflowTag.
template<class Unit, intmax_t factor,
         bool underflow = std::is_same<Unit, UnderflowTag>::value>
struct MakeSmallerUnit
{
    static_assert(factor >= 1, "The factor must be no less than 1.");
    typedef UnderflowTag  type;
};

// Unit is a ratio type.
template<class Unit, intmax_t factor>
struct MakeSmallerUnit<Unit, factor, false>
{
    static_assert(factor >= 1, "The factor must be no less than 1.");
    typedef typename MakeSmallerUnitCheckSize<Unit, factor>::type  type;
};


////////////////////////////////////////
// Check overflow.
template<intmax_t A, intmax_t B>
struct SafeMultiply
{
    static const bool value = (A <= (INT64_MAX / B));
};

template<class Res1, class Res2>
struct SafeRatioMultiply
{
    static const bool value =
        SafeMultiply<Res1::num, Res2::num>::value &&
        SafeMultiply<Res1::den, Res2::den>::value;
};

// Precondition: Unit is a ratio type.
template<class Unit, intmax_t factor,
         bool safe = SafeRatioMultiply<Unit, ratio<factor, 1>>::value>
struct MakeBiggerUnitCheckMultiply
{
    typedef typename ratio_multiply<Unit, ratio<factor, 1>>::type  type;
};

// Precondition: Unit is a ratio type.
template<class Unit, intmax_t factor>
struct MakeBiggerUnitCheckMultiply<Unit, factor, false>
{
    typedef OverflowTag  type;
};

// Unit is OverflowTag.
template<class Unit, intmax_t factor,
         bool overflow = std::is_same<Unit, OverflowTag>::value>
struct MakeBiggerUnit
{
    static_assert(factor >= 1, "The factor must be no less than 1.");
    typedef OverflowTag  type;
};

// Unit is a ratio type.
template<class Unit, intmax_t factor>
struct MakeBiggerUnit<Unit, factor, false>
{
    static_assert(factor >= 1, "The factor must be no less than 1.");
    typedef typename MakeBiggerUnitCheckMultiply<Unit, factor>::type  type;
};


////////////////////////////////////////
// Ratio divide.
/**
 * @brief Ratio divide.
 *
 * @param[out]    x  The quotient.
 * @param[in,out] u  Must be non-negative.
 * @param[in,out] v  Must be positive.
 *
 * Calculates the following equation:
 * @code
 * y1 = r2 * x2 + y2,
 * where, y1 = u1 / v1, u1 and v1 are known,
 *        r2 = N2 / D2,
 *        x2 is a non-negative integer,
 *        y2 = u2 / v2,
 *        0 <= y2 < r2.
 *
 * x2 is the quotient: the number of r2 that y1 has.
 * y2 is the remainder: the remaining number of fundamental periods (FPs)
 *                      that shall be expressed in a smaller unit.
 * @endcode
 *
 * The solution is:
 * @code
 * x2 = (D2 * u1) / (N2 * v1),  (round-down)
 * u2 = (D2 * u1) - (N2 * v1) * x2,
 * v2 =  D2 * v1.
 *
 * The calculation will never overflow under the design of Duration.
 * For the smallest unit (U0) that is not UnderFlow, N0 = 1 and D0 <= 100.
 * For all other bigger units, D = 1 and N > 1.
 * The smallest unit is no larger than Second.
 * The calculation starts from u := count_, and v := 1, and u/v is divided
 * by Day, ..., Second, ..., U1 (the unit that is just bigger than U0),
 * with their D = 1 and N > 1.
 * During the process, v is always 1.
 * After being divided by U1, u/v < U1 <= 1000 / D0.
 * When u/v is divided by U0, it will not overflow.
 * @endcode
 */
template<intmax_t N, intmax_t D>
inline void RatioDivide(count_t& x, count_t& u, count_t& v, ratio<N, D>) BOOST_NOEXCEPT
{
    BOOST_ASSERT(u >= 0);
    BOOST_ASSERT(v >  0);
    x = (D * u) / (N * v);
    u = (D * u) - (N * v) * x;
    v =  D * v;
}

inline void RatioDivide(count_t& x, count_t& u, count_t& v, OverflowTag) BOOST_NOEXCEPT
{
    x = 0;
}

inline void RatioDivide(count_t& x, count_t& u, count_t& v, UnderflowTag) BOOST_NOEXCEPT
{
    x = 0;
}


////////////////////////////////////////
// Conversion.
struct PrecisionUpTag   {};
struct PrecisionDownTag {};

template<class Rs, class Rd>
inline count_t
ConvertCount(const count_t& count, PrecisionUpTag) BOOST_NOEXCEPT
{
    // Source resolution is wider.
    typedef typename ratio_divide<Rs, Rd>::type  Rx;
    BOOST_ASSERT_MSG(count <=  INT64_MAX / Rx::num &&
                     count >=  INT64_MIN / Rx::num,
                     "Cannot convert the count, "
                     "since the conversion overflows.");
    return count * Rx::num;
}

template<class Rs, class Rd>
inline count_t
ConvertCount(const count_t& count, PrecisionDownTag) BOOST_NOEXCEPT
{
    // Destination resolution is wider.
    typedef typename ratio_divide<Rd, Rs>::type  Rx;
    // No overflow, truncates fractional part.
    return count / Rx::num;
}

template<class Rs, class Rd>
inline count_t
ConvertCount(const count_t& count) BOOST_NOEXCEPT
{
    typedef typename std::conditional<
        ratio_greater<Rs, Rd>::value,
        PrecisionUpTag, PrecisionDownTag>::type  P;
    return ConvertCount<Rs, Rd>(count, P());
}


////////////////////////////////////////
// Comparison.
struct EquallyFineTag  {};
struct LeftIsFinerTag  {};
struct RightIsFinerTag {};

template<class Res1, class Res2>
struct WhichIsFiner
{
    typedef typename std::conditional<
        ratio_equal<Res1, Res2>::value,
        EquallyFineTag,
        typename std::conditional<
            ratio_less<Res1, Res2>::value,
            LeftIsFinerTag, RightIsFinerTag>::type
        >::type  type;
};


template<class Res1, class Res2>
bool Equal(const count_t& lhs, const count_t& rhs, EquallyFineTag) BOOST_NOEXCEPT;

template<class Res1, class Res2>
bool Equal(const count_t& lhs, const count_t& rhs, LeftIsFinerTag) BOOST_NOEXCEPT;

template<class Res1, class Res2>
bool Equal(const count_t& lhs, const count_t& rhs, RightIsFinerTag) BOOST_NOEXCEPT;

template<class Res1, class Res2>
bool Less(const count_t& lhs, const count_t& rhs, EquallyFineTag) BOOST_NOEXCEPT;

template<class Res1, class Res2>
bool Less(const count_t& lhs, const count_t& rhs, LeftIsFinerTag) BOOST_NOEXCEPT;

template<class Res1, class Res2>
bool Less(const count_t& lhs, const count_t& rhs, RightIsFinerTag) BOOST_NOEXCEPT;

template<class Res1, class Res2>
bool LessEqual(const count_t& lhs, const count_t& rhs, EquallyFineTag) BOOST_NOEXCEPT;

template<class Res1, class Res2>
bool LessEqual(const count_t& lhs, const count_t& rhs, LeftIsFinerTag) BOOST_NOEXCEPT;

template<class Res1, class Res2>
bool LessEqual(const count_t& lhs, const count_t& rhs, RightIsFinerTag) BOOST_NOEXCEPT;


////////////////////////////////////////
template<class Res1, class Res2>
inline bool
Equal(const count_t& lhs, const count_t& rhs, EquallyFineTag) BOOST_NOEXCEPT
{
    return lhs == rhs;
}

template<class Res1, class Res2>
inline bool
Equal(const count_t& lhs, const count_t& rhs, LeftIsFinerTag) BOOST_NOEXCEPT
{
    typedef typename ratio_divide<Res2, Res1>::type  Rx;
    if (rhs > INT64_MAX / Rx::num)
    {
        return false;
    }
    else if (rhs < INT64_MIN / Rx::num)
    {
        return false;
    }
    else
    {
        return lhs == rhs * Rx::num;
    }
}

template<class Res1, class Res2>
inline bool
Equal(const count_t& lhs, const count_t& rhs, RightIsFinerTag) BOOST_NOEXCEPT
{
    return Equal<Res2, Res1>(rhs, lhs, LeftIsFinerTag());
}

template<class Res1, class Res2>
inline bool
Less(const count_t& lhs, const count_t& rhs, EquallyFineTag) BOOST_NOEXCEPT
{
    return lhs < rhs;
}

template<class Res1, class Res2>
inline bool
Less(const count_t& lhs, const count_t& rhs, LeftIsFinerTag) BOOST_NOEXCEPT
{
    typedef typename ratio_divide<Res2, Res1>::type  Rx;
    if (rhs > INT64_MAX / Rx::num)
    {
        return false;
    }
    else if (rhs < INT64_MIN / Rx::num)
    {
        return true;
    }
    else
    {
        return lhs < rhs * Rx::num;
    }
}

template<class Res1, class Res2>
inline bool
Less(const count_t& lhs, const count_t& rhs, RightIsFinerTag) BOOST_NOEXCEPT
{
    return !LessEqual<Res2, Res1>(rhs, lhs, LeftIsFinerTag());
}

template<class Res1, class Res2>
inline bool
LessEqual(const count_t& lhs, const count_t& rhs, EquallyFineTag) BOOST_NOEXCEPT
{
    return lhs <= rhs;
}

template<class Res1, class Res2>
inline bool
LessEqual(const count_t& lhs, const count_t& rhs, LeftIsFinerTag) BOOST_NOEXCEPT
{
    typedef typename ratio_divide<Res2, Res1>::type  Rx;
    if (rhs > INT64_MAX / Rx::num)
    {
        return false;
    }
    else if (rhs < INT64_MIN / Rx::num)
    {
        return true;
    }
    else
    {
        return lhs <= rhs * Rx::num;
    }
}

template<class Res1, class Res2>
inline bool
LessEqual(const count_t& lhs, const count_t& rhs, RightIsFinerTag) BOOST_NOEXCEPT
{
    return !Less<Res2, Res1>(rhs, lhs, LeftIsFinerTag());
}


////////////////////////////////////////
// Common type.
template<class Res1, class Res2>
struct Common
{
    typedef typename std::conditional<
        ratio_less_equal<Res1, Res2>::value,
        Res1, Res2>::type  type;
};


} // namespace aux /*}}}*/


////////////////////////////////////////////////////////////////////////////////
/**
 * @ingroup Chrono
 * @brief Time duration.
 *
 * @tparam Res  The time resolution in seconds.
 *              * It must be a `ratio` template.
 *              * It must be a non-positive power of `10`.
 *
 * A time duration is represented by the number of fundamental periods.
 * The number is a signed integer value of type `count_t`, which is an alias
 * for `int64_t`.
 * `Res` can be as small as `atto`.
 *
 * e.g., if `Res` is `nano`, the time resolution is `1` nanosecond, the
 * Duration can represent about -106,751 to +106,751 days, or -292 to 292 years.
 *
 * e.g., if `Res` is `atto`, the time resolution is `1` attosecond, the
 * Duration can represent about -9.223 to +9.223 seconds.
 */
template<class Res>
class Duration
{
    // Typedefs./*{{{*/
private:
    static_assert(Res::num == 1 &&
                  (Res::den == 1 || Res::den % 10 == 0),
                  "The time resolution must be a non-positive power of 10.");

public:
    typedef Res  Resolution;

private:
    template<class Res2>
    friend class Duration;

private:
    // The number of fundamental periods in a second.
    typedef typename ratio_divide<ratio<1>, Res>::type  Second;

    // If a unit type is UnderFlowTag, it is smaller than a fundamental period,
    // and it cannot be expressed by this Duration type.
    // If not underflow, the denominator is at most 100.
    // For all other larger units, the denominator is 1.
    typedef typename aux::MakeSmallerUnit<Second,      1000>::type  MilliSecond;
    typedef typename aux::MakeSmallerUnit<MilliSecond, 1000>::type  MicroSecond;
    typedef typename aux::MakeSmallerUnit<MicroSecond, 1000>::type  NanoSecond;
    typedef typename aux::MakeSmallerUnit<NanoSecond,  1000>::type  PicoSecond;
    typedef typename aux::MakeSmallerUnit<PicoSecond,  1000>::type  FemtoSecond;
    typedef typename aux::MakeSmallerUnit<FemtoSecond, 1000>::type  AttoSecond;
    typedef typename aux::MakeSmallerUnit<AttoSecond,  1000>::type  ZeptoSecond;
    typedef typename aux::MakeSmallerUnit<ZeptoSecond, 1000>::type  YoctoSecond;

    typedef typename aux::MakeBiggerUnit<Second, 60>::type  Minute;
    typedef typename aux::MakeBiggerUnit<Minute, 60>::type  Hour;
    typedef typename aux::MakeBiggerUnit<Hour,   24>::type  Day;

    /*}}}*/

    // Constructors./*{{{*/
public:
    /**
     * @brief Construct a default duration that has `0` seconds.
     */
    Duration(void) BOOST_NOEXCEPT :
        count_(0)
    {}

    /**
     * @brief Construct a duration.
     *
     * @param[in] n  The number of fundamental periods.
     */
    explicit Duration(count_t n) BOOST_NOEXCEPT :
        count_(n)
    {}

    /**
     * @brief Construct a duration.
     *
     * @param[in] sec The time length in seconds.
     */
    Duration(double sec, round_to_zero_t) BOOST_NOEXCEPT :
        count_(boost::math::lltrunc(sec * GetFrequency()))
    {}

    /**
     * @brief Construct a duration.
     *
     * @param[in] sec The time length in seconds.
     */
    Duration(double sec, round_to_nearest_t) BOOST_NOEXCEPT :
        count_(boost::math::llround(sec * GetFrequency()))
    {}

    /**
     * @brief Construct a duration.
     *
     * @param[in] sec The time length in seconds.
     */
    Duration(double sec, round_upward_t) BOOST_NOEXCEPT :
        count_(static_cast<count_t>(std::ceil(sec * GetFrequency())))
    {}

    /**
     * @brief Construct a duration.
     *
     * @param[in] sec The time length in seconds.
     */
    Duration(double sec, round_downward_t) BOOST_NOEXCEPT :
        count_(static_cast<count_t>(std::floor(sec * GetFrequency())))
    {}

    /*}}}*/

    // Copyable./*{{{*/
public:
    Duration(const Duration& rhs) BOOST_NOEXCEPT :
        count_(rhs.count_)
    {}

    Duration& operator=(const Duration& rhs) BOOST_NOEXCEPT
    {
        if (this != &rhs)
        {
            count_ = rhs.count_;
        }
        return *this;
    }

    /*}}}*/

    // Convertible./*{{{*/
public:
    /**
     * @brief Convert from a duration with a different resolution.
     *
     * If the source resolution is coarser, then the conversion is precise.
     *
     * If the source resolution is finer, then the conversion **truncates**
     * the count toward zero, i.e., discards the fractional part of the count.
     */
    template<class Res2>
    Duration(const Duration<Res2>& rhs) BOOST_NOEXCEPT :
        count_(aux::ConvertCount<Res2, Res>(rhs.count_))
    {}

    /**
     * @brief Convert from a duration with a different resolution.
     *
     * If the source resolution is coarser, then the conversion is precise.
     *
     * If the source resolution is finer, then the conversion **truncates**
     * the count toward zero, i.e., discards the fractional part of the count.
     */
    template<class Res2>
    Duration& operator=(const Duration<Res2>& rhs) BOOST_NOEXCEPT
    {
        count_ = aux::ConvertCount<Res2, Res>(rhs.count_);
        return *this;
    }

    /*}}}*/

    // Comparison./*{{{*/
public:
    template<class Res1, class Res2>
    friend bool
    operator< (const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res1, class Res2>
    friend bool
    operator<=(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res1, class Res2>
    friend bool
    operator==(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res1, class Res2>
    friend bool
    operator!=(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res1, class Res2>
    friend bool
    operator> (const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res1, class Res2>
    friend bool
    operator>=(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    /*}}}*/

    // Arithmetic./*{{{*/
public:
    template<class Res1, class Res2>
    friend Duration<typename aux::Common<Res1, Res2>::type>
    operator+(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res1, class Res2>
    friend Duration<typename aux::Common<Res1, Res2>::type>
    operator-(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res2>
    friend Duration<Res2>
    operator*(const Duration<Res2>& lhs, count_t n) BOOST_NOEXCEPT;

    template<class Res2>
    friend Duration<Res2>
    operator*(count_t n, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res1, class Res2>
    friend count_t
    operator/(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    template<class Res2>
    friend Duration<Res2>
    operator/(const Duration<Res2>& lhs, count_t n) BOOST_NOEXCEPT;

    template<class Res1, class Res2>
    friend Duration<typename aux::Common<Res1, Res2>::type>
    operator%(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT;

    Duration& operator++(void) BOOST_NOEXCEPT
    {
        ++count_;
        return *this;
    }

    Duration operator++(int) BOOST_NOEXCEPT
    {
        return Duration(count_++);
    }

    Duration& operator--(void) BOOST_NOEXCEPT
    {
        --count_;
        return *this;
    }

    Duration operator--(int) BOOST_NOEXCEPT
    {
        return Duration(count_--);
    }

    Duration& operator+=(const Duration& rhs) BOOST_NOEXCEPT
    {
        count_ += rhs.count_;
        return *this;
    }

    Duration& operator-=(const Duration& rhs) BOOST_NOEXCEPT
    {
        count_ -= rhs.count_;
        return *this;
    }

    template<class Res2>
    Duration& operator+=(const Duration<Res2>& rhs) BOOST_NOEXCEPT
    {
        count_ += aux::ConvertCount<Res2, Res>(rhs.count_);
        return *this;
    }

    template<class Res2>
    Duration& operator-=(const Duration<Res2>& rhs) BOOST_NOEXCEPT
    {
        count_ -= aux::ConvertCount<Res2, Res>(rhs.count_);
        return *this;
    }

    Duration& operator*=(count_t n) BOOST_NOEXCEPT
    {
        count_ *= n;
        return *this;
    }

    Duration& operator/=(count_t n) BOOST_NOEXCEPT
    {
        count_ /= n;
        return *this;
    }

    Duration& operator%=(const Duration& rhs) BOOST_NOEXCEPT
    {
        count_ %= rhs.count_;
        return *this;
    }

    template<class Res2>
    Duration& operator%=(const Duration<Res2>& rhs) BOOST_NOEXCEPT
    {
        count_ %= aux::ConvertCount<Res2, Res>(rhs.count_);
        return *this;
    }

    Duration operator-(void) BOOST_NOEXCEPT
    {
        return Duration(-count_);
    }

    bool operator!(void) BOOST_NOEXCEPT
    {
        return !count_;
    }

#if !defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    explicit operator bool() const BOOST_NOEXCEPT
    {
        return !!count_;
    }
#else // defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
private:
    bool ToBool(void) const BOOST_NOEXCEPT
    {
        return !!count_;
    }

public:
    typedef bool (Duration::* BoolType)(void) const BOOST_NOEXCEPT;
    operator BoolType() const BOOST_NOEXCEPT
    {
        return !!count_ ? &Duration::ToBool : nullptr;
    }
#endif // !defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

    /*}}}*/

    // Methods./*{{{*/
    /**
     * @brief Get the number of fundamental periods.
     */
    count_t GetCount(void) const BOOST_NOEXCEPT
    {
        return count_;
    }

    /**
     * @brief Get the fundamental period in seconds.
     */
    static double GetResolution(void) BOOST_NOEXCEPT
    {
        static const double res = static_cast<double>(Res::num)
                                / static_cast<double>(Res::den);
        return res;
    }

    /**
     * @brief Get the number of fundamental period per second.
     */
    static double GetFrequency(void) BOOST_NOEXCEPT
    {
        static const double freq = static_cast<double>(Res::den)
                                 / static_cast<double>(Res::num);
        return freq;
    }

    /**
     * @brief Get the size of duration in bytes.
     */
    static size_t GetSize(void) BOOST_NOEXCEPT
    {
        return sizeof (count_t);
    }

    void swap(Duration& rhs) BOOST_NOEXCEPT
    {
        boost::swap(count_, rhs.count_);
    }

    /**
     * @brief Split the duration into parts.
     *
     * @param[out] neg  Whether the duration is negative.
     * @param[out] d    Days.
     * @param[out] h    Hours.
     * @param[out] m    Minutes.
     * @param[out] s    Seconds.
     * @param[out] ms   MilliSeconds.
     * @param[out] us   MicroSeconds.
     * @param[out] ns   NanoSeconds.
     * @param[out] ps   PicoSeconds.
     * @param[out] fs   FemtoSeconds.
     * @param[out] as   AttoSeconds.
     * @param[out] zs   ZeptoSeconds.
     * @param[out] ys   YoctoSeconds.
     */
    void Split(bool& neg,
               count_t& d,  count_t& h,  count_t& m,
               count_t& s,  count_t& ms, count_t& us,
               count_t& ns, count_t& ps, count_t& fs,
               count_t& as, count_t& zs, count_t& ys) const BOOST_NOEXCEPT
    {
        neg = (count_ < 0);
        count_t u = neg ? -count_ : count_;
        count_t v = 1;
        aux::RatioDivide(d,  u, v, Day());
        aux::RatioDivide(h,  u, v, Hour());
        aux::RatioDivide(m,  u, v, Minute());
        aux::RatioDivide(s,  u, v, Second());
        aux::RatioDivide(ms, u, v, MilliSecond());
        aux::RatioDivide(us, u, v, MicroSecond());
        aux::RatioDivide(ns, u, v, NanoSecond());
        aux::RatioDivide(ps, u, v, PicoSecond());
        aux::RatioDivide(fs, u, v, FemtoSecond());
        aux::RatioDivide(as, u, v, AttoSecond());
        aux::RatioDivide(zs, u, v, ZeptoSecond());
        aux::RatioDivide(ys, u, v, YoctoSecond());
    }

    /*}}}*/

    // I/O./*{{{*/
public:
    /**
     * @brief Convert to formatted string.
     *
     * The format looks like:
     * ~~~
     * "<DD> <HH>:<MM>:<SS> <mmm>m <uuu>u <nnn>n <ppp>p <fff>f <aaa>a <zzz>z <yyy>y"
     * ~~~
     *
     * The "DD" days part is omitted, if the duration is less than a day.
     * The units smaller than second are shown if they are not `UnderflowTag`.
     */
    std::string ToString(void) const;

    /*}}}*/

    // Double./*{{{*/
public:
    /**
     * @brief Convert the duration to a double value in seconds.
     *
     * @param[in] sec The duration in seconds.
     */
    double ToDouble(void) const
    {
        return count_ * GetResolution();
    }
    /*}}}*/

    // Limits./*{{{*/
    static Duration Zero(void) BOOST_NOEXCEPT
    {
        return Duration();
    }

    static Duration Min(void) BOOST_NOEXCEPT
    {
        return Duration(INT64_MIN);
    }

    static Duration Max(void) BOOST_NOEXCEPT
    {
        return Duration(INT64_MAX);
    }
    /*}}}*/

    // Boolean./*{{{*/
    /**
     * @brief A shortcut to check whether a duration is zero.
     *
     * @return The return value equals to `*this == Duration::Zero()`, which
     *         also equals to `*this == Duration()`.
     */
    bool operator!(void) const BOOST_NOEXCEPT
    {
        return !count_;
    }
    /*}}}*/

private:
    count_t count_;
};


////////////////////////////////////////////////////////////////////////////////
// Comparison./*{{{*/
template<class Res1, class Res2>
inline bool
operator<(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::WhichIsFiner<Res1, Res2>::type  F;
    return aux::Less<Res1, Res2>(lhs.count_, rhs.count_, F());
}

template<class Res1, class Res2>
inline bool
operator<=(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::WhichIsFiner<Res1, Res2>::type  F;
    return aux::LessEqual<Res1, Res2>(lhs.count_, rhs.count_, F());
}

template<class Res1, class Res2>
inline bool
operator==(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::WhichIsFiner<Res1, Res2>::type  F;
    return aux::Equal<Res1, Res2>(lhs.count_, rhs.count_, F());
}

template<class Res1, class Res2>
inline bool
operator!=(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::WhichIsFiner<Res1, Res2>::type  F;
    return !aux::Equal<Res1, Res2>(lhs.count_, rhs.count_, F());
}

template<class Res1, class Res2>
inline bool
operator>(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::WhichIsFiner<Res1, Res2>::type  F;
    return !aux::LessEqual<Res1, Res2>(lhs.count_, rhs.count_, F());
}

template<class Res1, class Res2>
inline bool
operator>=(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::WhichIsFiner<Res1, Res2>::type  F;
    return !aux::Less<Res1, Res2>(lhs.count_, rhs.count_, F());
}

/*}}}*/


////////////////////////////////////////////////////////////////////////////////
// Arithmetic./*{{{*/
template<class Res1, class Res2>
inline Duration<typename aux::Common<Res1, Res2>::type>
operator+(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::Common<Res1, Res2>::type Res3;
    return Duration<Res3>(aux::ConvertCount<Res1, Res3>(lhs.count_) +
                          aux::ConvertCount<Res2, Res3>(rhs.count_));
}

template<class Res1, class Res2>
inline Duration<typename aux::Common<Res1, Res2>::type>
operator-(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::Common<Res1, Res2>::type Res3;
    return Duration<Res3>(aux::ConvertCount<Res1, Res3>(lhs.count_) -
                          aux::ConvertCount<Res2, Res3>(rhs.count_));
}

template<class Res>
inline Duration<Res>
operator*(const Duration<Res>& lhs, count_t n) BOOST_NOEXCEPT
{
    return Duration<Res>(lhs.count_ * n);
}

template<class Res>
inline Duration<Res>
operator*(count_t n, const Duration<Res>& rhs) BOOST_NOEXCEPT
{
    return Duration<Res>(n * rhs.count_);
}

template<class Res1, class Res2>
inline count_t
operator/(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::Common<Res1, Res2>::type Res3;
    return aux::ConvertCount<Res1, Res3>(lhs.count_) /
           aux::ConvertCount<Res2, Res3>(rhs.count_);
}

template<class Res>
inline Duration<Res>
operator/(const Duration<Res>& lhs, count_t n) BOOST_NOEXCEPT
{
    return Duration<Res>(lhs.count_ / n);
}

template<class Res1, class Res2>
inline Duration<typename aux::Common<Res1, Res2>::type>
operator%(const Duration<Res1>& lhs, const Duration<Res2>& rhs) BOOST_NOEXCEPT
{
    typedef typename aux::Common<Res1, Res2>::type Res3;
    return Duration<Res3>(aux::ConvertCount<Res1, Res3>(lhs.count_) %
                          aux::ConvertCount<Res2, Res3>(rhs.count_));
}
/*}}}*/


////////////////////////////////////////////////////////////////////////////////
// I/O./*{{{*/
template<class Res>
inline std::string
Duration<Res>::ToString(void) const
{
    bool neg;
    count_t d;
    count_t h;
    count_t m;
    count_t s;
    count_t ms;
    count_t us;
    count_t ns;
    count_t ps;
    count_t fs;
    count_t as;
    count_t zs;
    count_t ys;
    Split(neg, d, h, m, s, ms, us, ns, ps, fs, as, zs, ys);

    std::stringstream oss;
    if (neg)
    {
        oss << '-';
    }
    if (d)
    {
        oss << std::setw(0) << d << " ";
    }
    oss << std::setw(2) << std::setfill('0') << h  << ":"
        << std::setw(2) << std::setfill('0') << m  << ":"
        << std::setw(2) << std::setfill('0') << s;

    if (!std::is_same<MilliSecond, aux::UnderflowTag>::value)
    {
        oss << "-" << std::setw(3) << std::setfill('0') << ms << "m";
    }
    if (!std::is_same<MicroSecond, aux::UnderflowTag>::value)
    {
        oss << "-" << std::setw(3) << std::setfill('0') << us << "u";
    }
    if (!std::is_same<NanoSecond, aux::UnderflowTag>::value)
    {
        oss << "-" << std::setw(3) << std::setfill('0') << ns << "n";
    }
    if (!std::is_same<PicoSecond, aux::UnderflowTag>::value)
    {
        oss << "-" << std::setw(3) << std::setfill('0') << ps << "p";
    }
    if (!std::is_same<FemtoSecond, aux::UnderflowTag>::value)
    {
        oss << "-" << std::setw(3) << std::setfill('0') << fs << "f";
    }
    if (!std::is_same<AttoSecond, aux::UnderflowTag>::value)
    {
        oss << "-" << std::setw(3) << std::setfill('0') << as << "a";
    }
    if (!std::is_same<ZeptoSecond, aux::UnderflowTag>::value)
    {
        oss << "-" << std::setw(3) << std::setfill('0') << zs << "z";
    }
    if (!std::is_same<YoctoSecond, aux::UnderflowTag>::value)
    {
        oss << "-" << std::setw(3) << std::setfill('0') << ys << "y";
    }
    return oss.str();
}

template<class CharT, class TraitsT, class Res>
inline std::basic_ostream<CharT, TraitsT>&
operator<<(std::basic_ostream<CharT, TraitsT>& os, const Duration<Res>& dt)
{
    return (os << dt.ToString());
}

/*}}}*/


////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Support for `boost::hash<Duration>`.
 */
template<class Res>
inline size_t
hash_value(const Duration<Res>& dt) BOOST_NOEXCEPT
{
    return boost::hash<count_t>()(dt.GetCount());
}

template<class Res>
inline void
swap(Duration<Res>& lhs, Duration<Res>& rhs) BOOST_NOEXCEPT
{
    lhs.swap(rhs);
}


////////////////////////////////////////////////////////////////////////////////
// Makers./*{{{*/
inline Duration<atto>
AttoSeconds(count_t n) BOOST_NOEXCEPT
{
    return Duration<atto>(n);
}

inline Duration<femto>
FemtoSeconds(count_t n) BOOST_NOEXCEPT
{
    return Duration<femto>(n);
}

inline Duration<pico>
PicoSeconds(count_t n) BOOST_NOEXCEPT
{
    return Duration<pico>(n);
}

inline Duration<nano>
NanoSeconds(count_t n) BOOST_NOEXCEPT
{
    return Duration<nano>(n);
}

inline Duration<micro>
MicroSeconds(count_t n) BOOST_NOEXCEPT
{
    return Duration<micro>(n);
}

inline Duration<milli>
MilliSeconds(count_t n) BOOST_NOEXCEPT
{
    return Duration<milli>(n);
}

inline Duration<ratio<1>>
Seconds(count_t n) BOOST_NOEXCEPT
{
    return Duration<ratio<1>>(n);
}

inline Duration<ratio<1>>
Minutes(count_t n) BOOST_NOEXCEPT
{
    return Duration<ratio<1>>(n * 60);
}

inline Duration<ratio<1>>
Hours(count_t n) BOOST_NOEXCEPT
{
    return Duration<ratio<1>>(n * 3600);
}

inline Duration<ratio<1>>
Days(count_t n) BOOST_NOEXCEPT
{
    return Duration<ratio<1>>(n * 86400);
}

/*}}}*/


NSFX_CHRONO_CLOSE_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
namespace std /*{{{*/
{

template<class Res1, class Res2>
struct common_type<::nsfx::chrono::Duration<Res1>,
                   ::nsfx::chrono::Duration<Res2>>
{
    typedef ::nsfx::chrono::Duration<
                typename ::nsfx::chrono::aux::Common<Res1, Res2>::type> type;
};

} // namespace std /*}}}*/



#endif // DURATION_H__E90A15BC_96A8_4FA8_99A1_E3DC5B5A2A66

