file(GLOB app_src
  "*.cpp")

find_package(cryptopp CONFIG REQUIRED)
add_executable(Redux ${app_src})
target_include_directories(Redux PUBLIC ../include)
target_compile_features(Redux PRIVATE cxx_std_17)
target_link_libraries(Redux PRIVATE cryptopp-static)
