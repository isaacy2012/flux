
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


/*
 * Note that these test are exactly the same as the ones in test_checked_ops.cpp
 * but calling num::xxx rather than num::checked_xxx.
 *
 * If adding/modifying tests, make sure to do the same in both files
 */

#include <climits> // for CHAR_BIT

#include "../test_utils.hpp"

namespace {

/*
 * Compile-time tests
 *
 * We need to do these separately because overflow is a compile error, whereas
 * for runtime we want to test that it throws as expected
 */

template <auto>
struct Tester {};

#define CONSTEXPR_CALLABLE(expr) requires { typename Tester<expr>; }

template <typename T>
constexpr bool test_add_constexpr()
{
    constexpr auto& add = flux::num::add;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();


    // adding zero to anything doesn't change it, and doesn't overflow
    STATIC_CHECK(add(zero, zero) == zero);
    STATIC_CHECK(add(min, zero) == min);
    STATIC_CHECK(add(zero, min) == min);
    STATIC_CHECK(add(max, zero));
    STATIC_CHECK(add(zero, max) == max);

    // Make sure that we *can't* add(max, one) at compile time
    static_assert(not CONSTEXPR_CALLABLE(add(max, one)));
    static_assert(not CONSTEXPR_CALLABLE(add(one, max)));

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        // "adding" minus one to min overflows, and can't be constexpr-called
        static_assert(not CONSTEXPR_CALLABLE(add(min, minus_one)));
        static_assert(not CONSTEXPR_CALLABLE(add(minus_one, min)));

        // adding min and max gives -1, and doesn't overflow
        STATIC_CHECK(add(min, max) == minus_one);
        STATIC_CHECK(add(max, min) == minus_one);
    }

    return true;
}

static_assert(test_add_constexpr<signed char>());
static_assert(test_add_constexpr<unsigned char>());
static_assert(test_add_constexpr<signed short>());
static_assert(test_add_constexpr<unsigned short>());
static_assert(test_add_constexpr<signed int>());
static_assert(test_add_constexpr<unsigned int>());
static_assert(test_add_constexpr<signed long>());
static_assert(test_add_constexpr<unsigned long>());
static_assert(test_add_constexpr<signed long long>());
static_assert(test_add_constexpr<unsigned long long>());

template <typename T>
constexpr bool test_sub_constexpr()
{
    constexpr auto& sub = flux::num::sub;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Anything minus zero is itself, and doesn't overflow
    STATIC_CHECK(sub(zero, zero) == zero);
    STATIC_CHECK(sub(one, zero) == one);
    STATIC_CHECK(sub(min, zero) == min);

    // Anything minus itself is zero, and doesn't overflow
    STATIC_CHECK(sub(one, one) == zero);
    STATIC_CHECK(sub(max, max) == zero);
    STATIC_CHECK(sub(min, min) == zero);

    // min minus a positive value overflows, and can't be used in constexpr
    STATIC_CHECK(not CONSTEXPR_CALLABLE(sub(min, one)));

    // min minus max overflows
    STATIC_CHECK(not CONSTEXPR_CALLABLE(sub(min, max)));

    // max minus a positive value does not overflow
    STATIC_CHECK(sub(max, one) > zero);

    // max minus min is fine for unsigned, overflows for signed
    if constexpr (not flux::num::signed_integral<T>) {
        STATIC_CHECK(sub(max, min) == max);
    } else {
        STATIC_CHECK(not CONSTEXPR_CALLABLE(sub(max, min)));
    }

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        STATIC_CHECK(sub(minus_one, minus_one) == zero);

        STATIC_CHECK(sub(minus_one, min) == max);

        STATIC_CHECK(sub(minus_one, max) == min);

        STATIC_CHECK(sub(min, minus_one) < zero);

        STATIC_CHECK(not CONSTEXPR_CALLABLE(sub(max, minus_one)));
    }

    return true;
}

