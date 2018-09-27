/**
 * @file
 *
 * @brief Random number support for Network Simulation Frameworks.
 *
 * @version 1.0
 * @author  Wei Tang (gauchyler@uestc.edu.cn)
 * @date    2018-09-26
 *
 * @copyright Copyright (c) 2018.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#ifndef XOROSHIRO_ENGINE_H__F9ACBF0B_52EC_412C_B70D_3397EF964427
#define XOROSHIRO_ENGINE_H__F9ACBF0B_52EC_412C_B70D_3397EF964427


#include <nsfx/random/config.h>
#include <nsfx/random/engine/splitmix-engine.h>

#include <nsfx/random/detail/rot.h>
#include <nsfx/random/detail/is-power-2.h>

#include <type_traits> // is_same
#include <cstring> // memcpy


NSFX_OPEN_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
namespace random {
namespace aux {


////////////////////////////////////////
// The state of xoroshiro (n >= 4).
template<class UIntType, size_t n>
struct xoroshiro_state
{
    static_assert(is_power_2<n>::value,
                  "Invalid state size, it must be a power of 2.");

    xoroshiro_state(void) : p_(n-1) {}
    UIntType s_[n];
    size_t   p_; // Indicates the last register.
};

// The state of xoroshiro (n == 2).
template<class UIntType>
struct xoroshiro_state<UIntType, 2>
{
    UIntType s_[2];
};

////////////////////////////////////////
// The + scrambler (n >= 4).
template<class UIntType, size_t n, size_t i, size_t j>
struct xoroshiro_plus_scrambler
{
    typedef UIntType result_type;
    BOOST_STATIC_CONSTANT(size_t, state_size = n);
    static_assert(i < n, "Invalid parameter.");
    static_assert(j < n, "Invalid parameter.");
    static result_type scramble(xoroshiro_state<UIntType, n>& state)
    {
        return state.s_[(p_ + (i+1) & (n-1)) & (n-1)] +
               state.s_[(p_ + (j+1) & (n-1)) & (n-1)];
    }
};

// The * scrambler (n >= 4).
template<class UIntType, size_t n, size_t i, UIntType s>
struct xoroshiro_star_scrambler
{
    typedef UIntType result_type;
    BOOST_STATIC_CONSTANT(size_t, state_size = n);
    static_assert(i < n, "Invalid parameter.");
    static result_type scramble(xoroshiro_state<UIntType, n>& state)
    {
        return state.s_[(p_ + (i+1) & (n-1)) & (n-1)] * s;
    }
};

// The ** scrambler (n >= 4).
template<class UIntType, size_t n, size_t i, UIntType s, size_t r, UIntType t>
struct xoroshiro_starstar_scrambler
{
    typedef UIntType result_type;
    BOOST_STATIC_CONSTANT(size_t, state_size = n);
    static_assert(i < n, "Invalid parameter.");
    static result_type scramble(xoroshiro_state<UIntType, n>& state)
    {
        return rotl(state.s_[(p_ + (i+1) & (n-1)) & (n-1)] * s, r) * t;
    }
};

////////////////////////////////////////
// The + scrambler (n == 2).
template<class UIntType, size_t i, size_t j>
struct xoroshiro_plus_scrambler<UIntType, 2, i, j>
{
    typedef UIntType result_type;
    BOOST_STATIC_CONSTANT(size_t, state_size = n);
    static_assert(i < n, "Invalid parameter.");
    static_assert(j < n, "Invalid parameter.");
    static result_type scramble(xoroshiro_state<UIntType, n>& state)
    {
        return state.s_[i] + state.s_[j];
    }
};

// The * scrambler (n == 2).
template<class UIntType, size_t i, UIntType s>
struct xoroshiro_star_scrambler<UIntType, 2, i, s>
{
    typedef UIntType result_type;
    BOOST_STATIC_CONSTANT(size_t, state_size = n);
    static_assert(i < n, "Invalid parameter.");
    static result_type scramble(xoroshiro_state<UIntType, n>& state)
    {
        return state.s_[i] * s;
    }
};

// The ** scrambler (n == 2).
template<class UIntType, size_t i, UIntType s, size_t r, UIntType t>
struct xoroshiro_starstar_scrambler<UIntType, 2, i, s, r, t>
{
    typedef UIntType result_type;
    BOOST_STATIC_CONSTANT(size_t, state_size = n);
    static_assert(i < n, "Invalid parameter.");
    static result_type scramble(xoroshiro_state<UIntType, n>& state)
    {
        return rotl(state.s_[i] * s, r) * t;
    }
};


} // namespace aux
} // namespace random


////////////////////////////////////////////////////////////////////////////////
/**
 * @ingroup Random
 * @brief A xoroshiro pseudo-random number generator.
 */
