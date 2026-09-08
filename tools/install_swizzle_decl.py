#!/usr/bin/env python3
"""Install the generated swizzle macro header into the include tree.

The generator writes ``tools/vec_swizzle_decl.hpp``; this script moves that
file to ``include/eirin/detail/vec_swizzle_decl.hpp`` so the library can pick
it up.  Run ``tools/gen_swizzle_macros.py`` first.
"""

from __future__ import annotations

import shutil
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
SOURCE = SCRIPT_DIR / "vec_swizzle_decl.hpp"
TARGET = SCRIPT_DIR.parent / "include" / "eirin" / "detail" / "vec_swizzle_decl.hpp"


def main() -> None:
    """Move tools/vec_swizzle_decl.hpp into include/eirin/detail."""
    if not SOURCE.exists():
        raise SystemExit(f"{SOURCE} not found; run tools/gen_swizzle_macros.py first")

    TARGET.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(SOURCE), str(TARGET))
    print(f"moved {SOURCE}")
    print(f"  -> {TARGET}")


if __name__ == "__main__":
    main()
