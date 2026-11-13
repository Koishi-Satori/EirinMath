#!/bin/sh

# This script is used to build the documentation using Doxygen.

DOXYGEN_BIN=$(command -v doxygen)
if [ -z "$DOXYGEN_BIN" ]; then
    echo "Failed to find doxygen, please install it through package manager or manually"
    exit 1
fi

if [ ! -f "./Doxyfile" ]; then
    echo "Doxyfile not found. Please run this script from the root directory of the project."
    exit 1
fi

echo "Build doxygen documentation..."

$DOXYGEN_BIN "./Doxyfile"

SPHINX_BIN=$(command -v sphinx-build)
if [ -z "$SPHINX_BIN" ]; then
    echo "Failed to find sphinx-build, please install it through package manager or manually."
    exit 1
fi
SPHINX_OPT="-b html"
SPHINX_SOURCE_DIR="./"
SPHINX_BUILD_DIR="./_build"
SPHINX_BREATHE_OPT="-Dbreathe_projects.EirinMath=./xml"

echo "Build sphinx documentation..."

exec $SPHINX_BIN $SPHINX_OPT $SPHINX_BREATHE_OPT $SPHINX_SOURCE_DIR $SPHINX_BUILD_DIR