template<class UIntType, size_t n,
         size_t a, size_t b, size_t c,
         class Scrambler>
class xoroshiro_engine :
    public random::aux::xoroshiro_state<UIntType, n>
{
public:
    typedef UIntType result_type;
    BOOST_STATIC_CONSTANT(result_type, default_seed = 1u);
    BOOST_STATIC_CONSTANT(size_t, state_size = n);
    BOOST_STATIC_CONSTANT(size_t, rotate_a = a);
    BOOST_STATIC_CONSTANT(size_t, shift_b  = b);
    BOOST_STATIC_CONSTANT(size_t, rotate_c = c);

    static_assert(std::is_same<typename Scrambler::result_type,
                               result_type>::value,
                  "Incompatible scrambler type.");

    static_assert(Scrambler::state_size == state_size,
                  "Incompatible scrambler type.");

private:
    template<size_t n>
    struct state_size_tag {};

    // For n > 2.
    template<size_t n>
    void seed(result_type value, state_size_tag<n>)
    {
        seed(value, state_size_tag<2>());
        p_ = n - 1;
    }

    template<size_t n>
    void transform(state_size_tag<n>)
    {
        size_t q = p_;
        p_ = (p_ + 1) & (n - 1); // n must be 2^k, where k >= 1.
        const UIntType s0 = s_[p_];
              UIntType s1 = s_[q];
        s1 ^= s0;
        s_[q]  = random::aux::rotl(s0, a) ^ s1 ^ (s1 << b);
        s_[p_] = random::aux::rotl(s1, c);
    }

    // For n == 2.
    void seed(result_type value, state_size_tag<2>)
    {
        splitmix64 gen(value);
        for (size_t i = 0; i < n; ++i)
        {
            s_[i] = static_cast<UIntType>(gen());
        }
    }

    void transform(state_size_tag<2>)
    {
        const UIntType s0 = s_[0];
              UIntType s1 = s_[1];
        s1 ^= s0;
        s_[0] = random::aux::rotl(s0, a) ^ s1 ^ (s1 << b);
        s_[1] = random::aux::rotl(s1, c);
    }

public:
    xoroshiro_engine(void)
    {
        seed(default_seed);
    }

    explicit xoroshiro_engine(result_type value) :
    {
        seed(value);
    }

    void seed(result_type value)
    {
        seed(value, state_size_tag<n>());
    }

public:
    result_type operator()(void)
    {
        const UIntType result = Scrambler::scramble(s_);
        transform(state_size_tag<n>());
        return result;
    }

    void discard(uintmax_t z)
    {
        for (uintmax_t i = 0; i < z; ++i)
        {
            (*this)();
        }
    }

    static result_type (min)(void)
    {
        return 0;
    }

    static result_type (max)(void)
    {
        return std::numeric_limits<result_type>::(max)();
    }
};

////////////////////////////////////////
/**
 * @ingroup Random
 * @brief A xoroshiro64* pseudo-random number generator.
 *
 * It is discovered by David Blackman and Sebastiano Vigna in 2018.
 * See http://vigna.di.unimi.it/ftp/papers/ScrambledLinear.pdf
 *
 * This is \c xoroshiro64* 1.0, the authors' best and fastest 32-bit
 * small-state generator for 32-bit floating-point numbers.
 *
 * The authors suggest to use its upper bits for floating-point generation,
 * as it is slightly faster than \c xoroshiro64**.
 * It passes all tests the authors are aware of except for linearity tests,
 * as the lowest six bits have low linear complexity, so if low linear
 * complexity is not considered an issue (as it is usually the case), it can
 * be used to generate 32-bit outputs, too.
 *
 * The authors suggest to use a sign test to extract a random Boolean value,
 * and right shifts to extract subsets of bits.
 *
 * A \c splitmix64 generator is seeded, and its output is used to fill the state.
 */
