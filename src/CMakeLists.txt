project(libnavio CXX)

set(libnavio_src
    eventpoller.cpp
    fdevent.cpp
    timer.cpp
    signal.cpp
    log.cpp
    i2c.cpp
    spi.cpp
    bmp180.cpp
    pca9685.cpp
    l3gd20h.cpp
    lsm303dhlc.cpp
)

add_library(libnavio ${libnavio_src})
target_link_libraries(libnavio rt m)