static_assert(test_sub_constexpr<signed char>());
static_assert(test_sub_constexpr<unsigned char>());
static_assert(test_sub_constexpr<signed short>());
static_assert(test_sub_constexpr<unsigned short>());
static_assert(test_sub_constexpr<signed int>());
static_assert(test_sub_constexpr<unsigned int>());
static_assert(test_sub_constexpr<signed long>());
static_assert(test_sub_constexpr<unsigned long>());
static_assert(test_sub_constexpr<signed long long>());
static_assert(test_sub_constexpr<unsigned long long>());

template <typename T>
constexpr bool test_mul_constexpr()
{
    constexpr auto& mul = flux::num::mul;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Anything times zero is zero, and doesn't overflow
    STATIC_CHECK(mul(zero, zero) == zero);
    STATIC_CHECK(mul(zero, one)  == zero);
    STATIC_CHECK(mul(one, zero)  == zero);
    STATIC_CHECK(mul(zero, min)  == zero);
    STATIC_CHECK(mul(min, zero)  == zero);
    STATIC_CHECK(mul(zero, max)  == zero);
    STATIC_CHECK(mul(max, zero)  == zero);

    // Anything times one is itself, and doesn't overflow
    STATIC_CHECK(mul(one, one) == one);
    STATIC_CHECK(mul(min, one) == min);
    STATIC_CHECK(mul(one, min) == min);
    STATIC_CHECK(mul(one, max) == max);
    STATIC_CHECK(mul(max, one) == max);

    // max squared overflows
    STATIC_CHECK(not CONSTEXPR_CALLABLE(mul(max, max)));

    // min squared is min on unsigned, overflows on signed
    if constexpr (flux::num::unsigned_integral<T>) {
        STATIC_CHECK(mul(min, min) == min);
    } else {
        STATIC_CHECK(not CONSTEXPR_CALLABLE(mul(min, min)));
    }

    // min times max overflows on signed
    if constexpr (flux::num::unsigned_integral<T>) {
        STATIC_CHECK(mul(min, max) == min);
        STATIC_CHECK(mul(max, min) == min);
    } else {
        STATIC_CHECK(not CONSTEXPR_CALLABLE(mul(min, max)));
        STATIC_CHECK(not CONSTEXPR_CALLABLE(mul(max, min)));
    }

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        STATIC_CHECK(mul(minus_one, one) == minus_one);
        STATIC_CHECK(mul(one, minus_one) == minus_one);
        STATIC_CHECK(mul(minus_one, minus_one) == one);

        STATIC_CHECK(mul(minus_one, max) == min + one);

        STATIC_CHECK(not CONSTEXPR_CALLABLE(mul(min, minus_one)));
        STATIC_CHECK(not CONSTEXPR_CALLABLE(mul(minus_one, min)));
    }

    return true;
}

static_assert(test_mul_constexpr<signed char>());
static_assert(test_mul_constexpr<unsigned char>());
static_assert(test_mul_constexpr<signed short>());
static_assert(test_mul_constexpr<unsigned short>());
static_assert(test_mul_constexpr<signed int>());
static_assert(test_mul_constexpr<unsigned int>());
static_assert(test_mul_constexpr<signed long>());
static_assert(test_mul_constexpr<unsigned long>());
static_assert(test_mul_constexpr<signed long long>());
static_assert(test_mul_constexpr<unsigned long long>());

template <typename T>
constexpr bool test_div_constexpr()
{
    constexpr auto& div = flux::num::div;

    constexpr T zero{0};
    constexpr T one{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    STATIC_CHECK(div(T{10}, T{5}) == T{2});

    // zero divided by anything is zero
    STATIC_CHECK(div(zero, one) == zero);
    STATIC_CHECK(div(zero, max) == zero);

    // one divided by one is one (test this once)
    STATIC_CHECK(div(one, one) == one);

    // anything divided by one is unchanged
    STATIC_CHECK(div(max, one) == max);
    STATIC_CHECK(div(min, one) == min);

    // dividing by zero is an error
    STATIC_CHECK(not CONSTEXPR_CALLABLE(div(one, zero)));

    // For signed type, dividing by -1 negates (except for the lowest value)
    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one{-1};
        STATIC_CHECK(div(one, minus_one) == minus_one);
        STATIC_CHECK(div(minus_one, one) == minus_one);
        STATIC_CHECK(div(minus_one, minus_one) == one);
        STATIC_CHECK(div(max, minus_one) == T{min + 1});
        STATIC_CHECK(not CONSTEXPR_CALLABLE(div(min, minus_one)));
    }

    return true;
}

