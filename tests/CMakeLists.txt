find_package(cryptopp CONFIG REQUIRED)

# The storage layer only, without app.cpp -- the test provides its own main().
add_executable(redux_tests
  regression_tests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/account.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/file.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/input.cpp
)

target_include_directories(redux_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_features(redux_tests PRIVATE cxx_std_17)
target_link_libraries(redux_tests PRIVATE cryptopp::cryptopp)

add_test(NAME redux_regression_tests COMMAND redux_tests)

# The end-of-input cases fail by hanging rather than returning if they ever regress, so cap the
# run instead of letting CI block on it.
set_tests_properties(redux_regression_tests PROPERTIES TIMEOUT 120)
