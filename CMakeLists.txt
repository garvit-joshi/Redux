cmake_minimum_required(VERSION 3.22)

project(
  Redux
  VERSION 1.11
  DESCRIPTION "A credential storage with encryption."
  LANGUAGES CXX
)

add_subdirectory(src)

# The regression tests relocate the config directory by overriding HOME, which is how the POSIX
# build resolves it. The Windows build uses GetUserProfileDirectory instead, so they are not
# portable as written.
if(NOT WIN32)
  enable_testing()
  add_subdirectory(tests)
endif()