static_assert(test_div_constexpr<signed char>());
static_assert(test_div_constexpr<unsigned char>());
static_assert(test_div_constexpr<signed short>());
static_assert(test_div_constexpr<unsigned short>());
static_assert(test_div_constexpr<signed int>());
static_assert(test_div_constexpr<unsigned int>());
static_assert(test_div_constexpr<signed long>());
static_assert(test_div_constexpr<unsigned long>());
static_assert(test_div_constexpr<signed long long>());
static_assert(test_div_constexpr<unsigned long long>());

template <typename T>
constexpr bool test_mod_constexpr()
{
    constexpr auto& mod = flux::num::mod;

    constexpr T zero{0};
    constexpr T one{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // 0 % x == 0 for all x
    STATIC_CHECK(mod(zero, one) == zero);
    STATIC_CHECK(mod(zero, T{2}) == zero);
    STATIC_CHECK(mod(zero, max) == zero);

    // x % 1 == 0 for all x
    STATIC_CHECK(mod(one, one) == zero);
    STATIC_CHECK(mod(T{2}, one) == zero);
    STATIC_CHECK(mod(min, one) == zero);
    STATIC_CHECK(mod(max, one) == zero);

    // x % max == x for all x < max
    STATIC_CHECK(mod(one, max) == one);
    STATIC_CHECK(mod(T{2}, max) == T{2});
    STATIC_CHECK(mod(max, max) == zero);

    // x % 0 is an error
    STATIC_CHECK(not CONSTEXPR_CALLABLE(mod(one, zero)));

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};
        // modulus with negative numbers makes my head hurt :(
        STATIC_CHECK(mod(one, minus_one) == zero);
        STATIC_CHECK(mod(minus_one, minus_one) == zero);
        STATIC_CHECK(mod(minus_one, one) == zero);
        STATIC_CHECK(mod(T{-3}, T{2}) == minus_one);

        // This should be an error
        STATIC_CHECK(not CONSTEXPR_CALLABLE(mod(min, minus_one)));
    }

    return true;
};

static_assert(test_mod_constexpr<signed char>());
static_assert(test_mod_constexpr<unsigned char>());
static_assert(test_mod_constexpr<signed short>());
static_assert(test_mod_constexpr<unsigned short>());
static_assert(test_mod_constexpr<signed int>());
static_assert(test_mod_constexpr<unsigned int>());
static_assert(test_mod_constexpr<signed long>());
static_assert(test_mod_constexpr<unsigned long>());
static_assert(test_mod_constexpr<signed long long>());
static_assert(test_mod_constexpr<unsigned long long>());