typedef xoroshiro_engine<uint32_t, 2, 26, 9, 13,
        random::aux::xoroshiro_star_scrambler<uint32_t, 2, 0, 0x9e3779bb> >
    xoroshiro64star;

/**
 * @ingroup Random
 * @brief A xoroshiro64** pseudo-random number generator.
 *
 * It is discovered by David Blackman and Sebastiano Vigna in 2018.
 * See http://vigna.di.unimi.it/ftp/papers/ScrambledLinear.pdf
 *
 * This is \c xoroshiro64** 1.0, the authors' 32-bit all-purpose, rock-solid,
 * small-state generator.
 * It is extremely fast and it passes all tests the authors are aware of, but
 * its state space is not large enough for any parallel application.
 *
 * For generating just single-precision (i.e., 32-bit) floating-point
 * numbers, \c xoroshiro64* is even faster.
 *
 * A \c splitmix64 generator is seeded, and its output is used to fill the state.
 */
typedef xoroshiro_engine<uint32_t, 2, 26, 9, 13,
        random::aux::xoroshiro_starstar_scrambler<uint32_t, 2, 0, 0x9e3779bb, 5, 5> >
    xoroshiro64starstar;

/**
 * @ingroup Random
 * @brief A xoroshiro128+ pseudo-random number generator.
 *
 * It is discovered by David Blackman and Sebastiano Vigna in 2018.
 * See http://vigna.di.unimi.it/ftp/papers/ScrambledLinear.pdf
 *
 * This is \c xoroshiro128+ 1.0, the authors' best and fastest small-state
 * generator for floating-point numbers.
 * The authors suggest to use its upper bits for floating-point generation,
 * as it is slightly faster than \c xoroshiro128**.
 * It passes all tests the authors are aware of except for the four lower bits,
 * which might fail linearity tests (and just those), so if low linear
 * complexity is not considered an issue (as it is usually the case), it can
 * be used to generate 64-bit outputs, too; moreover, this generator has a
 * very mild Hamming-weight dependency making our test
 * (http://prng.di.unimi.it/hwd.php) fail after 8 TB of output; the authors
 * believe this slight bias cannot affect any application.
 * If the users are concerned, use \c xoroshiro128** or \c xoshiro256+.
 *
 * The authors suggest to use a sign test to extract a random Boolean value,
 * and right shifts to extract subsets of bits.
 *
 * The authors suggest to seed a \c splitmix64 generator and use its output to
 * fill the state.
 */
typedef xoroshiro_engine<uint64_t, 2, 24, 16, 37,
        random::aux::xoroshiro_plus_scrambler<uint64_t, 2, 0, 1> >
    xoroshiro128plus;

/**
 * @ingroup Random
 * @brief A xoroshiro128** pseudo-random number generator.
 *
 * It is discovered by David Blackman and Sebastiano Vigna in 2018.
 * See http://vigna.di.unimi.it/ftp/papers/ScrambledLinear.pdf
 *
 * This is \c xoroshiro128** 1.0, the authors' all-purpose, rock-solid,
 * small-state generator.
 * It is extremely (sub-ns) fast and it passes all tests the authors are
 * aware of, but its state space is large enough only for mild parallelism.
 *
 * For generating just floating-point numbers, \c xoroshiro128+ is even
 * faster (but it has a very mild bias, see the comments).
 *
 * The authors suggest to seed a \c splitmix64 generator and use its output to
 * fill the state.
 */
typedef xoroshiro_engine<uint64_t, 2, 24, 16, 37,
        random::aux::xoroshiro_starstar_scrambler<uint64_t, 2, 0, 5, 7, 9> >
    xoroshiro128starstar;


NSFX_CLOSE_NAMESPACE


#endif // XOROSHIRO_ENGINE_H__F9ACBF0B_52EC_412C_B70D_3397EF964427

