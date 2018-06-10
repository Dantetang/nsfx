﻿/**
 * @file
 *
 * @brief Utility for Network Simulation Frameworks.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2015-02-26
 *
 * @copyright Copyright (c) 2015.
 *            National Key Laboratory of Science and Technology on Communications,
 *            University of Electronic Science and Technology of China.
 *            All rights reserved.
 */

#ifndef CIRCULAR_SEQUENCE_NUMBER_H__4D0E7B48_E09B_44E6_A896_D56EA9C74306
#define CIRCULAR_SEQUENCE_NUMBER_H__4D0E7B48_E09B_44E6_A896_D56EA9C74306


#include <nsfx/utility/config.h>
#include <nsfx/utility/least-int.h>
#include <nsfx/exception/exception.h>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/conditional.hpp>
#include <boost/functional/hash.hpp>
#include <boost/core/swap.hpp>
#include <iostream>
#include <sstream>


NSFX_OPEN_NAMESPACE


/////////////////////////////////////////////////////////////////////////////////
/**
 * @ingroup Utility
 * @brief A cyclic sequence number.
 *
 * @tparam bits The number of bits. Must be within <code>[2, 64]</code>.
 */
template<size_t bits>
class CircularSequenceNumberTraits/*{{{*/
{
    static_assert(bits >= 2 && bits <= 64,
                  "Invalid number of bits for CircularSequenceNumber.");

public:
    typedef typename LeastInt<bits>::UintType ValueType;

    BOOST_STATIC_CONSTANT(ValueType, MAX_GAP =
        static_cast<ValueType>(~(static_cast<ValueType>(-1) << (bits - 1)))
    );

    BOOST_STATIC_CONSTANT(ValueType, MAX_VALUE =
        static_cast<ValueType>(-1) >> (sizeof (ValueType) * 8 - bits)
    );

};/*}}}*/


/////////////////////////////////////////////////////////////////////////////////
template<size_t bits>
class CircularSequenceNumber/*{{{*/
{
    static_assert(bits >= 2 && bits <= 64,
                  "Invalid number of bits for CircularSequenceNumber.");

public:
    typedef CircularSequenceNumber              ThisType;
    typedef CircularSequenceNumberTraits<bits>  TraitsType;
    typedef typename TraitsType::ValueType      ValueType;

public:
    BOOST_CONSTEXPR CircularSequenceNumber(void) BOOST_NOEXCEPT :
        value_(0)
    {
    }

    explicit BOOST_CONSTEXPR CircularSequenceNumber(ValueType value) :
        value_(value)
    {
        if (value > TraitsType::MAX_VALUE)
        {
            BOOST_THROW_EXCEPTION(OutOfBounds());
        }
    }

    BOOST_CONSTEXPR ValueType GetValue(void) const BOOST_NOEXCEPT
    {
        return value_;
    }

    // Operators.
public:
    BOOST_CONSTEXPR ThisType& operator++(void) BOOST_NOEXCEPT
    {
        InternalIncrement();
        return *this;
    }

    BOOST_CONSTEXPR ThisType operator++(int) BOOST_NOEXCEPT
    {
        ValueType old = value_;
        InternalIncrement();
        return ThisType(old);
    }

    BOOST_CONSTEXPR ThisType& operator--(void) BOOST_NOEXCEPT
    {
        InternalDecrement();
        return *this;
    }

    BOOST_CONSTEXPR ThisType operator--(int) BOOST_NOEXCEPT
    {
        ValueType old = value_;
        InternalDecrement();
        return ThisType(old);
    }

private:
    BOOST_CONSTEXPR void InternalIncrement(void) BOOST_NOEXCEPT
    {
        ++value_;
        if (value_ > TraitsType::MAX_VALUE)
        {
            value_ = 0;
        }
    }

    BOOST_CONSTEXPR void InternalDecrement(void) BOOST_NOEXCEPT
    {
        if (value_ == 0)
        {
            value_ = TraitsType::MAX_VALUE;
        }
        else
        {
            --value_;
        }
    }

public:
    bool operator< (const ThisType& rhs) const BOOST_NOEXCEPT
    {
        if (value_ < rhs.value_)
        {
            return static_cast<ValueType>(rhs.value_ - value_) <=
                TraitsType::MAX_GAP;
        }
        else
        {
            return static_cast<ValueType>(value_ - rhs.value_) >
                TraitsType::MAX_GAP;
        }
    }

    bool operator<=(const ThisType& rhs) const BOOST_NOEXCEPT
    {
        if (value_ <= rhs.value_)
        {
            return static_cast<ValueType>(rhs.value_ - value_) <=
                TraitsType::MAX_GAP;
        }
        else
        {
            return static_cast<ValueType>(value_ - rhs.value_) >
                TraitsType::MAX_GAP;
        }
    }

    bool operator==(const ThisType& rhs) const BOOST_NOEXCEPT
    {
        return value_ == rhs.value_;
    }

    bool operator!=(const ThisType& rhs) const BOOST_NOEXCEPT
    {
        return value_ != rhs.value_;
    }

    bool operator> (const ThisType& rhs) const BOOST_NOEXCEPT
    {
        return !(*this <= rhs);
    }

    bool operator>=(const ThisType& rhs) const BOOST_NOEXCEPT
    {
        return !(*this < rhs);
    }

    void swap(ThisType& rhs) BOOST_NOEXCEPT
    {
        if (this != &rhs)
        {
            boost::swap(value_, rhs.value_);
        }
    }

    std::string ToString(void) const
    {
        return ToString(
            boost::integral_constant<bool, sizeof (ValueType) == 1>());
    }

private:
    std::string ToString(boost::true_type /* 8-bit */) const
    {
        std::ostringstream oss;
        oss << static_cast<typename LeastInt<32>::UintType>(value_);
        oss << value_;
        return oss.str();
    }

    std::string ToString(boost::false_type /* larger than 8-bit */) const
    {
        std::ostringstream oss;
        oss << value_;
        return oss.str();
    }


private:
    ValueType value_;

};/*}}}*/


/////////////////////////////////////////////////////////////////////////////////
// Free functions./*{{{*/
template<size_t bits>
inline size_t
hash_value(const CircularSequenceNumber<bits>& csn) BOOST_NOEXCEPT
{
    typedef typename CircularSequenceNumber<bits>::ValueType  ValueType;
    return boost::hash<ValueType>()(csn.GetValue());
}

template<size_t bits>
inline void
swap(CircularSequenceNumber<bits>& lhs,
     CircularSequenceNumber<bits>& rhs) BOOST_NOEXCEPT
{
    return lhs.swap(rhs);
}

template<class Char, class Traits, size_t bits>
inline std::basic_ostream<Char, Traits>&
operator<<(std::basic_ostream<Char, Traits>& os,
           const CircularSequenceNumber<bits>& seqno)
{
    return os << seqno.ToString();
}


/*}}}*/


NSFX_CLOSE_NAMESPACE


#endif // CIRCULAR_SEQUENCE_NUMBER_H__4D0E7B48_E09B_44E6_A896_D56EA9C74306

