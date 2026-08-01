# The storage layer only, without app.cpp -- the test provides its own main(). Links redux_core
# (defined in src/CMakeLists.txt) rather than recompiling ../src files of its own, so there is one
# source list for the storage layer, not two kept in sync by hand.
add_executable(redux_tests
  regression_tests.cpp
)
target_link_libraries(redux_tests PRIVATE redux_core)

add_test(NAME redux_regression_tests COMMAND redux_tests)

# The end-of-input cases fail by hanging rather than returning if they ever regress, so cap the
# run instead of letting CI block on it.
set_tests_properties(redux_regression_tests PROPERTIES TIMEOUT 120)

# p0_regressions.sh greps source and workflow files for specific fixes rather than exercising
# built code, so it needs bash and standard POSIX tools -- not available on the Windows runners.
if(NOT WIN32)
  add_test(NAME redux_p0_regressions COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/p0_regressions.sh)
endif()