template <typename T>
constexpr bool test_neg_constexpr()
{
    constexpr auto& neg = flux::num::neg;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T minus_one = T{-1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Only neg(min) overflows
    STATIC_CHECK(neg(zero) == zero);
    STATIC_CHECK(neg(one) == minus_one);
    STATIC_CHECK(neg(minus_one) == one);
    STATIC_CHECK(neg(max) == min + 1);

    STATIC_CHECK(not CONSTEXPR_CALLABLE(neg(min)));

    return true;
}
static_assert(test_neg_constexpr<signed char>());
static_assert(test_neg_constexpr<signed short>());
static_assert(test_neg_constexpr<signed int>());
static_assert(test_neg_constexpr<signed long>());
static_assert(test_neg_constexpr<signed long long>());

template <typename T, typename U>
struct test_shl_constexpr {
    constexpr bool operator()() const
    {
        constexpr auto& shl = flux::num::shl;

        constexpr auto width = sizeof(T) * CHAR_BIT;
        constexpr T zero = T{0};
        constexpr T min = std::numeric_limits<T>::lowest();
        constexpr T max = std::numeric_limits<T>::max();

        STATIC_CHECK(shl(T{1}, U{0}) == T{1});
        STATIC_CHECK(shl(T{1}, U{1}) == T{2});
        STATIC_CHECK(shl(T{1}, U{2}) == T{4});

        // Shifts greater than width are an error
        STATIC_CHECK(not CONSTEXPR_CALLABLE(shl(T{1}, U{width + 1})));

        // Negative shifts are an error
        if constexpr (flux::num::signed_integral<U>) {
            STATIC_CHECK(not CONSTEXPR_CALLABLE(shl(T{1}, U{-1})));
        }

        if constexpr (flux::num::signed_integral<T>) {
            STATIC_CHECK(shl(T{1}, U{width - 1}) == min);
            STATIC_CHECK(shl(min, U{1}) == zero);
        } else {
            STATIC_CHECK(shl(T{1}, U{width - 1}) == T{1} + max / T{2});
        }

        return true;
    }
};

template <template <typename...> class Fn, typename T0, typename... Types>
constexpr bool test_helper0()
{
    return (Fn<T0, Types>{}() && ...);
}

template <template <typename...> typename Fn, typename... Types>
constexpr bool test_helper1()
{
    return (test_helper0<Fn, Types, Types...>() && ...);
}

constexpr bool do_test_shl_constexpr()
{
    return test_helper1<test_shl_constexpr,
                        signed char, unsigned char,
                        signed short, unsigned short,
                        signed int, unsigned int,
                        signed long, unsigned long,
                        signed long long, unsigned long long>();
}
static_assert(do_test_shl_constexpr());

template <typename T, typename U>
struct test_shr_constexpr {
    constexpr bool operator()() const
    {
        constexpr auto& shr = flux::num::shr;

        constexpr auto width = sizeof(T) * CHAR_BIT;
        constexpr T zero = T{0};
        constexpr T min = std::numeric_limits<T>::lowest();
        constexpr T max = std::numeric_limits<T>::max();

        STATIC_CHECK(shr(max, U{1}) == max / T{2});
        STATIC_CHECK(shr(max, U{2}) == max / T{4});
        STATIC_CHECK(shr(max, U{3}) == max / T{8});

        // Shifts greater than width are an error
        STATIC_CHECK(not CONSTEXPR_CALLABLE(shr(T{1}, U{width + 1})));

        // Negative shifts are an error
        if constexpr (flux::num::signed_integral<U>) {
            STATIC_CHECK(not CONSTEXPR_CALLABLE(shr(T{1}, U{-1})));
        }

        if constexpr (flux::num::unsigned_integral<T>) {
            STATIC_CHECK(shr(max, U{width - 1}) == T{1});
        } else {
            STATIC_CHECK(shr(max, U{width - 1}) == zero);

            STATIC_CHECK(shr(min, U{1}) == min / T{2});
            STATIC_CHECK(shr(min, U{2}) == min / T{4});
            STATIC_CHECK(shr(min, U{3}) == min / T{8});

            STATIC_CHECK(shr(min, U{width - 1}) == T{-1});
        }

        return true;
    }
};

constexpr bool do_test_shr_constexpr()
{
    return test_helper1<test_shr_constexpr,
                        signed char, unsigned char,
                        signed short, unsigned short,
                        signed int, unsigned int,
                        signed long, unsigned long,
                        signed long long, unsigned long long>();
}
static_assert(do_test_shr_constexpr());


/*
 * Runtime tests
 */

template <typename T>
void test_add_runtime()
{
    constexpr auto& add = flux::num::add;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();


    // adding zero to anything doesn't change it, and doesn't overflow
    REQUIRE(add(zero, zero) == zero);
    REQUIRE(add(min, zero) == min);
    REQUIRE(add(zero, min) == min);
    REQUIRE(add(max, zero));
    REQUIRE(add(zero, max) == max);

    // add(max, one) overflows
    REQUIRE_THROWS_AS(add(max, one), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(add(one, max), flux::unrecoverable_error);

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        // "adding" minus one to min overflows, and can't be constexpr-called
        REQUIRE_THROWS_AS(add(min, minus_one), flux::unrecoverable_error);
        REQUIRE_THROWS_AS(add(minus_one, min), flux::unrecoverable_error);

        // adding min and max gives -1, and doesn't overflow
        REQUIRE(add(min, max) == minus_one);
        REQUIRE(add(max, min) == minus_one);
    }
}

template <typename T>
void test_sub_runtime()
{
    constexpr auto& sub = flux::num::sub;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Anything minus zero is itself, and doesn't overflow
    REQUIRE(sub(zero, zero) == zero);
    REQUIRE(sub(one, zero) == one);
    REQUIRE(sub(min, zero) == min);

    // Anything minus itself is zero, and doesn't overflow
    REQUIRE(sub(one, one) == zero);
    REQUIRE(sub(max, max) == zero);
    REQUIRE(sub(min, min) == zero);

    // min minus a positive value overflows, and can't be used in constexpr
    REQUIRE_THROWS_AS(sub(min, one), flux::unrecoverable_error);

    // min minus max overflows
    REQUIRE_THROWS_AS(sub(min, max), flux::unrecoverable_error);

    // max minus a positive value does not overflow
    REQUIRE(sub(max, one) > zero);

    // max minus min is fine for unsigned, overflows for signed
    if constexpr (not flux::num::signed_integral<T>) {
        REQUIRE(sub(max, min) == max);
    } else {
        REQUIRE_THROWS_AS(sub(max, min), flux::unrecoverable_error);
    }

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        REQUIRE(sub(minus_one, minus_one) == zero);

        REQUIRE(sub(minus_one, min) == max);

        REQUIRE(sub(minus_one, max) == min);

        REQUIRE(sub(min, minus_one) < zero);

        REQUIRE_THROWS_AS(sub(max, minus_one), flux::unrecoverable_error);
    }
}

