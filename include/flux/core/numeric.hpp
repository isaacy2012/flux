
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_NUMERIC_HPP_INCLUDED
#define FLUX_CORE_NUMERIC_HPP_INCLUDED

#include <flux/core/assert.hpp>

#include <limits>

namespace flux::num {

FLUX_EXPORT
inline constexpr auto wrapping_add = []<std::signed_integral T>(T lhs, T rhs) -> T
{
    using U = std::make_unsigned_t<T>;
    return static_cast<T>(static_cast<U>(lhs) + static_cast<U>(rhs));
};

FLUX_EXPORT
inline constexpr auto wrapping_sub = []<std::signed_integral T>(T lhs, T rhs) -> T
{
    using U = std::make_unsigned_t<T>;
    return static_cast<T>(static_cast<U>(lhs) - static_cast<U>(rhs));
};

FLUX_EXPORT
inline constexpr auto wrapping_mul = []<std::signed_integral T>(T lhs, T rhs) -> T
{
    using U = std::make_unsigned_t<T>;
    return static_cast<T>(static_cast<U>(lhs) * static_cast<U>(rhs));
};

namespace detail {

#if defined(__has_builtin)
#  if __has_builtin(__builtin_add_overflow) && \
      __has_builtin(__builtin_sub_overflow) && \
      __has_builtin(__builtin_mul_overflow)
#    define FLUX_HAVE_BUILTIN_OVERFLOW_OPS 1
#  endif
#endif

#ifdef FLUX_HAVE_BUILTIN_OVERFLOW_OPS
    inline constexpr bool use_builtin_overflow_ops = true;
#else
    inline constexpr bool use_builtin_overflow_ops = false;
#endif

#undef FLUX_HAVE_BUILTIN_OVERFLOW_OPS

}

FLUX_EXPORT
template <std::signed_integral T>
struct overflow_result {
    T value;
    bool overflowed;
};

FLUX_EXPORT
inline constexpr auto overflowing_add = []<std::signed_integral T>(T lhs, T rhs)
    -> overflow_result<T>
{
    if constexpr (detail::use_builtin_overflow_ops) {
        bool overflowed = __builtin_add_overflow(lhs, rhs, &lhs);
        return {lhs, overflowed};
    } else {
        T value = wrapping_add(lhs, rhs);
        bool overflowed = ((lhs < T{}) == (rhs < T{})) && ((lhs < T{}) != (value < T{}));
        return {value, overflowed};
    }
};

FLUX_EXPORT
inline constexpr auto overflowing_sub = []<std::signed_integral T>(T lhs, T rhs)
    -> overflow_result<T>
{
    if constexpr (detail::use_builtin_overflow_ops) {
        bool overflowed = __builtin_sub_overflow(lhs, rhs, &lhs);
        return {lhs, overflowed};
    } else {
        T value = wrapping_sub(lhs, rhs);
        bool overflowed = (lhs >= T{} && rhs < T{} && value < T{}) ||
                         (lhs < T{} && rhs > T{} && value > T{});
        return {value, overflowed};
    }
};

FLUX_EXPORT
inline constexpr auto overflowing_mul = []<std::signed_integral T>(T lhs, T rhs)
    -> overflow_result<T>
{
    if constexpr (detail::use_builtin_overflow_ops) {
        bool overflowed = __builtin_mul_overflow(lhs, rhs, &lhs);
        return {lhs, overflowed};
    } else {
        T value =  wrapping_mul(lhs, rhs);
        return {value, lhs != 0 && value/lhs != rhs};
    }
};

FLUX_EXPORT
inline constexpr auto checked_add =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
    if (std::is_constant_evaluated()) {
        return lhs + rhs;
    } else {
        if constexpr (config::on_overflow == overflow_policy::ignore) {
            return lhs + rhs;
        } else if constexpr (config::on_overflow == overflow_policy::wrap) {
            return wrapping_add(lhs, rhs);
        } else {
            auto res = overflowing_add(lhs, rhs);
            if (res.overflowed) {
                runtime_error("signed overflow in addition", loc);
            }
            return res.value;
        }
    }
};

FLUX_EXPORT
inline constexpr auto checked_sub =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
  if (std::is_constant_evaluated()) {
      return lhs - rhs;
  } else {
      if constexpr (config::on_overflow == overflow_policy::ignore) {
          return lhs - rhs;
      } else if constexpr (config::on_overflow == overflow_policy::wrap) {
          return wrapping_sub(lhs, rhs);
      } else {
          auto res = overflowing_sub(lhs, rhs);
          if (res.overflowed) {
              runtime_error("signed overflow in subtraction", loc);
          }
          return res.value;
      }
  }
};

FLUX_EXPORT
inline constexpr auto checked_mul =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
  if (std::is_constant_evaluated()) {
      return lhs * rhs;
  } else {
      if constexpr (config::on_overflow == overflow_policy::ignore) {
          return lhs * rhs;
      } else if constexpr (config::on_overflow == overflow_policy::wrap) {
          return wrapping_mul(lhs, rhs);
      } else {
          auto res = overflowing_mul(lhs, rhs);
          if (res.overflowed) {
              runtime_error("signed overflow in multiplication", loc);
          }
          return res.value;
      }
  }
};

namespace detail {
template<std::signed_integral... Ts>
inline constexpr auto variadic_checked_mul_impl(Ts... ts);

template<std::signed_integral T1, std::signed_integral T2>
inline constexpr auto variadic_checked_mul_impl(T1 lhs, T2 rhs) {
    return checked_mul(lhs, rhs);
};

template<std::signed_integral T1, std::signed_integral T2, std::signed_integral... Ts>
inline constexpr auto variadic_checked_mul_impl(T1 lhs, T2 rhs, Ts... ts) {
    return variadic_checked_mul_impl(
        variadic_checked_mul_impl<T1, T2>(lhs, rhs), ts...);
};

}

template<std::signed_integral T1, std::signed_integral T2, std::signed_integral... Ts>
FLUX_EXPORT
inline constexpr auto variadic_checked_mul(T1 lhs, T2 rhs, Ts... ts) {
    return detail::variadic_checked_mul_impl(lhs, rhs, ts...);
};


FLUX_EXPORT
inline constexpr auto checked_pow =
        []<std::signed_integral T, std::unsigned_integral U>(T base, U exponent,
                                   std::source_location loc = std::source_location::current())
                -> T
{
    T res{1};
    for(U i{0}; i < exponent; i++) {
        res = checked_mul(res, base, loc);
    }
    return res;
};


inline constexpr auto checked_div =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
    if (std::is_constant_evaluated()) {
        return lhs / rhs;
    } else {
        if constexpr (config::on_divide_by_zero == divide_by_zero_policy::ignore) {
            return lhs / rhs;
        } else {
            if (rhs == 0) {
                runtime_error("divide by zero", loc);
            }
            return lhs / rhs;
        }
    }
};

inline constexpr auto checked_mod =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
    if (std::is_constant_evaluated()) {
        return lhs % rhs;
    } else {
        if constexpr (config::on_divide_by_zero == divide_by_zero_policy::ignore) {
            return lhs % rhs;
        } else {
            if (rhs == 0) {
                runtime_error("divide by zero", loc);
            }
            return lhs % rhs;
        }
    }
};

} // namespace flux::num

#endif
