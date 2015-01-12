project(examples CXX)

include_directories(../src)

add_executable(adafruit adafruit.cpp)
target_link_libraries(adafruit libnavio)

add_executable(navio navio.cpp)
target_link_libraries(adafruit libnavio)
