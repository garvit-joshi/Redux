#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

fail() {
  printf 'FAIL: %s\n' "$1" >&2
  exit 1
}

grep -qE 'first == std::string::npos' src/input.cpp \
  || fail "input trim should handle all-whitespace strings before slicing"

grep -qE 'getenv\("HOME"\)' src/file.cpp \
  || fail "non-Windows user data path should be based on HOME"

grep -qE '"/home/"' src/file.cpp \
  && fail "non-Windows user data path still hard-codes /home"

grep -qE 'branches: \[ master \]' .github/workflows/codeql-analysis.yml \
  && fail "CodeQL workflow still targets master instead of main"

grep -rqE 'actions/checkout@v2|actions/upload-artifact@v2|github/codeql-action/(init|analyze)@v1' .github/workflows \
  && fail "GitHub workflows still use deprecated v1/v2 actions"

grep -rqE 'runs-on: (ubuntu-20\.04|macos-11|macos-13)' .github/workflows \
  && fail "GitHub workflows still use old runner images"

grep -qE 'runs-on: macos-26$' .github/workflows/macOS-release.yml \
  || fail "macOS workflow should run on macos-26"

grep -qE 'cryptopp::cryptopp' src/CMakeLists.txt \
  || fail "CMake should link the vcpkg-provided cryptopp::cryptopp target"

# vcpkg dependencies are now pinned through the manifest (vcpkg.json) rather than an ad-hoc
# `vcpkg install` step per workflow, so Crypto++ provisioning is verified there instead of by
# grepping for a triplet in one platform's workflow.
[ -f vcpkg.json ] \
  || fail "vcpkg.json manifest is missing"

grep -qE '"cryptopp"' vcpkg.json \
  || fail "vcpkg.json should declare cryptopp as a dependency"

grep -rqE 'vcpkg install cryptopp' .github/workflows \
  && fail "workflows should rely on the vcpkg manifest instead of an ad-hoc vcpkg install step"

# Every action reference must name a concrete release (vX.Y.Z), never a movable major tag like
# @v4: a bare major tag silently changes what runs whenever upstream repoints it, while a
# concrete version only changes when this repo deliberately edits it.
if grep -rhE 'uses:.*@' .github/workflows | grep -vqE '@v[0-9]+\.[0-9]+\.[0-9]+$'; then
  fail "every workflow 'uses:' line must name a concrete release version (vX.Y.Z)"
fi

printf 'P0 regression checks passed.\n'
