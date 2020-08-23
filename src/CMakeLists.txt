file(GLOB app_src
  "*.cpp")

add_executable(app ${app_src})

target_compile_features(app PRIVATE cxx_std_17)

target_include_directories(app PUBLIC ../include)

target_link_libraries(app PRIVATE cryptopp)
