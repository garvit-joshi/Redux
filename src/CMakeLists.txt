find_package(cryptopp CONFIG REQUIRED)

# The storage layer, split out from the app so tests/CMakeLists.txt can link it directly instead
# of recompiling ../src files of its own -- one source list instead of two kept in sync by hand.
add_library(redux_core STATIC
  account.cpp
  file.cpp
  input.cpp
)
target_include_directories(redux_core PUBLIC ../include)
target_compile_features(redux_core PUBLIC cxx_std_17)
target_link_libraries(redux_core PUBLIC cryptopp::cryptopp)
target_compile_options(redux_core PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

add_executable(Redux
  app.cpp
  after_signin_services.cpp
  feature.cpp
  login.cpp
  logout.cpp
  signup.cpp
  startup_services.cpp
  utils.cpp
)
target_link_libraries(Redux PRIVATE redux_core)
target_compile_options(Redux PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
