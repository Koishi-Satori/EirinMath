#include <eirin/fixed.hpp>

#pragma once

#define F32_FROM_BENCH(idx) fixed32::from_internal_value(state.range(idx))
#define BENCH_F32_VAL(fp)   (operator""_f32(#fp, strlen(#fp))).internal_value()

// prevent compiler optimization.
#ifdef EIRIN_MATH_HAS_INT128
#    define F64_FROM_BENCH(idx) fixed64::from_internal_value(state.range(idx))
#    define BENCH_F64_VAL(fp)   (operator""_f64(#fp, strlen(#fp))).internal_value()
#endif

#define DOUBLE_FROM_BENCH(idx) static_cast<double>(state.range(idx)) / 1000.0
#define BENCH_DOUBLE_VAL(db)   static_cast<int64_t>(db * 1000.0)

double db_identity(double val);
