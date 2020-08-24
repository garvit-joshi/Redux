file(GLOB app_src
  "*.cpp")

find_package(cryptopp CONFIG REQUIRED)
add_executable(app ${app_src})
target_include_directories(app PUBLIC ../include)
target_compile_features(app PRIVATE cxx_std_17)
target_link_libraries(app PRIVATE cryptopp-static)