template <typename T>
void test_mul_runtime()
{
    constexpr auto& mul = flux::num::mul;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Anything times zero is zero, and doesn't overflow
    REQUIRE(mul(zero, zero) == zero);
    REQUIRE(mul(zero, one)  == zero);
    REQUIRE(mul(one, zero)  == zero);
    REQUIRE(mul(zero, min)  == zero);
    REQUIRE(mul(min, zero)  == zero);
    REQUIRE(mul(zero, max)  == zero);
    REQUIRE(mul(max, zero)  == zero);

    // Anything times one is itself, and doesn't overflow
    REQUIRE(mul(one, one) == one);
    REQUIRE(mul(min, one) == min);
    REQUIRE(mul(one, min) == min);
    REQUIRE(mul(one, max) == max);
    REQUIRE(mul(max, one) == max);

    // max squared overflows
    REQUIRE_THROWS_AS(mul(max, max), flux::unrecoverable_error);

    // min squared is min on unsigned, overflows on signed
    if constexpr (flux::num::unsigned_integral<T>) {
        REQUIRE(mul(min, min) == min);
    } else {
        REQUIRE_THROWS_AS(mul(min, min), flux::unrecoverable_error);
    }

    // min times max overflows on signed
    if constexpr (flux::num::unsigned_integral<T>) {
        REQUIRE(mul(min, max) == min);
        REQUIRE(mul(max, min) == min);
    } else {
        REQUIRE_THROWS_AS(mul(min, max), flux::unrecoverable_error);
        REQUIRE_THROWS_AS(mul(max, min), flux::unrecoverable_error);
    }

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        REQUIRE(mul(minus_one, one) == minus_one);
        REQUIRE(mul(one, minus_one) == minus_one);
        REQUIRE(mul(minus_one, minus_one) == one);

        REQUIRE(mul(minus_one, max) == min + one);

        REQUIRE_THROWS_AS(mul(min, minus_one), flux::unrecoverable_error);
        REQUIRE_THROWS_AS(mul(minus_one, min), flux::unrecoverable_error);
    }
}

