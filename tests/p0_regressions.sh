#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

fail() {
  printf 'FAIL: %s\n' "$1" >&2
  exit 1
}

rg -q 'first == std::string::npos' src/input.cpp \
  || fail "input trim should handle all-whitespace strings before slicing"

rg -q 'getenv\("HOME"\)' src/file.cpp \
  || fail "non-Windows user data path should be based on HOME"

rg -q '"/home/"' src/file.cpp \
  && fail "non-Windows user data path still hard-codes /home"

rg -q 'branches: \[ master \]' .github/workflows/codeql-analysis.yml \
  && fail "CodeQL workflow still targets master instead of main"

rg -q 'actions/checkout@v2|actions/upload-artifact@v2|github/codeql-action/(init|analyze)@v1' .github/workflows \
  && fail "GitHub workflows still use deprecated v1/v2 actions"

rg -q 'runs-on: (ubuntu-20\.04|macos-11|macos-13)' .github/workflows \
  && fail "GitHub workflows still use old runner images"

rg -q 'runs-on: macos-26$' .github/workflows/macOS-release.yml \
  || fail "macOS workflow should run on macos-26"

rg -q 'cryptopp:arm64-osx' .github/workflows/macOS-release.yml \
  || fail "macOS workflow should install Crypto++ for arm64-osx"

rg -q 'cryptopp::cryptopp' src/CMakeLists.txt \
  || fail "CMake should link the vcpkg-provided cryptopp::cryptopp target"

printf 'P0 regression checks passed.\n'
