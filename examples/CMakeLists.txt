project(examples CXX)

include_directories(../src)

add_executable(bmp180 bmp180.cpp)
target_link_libraries(bmp180 libnavio)

add_executable(pca9685 pca9685.cpp)
target_link_libraries(pca9685 libnavio)

add_executable(l3gd20h l3gd20h.cpp)
target_link_libraries(l3gd20h libnavio)