template <typename T>
void test_div_runtime()
{
    constexpr auto& div = flux::num::div;

    constexpr T zero{0};
    constexpr T one{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(div(T{10}, T{5}) == T{2});

    // zero divided by anything is zero
    REQUIRE(div(zero, one) == zero);
    REQUIRE(div(zero, max) == zero);

    // one divided by one is one (test this once)
    REQUIRE(div(one, one) == one);

    // anything divided by one is unchanged
    REQUIRE(div(max, one) == max);
    REQUIRE(div(min, one) == min);

    // dividing by zero is an error
    REQUIRE_THROWS_AS(div(one, zero), flux::unrecoverable_error);

    // For signed type, dividing by -1 negates (except for the lowest value)
    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one{-1};
        REQUIRE(div(one, minus_one) == minus_one);
        REQUIRE(div(minus_one, one) == minus_one);
        REQUIRE(div(minus_one, minus_one) == one);
        REQUIRE(div(max, minus_one) == T{min + 1});

        // min/-1 overflows
        REQUIRE_THROWS_AS(div(min, minus_one), flux::unrecoverable_error);
    }
}

template <typename T>
void test_mod_runtime()
{
    constexpr auto& mod = flux::num::mod;

    constexpr T zero{0};
    constexpr T one{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // 0 % x == 0 for all x
    REQUIRE(mod(zero, one) == zero);
    REQUIRE(mod(zero, T{2}) == zero);
    REQUIRE(mod(zero, max) == zero);

    // x % 1 == 0 for all x
    REQUIRE(mod(one, one) == zero);
    REQUIRE(mod(T{2}, one) == zero);
    REQUIRE(mod(min, one) == zero);
    REQUIRE(mod(max, one) == zero);

    // x % max == x for all x < max
    REQUIRE(mod(one, max) == one);
    REQUIRE(mod(T{2}, max) == T{2});
    REQUIRE(mod(max, max) == zero);

    // x % 0 is an error
    REQUIRE_THROWS_AS(mod(one, zero), flux::unrecoverable_error);

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};
        // modulus with negative numbers makes my head hurt :(
        REQUIRE(mod(one, minus_one) == zero);
        REQUIRE(mod(minus_one, minus_one) == zero);
        REQUIRE(mod(minus_one, one) == zero);
        REQUIRE(mod(T{-3}, T{2}) == minus_one);

        // This should be an error
        REQUIRE_THROWS_AS(mod(min, minus_one), flux::unrecoverable_error);
    }
};

template <typename T>
void test_neg_runtime()
{
    constexpr auto& neg = flux::num::neg;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T minus_one = T{-1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Only neg(min) overflows
    REQUIRE(neg(zero) == zero);
    REQUIRE(neg(one) == minus_one);
    REQUIRE(neg(minus_one) == one);
    REQUIRE(neg(max) == min + 1);

    REQUIRE_THROWS_AS(neg(min), flux::unrecoverable_error);
}

template <typename T, typename U>
struct test_shl_runtime {
    bool operator()() const
    {
        constexpr auto& shl = flux::num::checked_shl;

        constexpr auto width = sizeof(T) * CHAR_BIT;
        constexpr T zero = T{0};
        constexpr T min = std::numeric_limits<T>::lowest();
        constexpr T max = std::numeric_limits<T>::max();

        REQUIRE(shl(T{1}, U{0}) == T{1});
        REQUIRE(shl(T{1}, U{1}) == T{2});
        REQUIRE(shl(T{1}, U{2}) == T{4});

        // Shifts greater than width are an error
        REQUIRE_THROWS_AS(shl(T{1}, U{width + 1}), flux::unrecoverable_error);

        // Negative shifts are an error
        if constexpr (flux::num::signed_integral<U>) {
            REQUIRE_THROWS_AS(shl(T{1}, U{-1}), flux::unrecoverable_error);
        }

        if constexpr (flux::num::signed_integral<T>) {
            REQUIRE(shl(T{1}, U{width - 1}) == min);
            REQUIRE(shl(min, U{1}) == zero);
        } else {
            REQUIRE(shl(T{1}, U{width - 1}) == T{1} + max / T{2});
        }

        return true;
    }
};

template <typename T, typename U>
struct test_shr_runtime {
    bool operator()() const
    {
        constexpr auto& shr = flux::num::checked_shr;

        constexpr auto width = sizeof(T) * CHAR_BIT;
        constexpr T zero = T{0};
        constexpr T min = std::numeric_limits<T>::lowest();
        constexpr T max = std::numeric_limits<T>::max();

        REQUIRE(shr(max, U{1}) == max / T{2});
        REQUIRE(shr(max, U{2}) == max / T{4});
        REQUIRE(shr(max, U{3}) == max / T{8});

        // Shifts greater than width are an error
        REQUIRE_THROWS_AS(shr(T{1}, U{width + 1}), flux::unrecoverable_error);

        // Negative shifts are an error
        if constexpr (flux::num::signed_integral<U>) {
            REQUIRE_THROWS_AS(shr(T{1}, U{-1}), flux::unrecoverable_error);
        }

        if constexpr (flux::num::unsigned_integral<T>) {
            REQUIRE(shr(max, U{width - 1}) == T{1});
        } else {
            REQUIRE(shr(max, U{width - 1}) == zero);

            REQUIRE(shr(min, U{1}) == min / T{2});
            REQUIRE(shr(min, U{2}) == min / T{4});
            REQUIRE(shr(min, U{3}) == min / T{8});

            REQUIRE(shr(min, U{width - 1}) == T{-1});
        }

        return true;
    }
};

}

