#!/bin/bash

set -euo pipefail

# Resolve paths relative to this script so it can be run from anywhere.
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

formatter=""

find_clang_format()
{
    local name="$1"

    if [[ -n "$formatter" ]]; then
        return
    fi

    if type $name >/dev/null 2>&1; then
        echo "find clang-format: $name."
        formatter=$name
    else
        echo "can not find $name."
    fi
}

find_clang_format "clang-format"
find_clang_format "clang-format-13"
find_clang_format "clang-format-14"
find_clang_format "clang-format-15"
find_clang_format "clang-format-16"
find_clang_format "clang-format-17"
find_clang_format "clang-format-18"
find_clang_format "clang-format-19"
find_clang_format "clang-format-20"
find_clang_format "clang-format-21"
find_clang_format "clang-format-22"

if type python3 >/dev/null 2>&1; then
    python3 ./gen_swizzle_macros.py

    if [[ -n "$formatter" ]]; then
        $formatter --style=file:../.clang-format ./vec_swizzle_decl.hpp > ./tmp.vec_swizzle_decl.hpp
        cat ./tmp.vec_swizzle_decl.hpp > ./vec_swizzle_decl.hpp
        rm ./tmp.vec_swizzle_decl.hpp
    fi

    python3 ./install_swizzle_decl.py
fi
