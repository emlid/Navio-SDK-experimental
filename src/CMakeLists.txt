project(libnavio CXX)

set(libnavio_src
    application.cpp
    poller.cpp
    descriptor.cpp
    timer.cpp
    signal.cpp
    log.cpp
    i2c.cpp
    spi.cpp
    utils.cpp
    bmp180.cpp
    pca9685.cpp
    l3gd20h.cpp
    lsm303dhlc.cpp
    ssd1306.cpp
    ads1115.cpp
)

add_library(libnavio ${libnavio_src})
target_link_libraries(libnavio rt m)
