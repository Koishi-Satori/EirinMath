#ifndef EIRIN_MATH_TEST_COMPILE_CHECK_HPP
#define EIRIN_MATH_TEST_COMPILE_CHECK_HPP

#pragma once

#include <eirin/detail/matrix_func.hpp>

// Dependent-context, type-level compile probes for negative compile checks.
//
// A requires-expression written directly inside a non-template test body is a
// hard error whenever any requirement is invalid; it only evaluates to false
// when it is checked through the instantiation of a templated entity.  These
// variable templates move the check into a dependent context, so an expression
// that must not compile becomes a `false` constant that can be asserted at
// namespace scope, e.g.
//
//   static_assert(!eirin::testing::can_assign_v<
//       eirin::swizzle_proxy<4, int, true, 0, 1>,
//       eirin::swizzle_proxy<4, int, true, 0, 1>>);
//
// Only the types of the operands are used (no local objects are named), so the
// probes are safe to instantiate anywhere in a test TU.

namespace eirin
{
namespace testing
{
    /// True iff `lhs = rhs` is a valid expression.
    template <typename L, typename R>
    inline constexpr bool can_assign_v = requires(L lhs, R rhs) { lhs = rhs; };

    // register binary operator call testing concept
#define EIRIN_TESTING_BINARY_ARITH_PROBE(name, op) \
    template <typename L, typename R>              \
    inline constexpr bool name = requires(L lhs, R rhs) { lhs op rhs; };

    EIRIN_TESTING_BINARY_ARITH_PROBE(can_add_v, +)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_sub_v, -)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_mul_v, *)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_div_v, /)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_mod_v, %)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_xor_v, ^)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_and_v, &)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_or_v, |)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_shl_v, <<)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_shr_v, >>)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_equal_v, ==)
    EIRIN_TESTING_BINARY_ARITH_PROBE(can_not_equal_v, !=)

#undef EIRIN_TESTING_BINARY_ARITH_PROBE

    // register single argument function call testing concept
#define EIRIN_TESTING_SINGLE_ARG_CALL_PROBE(name, fn) \
    template <typename T>                             \
    inline constexpr bool name = requires(T value) { fn(value); };

    EIRIN_TESTING_SINGLE_ARG_CALL_PROBE(can_inverse_v, eirin::inverse)
    EIRIN_TESTING_SINGLE_ARG_CALL_PROBE(can_transpose_v, eirin::transpose)
    EIRIN_TESTING_SINGLE_ARG_CALL_PROBE(can_determinant_v, eirin::determinant)

#undef EIRIN_TESTING_SINGLE_ARG_CALL_PROBE

#define EIRIN_TESTING_PREFIX_ARITH_PROBE(name, op) \
    template <typename T>                          \
    inline constexpr bool name = requires(T value) { op value; };

#define EIRIN_TESTING_POSTFIX_ARITH_PROBE(name, op) \
    template <typename T>                           \
    inline constexpr bool name = requires(T value) { value op; };

    EIRIN_TESTING_PREFIX_ARITH_PROBE(can_preincrement_v, ++)
    EIRIN_TESTING_PREFIX_ARITH_PROBE(can_predecrement_v, --)
    EIRIN_TESTING_POSTFIX_ARITH_PROBE(can_postincrement_v, ++)
    EIRIN_TESTING_POSTFIX_ARITH_PROBE(can_postdecrement_v, --)

#undef EIRIN_TESTING_PREFIX_ARITH_PROBE
#undef EIRIN_TESTING_POSTFIX_ARITH_PROBE

    // register compound assign call testing concept
#define EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(name, op) \
    template <typename L, typename R>                 \
    inline constexpr bool name = requires(L lhs, R rhs) { lhs op rhs; };

    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_add_assign_v, +=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_sub_assign_v, -=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_mul_assign_v, *=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_div_assign_v, /=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_mod_assign_v, %=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_xor_assign_v, ^=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_and_assign_v, &=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_or_assign_v, |=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_shl_assign_v, <<=)
    EIRIN_TESTING_COMPOUND_ASSIGN_PROBE(can_shr_assign_v, >>=)

#undef EIRIN_TESTING_COMPOUND_ASSIGN_PROBE
} // namespace testing
} // namespace eirin

// Expression wrappers for use inside test bodies.  Each probe expands to a
// constant that is `true` iff the corresponding expression is well-formed;
// negate it (e.g. with EXPECT_FALSE) to assert that an expression must not
// compile.  The operands are only inspected through decltype, never evaluated.
#define EIRIN_TESTING_COMPILE_ASSIGN(lhs, rhs)     (eirin::testing::can_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_ADD(lhs, rhs)        (eirin::testing::can_add_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_SUB(lhs, rhs)        (eirin::testing::can_sub_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_MUL(lhs, rhs)        (eirin::testing::can_mul_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_DIV(lhs, rhs)        (eirin::testing::can_div_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_MOD(lhs, rhs)        (eirin::testing::can_mod_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_XOR(lhs, rhs)        (eirin::testing::can_xor_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_AND(lhs, rhs)        (eirin::testing::can_and_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_OR(lhs, rhs)         (eirin::testing::can_or_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_SHL(lhs, rhs)        (eirin::testing::can_shl_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_SHR(lhs, rhs)        (eirin::testing::can_shr_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_EQUAL(lhs, rhs)      (eirin::testing::can_equal_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_NOT_EQUAL(lhs, rhs)  (eirin::testing::can_not_equal_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_INVERSE(expr)        (eirin::testing::can_inverse_v<decltype(expr)>)
#define EIRIN_TESTING_COMPILE_TRANSPOSE(expr)      (eirin::testing::can_transpose_v<decltype(expr)>)
#define EIRIN_TESTING_COMPILE_DETERMINANT(expr)    (eirin::testing::can_determinant_v<decltype(expr)>)
#define EIRIN_TESTING_COMPILE_ADD_ASSIGN(lhs, rhs) (eirin::testing::can_add_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_SUB_ASSIGN(lhs, rhs) (eirin::testing::can_sub_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_MUL_ASSIGN(lhs, rhs) (eirin::testing::can_mul_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_DIV_ASSIGN(lhs, rhs) (eirin::testing::can_div_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_MOD_ASSIGN(lhs, rhs) (eirin::testing::can_mod_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_XOR_ASSIGN(lhs, rhs) (eirin::testing::can_xor_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_AND_ASSIGN(lhs, rhs) (eirin::testing::can_and_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_OR_ASSIGN(lhs, rhs)  (eirin::testing::can_or_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_SHL_ASSIGN(lhs, rhs) (eirin::testing::can_shl_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_SHR_ASSIGN(lhs, rhs) (eirin::testing::can_shr_assign_v<decltype(lhs), decltype(rhs)>)
#define EIRIN_TESTING_COMPILE_PRE_INCR(lhs)        (eirin::testing::can_preincrement_v<decltype(lhs)>)
#define EIRIN_TESTING_COMPILE_POST_INCR(lhs)       (eirin::testing::can_postincrement_v<decltype(lhs)>)
#define EIRIN_TESTING_COMPILE_PRE_DECR(lhs)        (eirin::testing::can_predecrement_v<decltype(lhs)>)
#define EIRIN_TESTING_COMPILE_POST_DECR(lhs)       (eirin::testing::can_postdecrement_v<decltype(lhs)>)

#endif // EIRIN_MATH_TEST_COMPILE_CHECK_HPP