TEST_CASE("num.add")
{
    test_add_runtime<signed char>();
    test_add_runtime<unsigned char>();
    test_add_runtime<signed short>();
    test_add_runtime<unsigned short>();
    test_add_runtime<signed int>();
    test_add_runtime<unsigned int>();
    test_add_runtime<signed long>();
    test_add_runtime<unsigned long>();
    test_add_runtime<signed long long>();
    test_add_runtime<unsigned long long>();
}

TEST_CASE("num.sub")
{
    test_sub_runtime<signed char>();
    test_sub_runtime<unsigned char>();
    test_sub_runtime<signed short>();
    test_sub_runtime<unsigned short>();
    test_sub_runtime<signed int>();
    test_sub_runtime<unsigned int>();
    test_sub_runtime<signed long>();
    test_sub_runtime<unsigned long>();
    test_sub_runtime<signed long long>();
    test_sub_runtime<unsigned long long>();
}

TEST_CASE("num.mul")
{
    test_mul_runtime<signed char>();
    test_mul_runtime<unsigned char>();
    test_mul_runtime<signed short>();
    test_mul_runtime<unsigned short>();
    test_mul_runtime<signed int>();
    test_mul_runtime<unsigned int>();
    test_mul_runtime<signed long>();
    test_mul_runtime<unsigned long>();
    test_mul_runtime<signed long long>();
    test_mul_runtime<unsigned long long>();
}

TEST_CASE("num.div")
{
    test_div_runtime<signed char>();
    test_div_runtime<unsigned char>();
    test_div_runtime<signed short>();
    test_div_runtime<unsigned short>();
    test_div_runtime<signed int>();
    test_div_runtime<unsigned int>();
    test_div_runtime<signed long>();
    test_div_runtime<unsigned long>();
    test_div_runtime<signed long long>();
    test_div_runtime<unsigned long long>();
}

TEST_CASE("num.mod")
{
    test_mod_runtime<signed char>();
    test_mod_runtime<unsigned char>();
    test_mod_runtime<signed short>();
    test_mod_runtime<unsigned short>();
    test_mod_runtime<signed int>();
    test_mod_runtime<unsigned int>();
    test_mod_runtime<signed long>();
    test_mod_runtime<unsigned long>();
    test_mod_runtime<signed long long>();
    test_mod_runtime<unsigned long long>();
}

TEST_CASE("num.neg")
{
    test_neg_runtime<signed char>();
    test_neg_runtime<signed short>();
    test_neg_runtime<signed int>();
    test_neg_runtime<signed long>();
    test_neg_runtime<signed long long>();
}

TEST_CASE("num.shl")
{
    test_helper1<test_shl_runtime,
                 signed char, unsigned char,
                 signed short, unsigned short,
                 signed int, unsigned int,
                 signed long, unsigned long,
                 signed long long, unsigned long long>();
}

TEST_CASE("num.shr")
{
    test_helper1<test_shr_runtime,
                 signed char, unsigned char,
                 signed short, unsigned short,
                 signed int, unsigned int,
                 signed long, unsigned long,
                 signed long long, unsigned long long>();
}

