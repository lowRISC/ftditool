
// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <span>
#include <variant>
#include <optional>
#include <cstdint>

namespace embeddedpp {

using Data = std::span<uint8_t>;

enum class SpiDirection {
  Write,
  Read,
  Both,
};

enum class SpiIoMode {
  Single,
  Dual,
  Quad,
};

struct Transfer {
  SpiIoMode mode;
  SpiDirection direction;
  Data data;
};

using Transfers = std::span<Transfer>;

enum class Code {
  Ok,
  SpiMultiModeError,  // Dual and Quad modes not suported in the same transaction.
  Generic,
};

template <typename T>
using Result = std::variant<T, Code>;
using Status = Result<int>;

/**
 * @brief Returns whether the given Result contains an error.
 *
 * A Result is considered an error if it currently holds a value of type `Code`.
 *
 * @tparam T The success value type stored in the Result.
 * @param res The Result to inspect.
 * @return true if the Result contains a Code error, false otherwise.
 *
 * @note This function does not modify the Result.
 */
template <typename T>
bool is_error(Result<T> res) {
  return std::holds_alternative<Code>(res);
}

/**
 * @brief Extracts the success value from a Result.
 *
 * @tparam T The success value type stored in the Result.
 * @param res The Result to unwrap.
 * @return The contained success value.
 *
 * @pre The Result must not contain an error.
 *
 * @warning If `res` contains a Code error, this function throws
 * `std::bad_variant_access`.
 */
template <typename T>
T unwrap(Result<T> res) {
  return std::get<T>(res);
}

/**
 * @brief Extracts the success value from a Result or returns a fallback value on error.
 *
 * @tparam T The success value type stored in the Result.
 * @param res The Result to unwrap.
 * @param value The value to return if the Result contains an error.
 * @return The contained success value, or `value` if `res` contains an error.
 */
template <typename T>
T unwrap_or(Result<T> res, T value) {
  if (is_error(res)) {
    return value;
  }
  return std::get<T>(res);
}

/**
 * @brief Converts a Result into an std::optional.
 *
 * @tparam T The success value type stored in the Result.
 * @param res The Result to convert.
 * @return std::optional containing the success value if present, or std::nullopt if `res`
 * is an error.
 */
template <typename T>
std::optional<T> to_optional(Result<T> res) {
  if (is_error(res)) {
    return std::nullopt;
  }
  return std::get<T>(res);
}

/**
 * @brief Evaluates an expression returning a Result and propagates errors automatically.
 *
 * Evaluates `expr_`, which must return a `Result<T>`. If it contains an error,
 * the current function returns immediately with that error. Otherwise, the
 * success value is extracted and returned as the value of the macro expression.
 *
 * This enables concise error propagation similar to Rust's `?` operator.
 *
 * Example:
 * @code
 * Result<int> parse();
 * Result<int> compute() {
 *     int x = TRY(parse());
 *     return x + 1;
 * }
 * @endcode
 *
 * @param expr_ An expression yielding a Result<T>.
 * @return The unwrapped success value of expr_.
 *
 * @note This macro uses a GCC/Clang statement-expression extension and is not portable.
 */
#define TRY(expr_)                                                                                 \
  ({                                                                                               \
    auto val = (expr_);                                                                            \
    if (is_error(val)) {                                                                           \
      return val;                                                                                  \
    }                                                                                              \
    unwrap(val);                                                                                   \
  })

/**
 * @brief Like TRY(), but propagates errors by returning std::nullopt.
 *
 * Evaluates `expr_`, which must return a `Result<T>`. If it contains an error,
 * the current function returns `std::nullopt`. Otherwise, the contained success
 * value is extracted and returned.
 *
 * Example:
 * @code
 * std::optional<int> parse_opt();
 * std::optional<int> compute_opt() {
 *     int x = TRY_OPT(parse());
 *     return x + 1;
 * }
 * @endcode
 *
 * @param expr_ An expression yielding a Result<T>.
 * @return The unwrapped success value of expr_.
 *
 * @note This macro uses a GCC/Clang statement-expression extension and is not portable.
 */
#define TRY_OPT(expr_)                                                                             \
  ({                                                                                               \
    auto val         = (expr_);                                                                    \
    using ReturnType = std::variant_alternative_t<0, decltype(val)>;                               \
    ReturnType result; /* Declare a variable to hold the final result */                           \
    if (is_error(val)) {                                                                           \
      if (std::get<embeddedpp::Code>(val) != embeddedpp::Code::Ok) {                               \
        return std::nullopt;                                                                       \
      }                                                                                            \
    } else {                                                                                       \
      result = unwrap(val);                                                                        \
    }                                                                                              \
    result;                                                                                        \
  })
}  // namespace embeddedpp
