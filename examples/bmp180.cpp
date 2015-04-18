#include <eventpoller.h>
#include <i2c.h>
#include <bmp180.h>
#include <timer.h>
#include <log.h>
#include <math.h>

float wtfround(float number, float precision) {
    return (float) (floor(number * (1.0f/precision) + 0.5)/(1.0f/precision));
}

int main(int argc, char **argv) {
    EventPoller ep;
    I2C         i2c;
    BMP180      barometer;
    Timer       barometer_timer, stats_timer;

    Info() << "Initializing I2C";
    if (i2c.openDevice("/dev/i2c-1") < 0) {
        Error() << "Unable to open i2c device";
        return 255;
    }

    Info() << "Initializing sensors";
    if (barometer.initialize() < 0) {
        Error() << "Unable to initialize BMP180";
        return 255;
    }
    barometer.setOversampling(BMP180_OVERSAMPLING_OCTA);
    barometer.onTemperatureAndPressure = [&](float temperature, float pressure) {
        Info() << "Temperature" << temperature << "Pressure" << pressure;
    };

    Info() << "Initializing timers";
    barometer_timer.callback = [&]() {
        if (barometer.getTemperatureAndPressure()<0) {
            Error() << "Unable to get temperature and pressure";
        }
    };
    barometer_timer.start(1000);

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

    Info() << "Polling events";
    ep.loop();

    return 0;
}
