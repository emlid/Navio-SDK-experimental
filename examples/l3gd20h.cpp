#include <eventpoller.h>
#include <i2c.h>
#include <l3gd20h.h>
#include <timer.h>
#include <log.h>
#include <math.h>

float wtfround(float number, float precision) {
    return (float) (floor(number * (1.0f/precision) + 0.5)/(1.0f/precision));
}

int main(int argc, char **argv) {
    EventPoller ep;
    I2C         i2c;
    L3GD20H     gyroscope;
    Timer       gyro_timer, stats_timer;

    float       xa=0, ya=0, za=0;
    uint32_t    gc=0;

    Info() << "Initializing I2C";
    if (i2c.openDevice("/dev/i2c-1") < 0) {
        Error() << "Unable to open i2c device";
        return 255;
    }

    Info() << "Initializing sensors";
    if (gyroscope.initialize() < 0) {
        Error() << "Unable to initialize gyroscope";
        return 255;
    }

    gyroscope.onData = [&](float x, float y, float z) {
        xa += x; ya += y; za +=z;
        gc++;
    };

    if (gyroscope.start(L3GD20H_RATE_OCTA) < 0) {
        Error() << "Unable to start gyroscope data colection";
        return 255;
    }

    Info() << "Initializing timers";
    stats_timer.callback = [&]() {
        float epoll_mono, callback_mono, epoll_cpu, callback_cpu;
        ep.getTimings(epoll_mono, callback_mono, epoll_cpu, callback_cpu);
        Info() << "Real Time: epoll" << epoll_mono
               << "callbacks" << callback_mono
               << "ratio" << wtfround(epoll_mono / (epoll_mono + callback_mono) * 100, 0.1) << "%"
               << "/" << wtfround(callback_mono / (epoll_mono + callback_mono) * 100, 0.1) << "%";
        Info() << "CPU Time: epoll" << epoll_cpu
               << "callbacks" << callback_cpu
               << "ratio" << wtfround(epoll_cpu / (epoll_cpu + callback_cpu) * 100, 0.1) << "%"
               << "/" << wtfround(callback_cpu / (epoll_cpu + callback_cpu) * 100, 0.1) << "%";
    };
    stats_timer.start(5000, 5000);

    gyro_timer.callback = [&]() {
        Info() << "Gyroscope XYZ" << xa / gc << ya / gc << za / gc
               << "samples: " << gc;
        xa = ya = za = 0;
        gc = 0;
    };
    gyro_timer.start(500, 500);


    Info() << "Polling events";
    ep.loop();

    return 0;
}

