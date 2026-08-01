cmake_minimum_required(VERSION 3.22)

project(
  Redux
  VERSION 1.11
  DESCRIPTION "A credential storage with encryption."
  LANGUAGES CXX
)

add_subdirectory(src)

# The regression tests used to relocate the config directory by overriding HOME, which the
# Windows build never consults (it resolves the profile directory via the Win32 API instead), so
# tests were POSIX-only. They now relocate storage via REDUX_CONFIG_DIR, which every platform
# checks first, so the suite runs everywhere.
enable_testing()
add_subdirectory(tests)
