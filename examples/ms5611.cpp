
#include <poller.h>
#include <i2c.h>
#include <ms5611.h>
#include <timer.h>
#include <log.h>
#include <utils.h>
#include <application.h>

class Main: public Application
{
    I2C         i2c;
    MS5611      ms5611;
    Timer       ms5611_timer, stats_timer;

protected:
    virtual bool _onStart() {
        Info() << "Initializing I2C";
        if (i2c.openDevice("/dev/i2c-1") < 0) {
            Error() << "Unable to open i2c device";
            return false;
        }

        Info() << "Initializing sensors";
        if (ms5611.initialize() < 0) {
            Error() << "Unable to initialize ms5611";
            return false;
        }
        ms5611.setOversampling(MS5611_OVERSAMPLING_4096);
        ms5611.onTemperatureAndPressure = [&](float temperature, float pressure) {
            Info() << "Temperature" << temperature << "Pressure" << pressure;
        };

        Info() << "Initializing timers";
        ms5611_timer.onTimeout = [&]() {
            if (ms5611.getTemperatureAndPressure()<0) {
                Error() << "Unable to get temperature and pressure";
            }
        };
        ms5611_timer.start(1000);

        stats_timer.onTimeout = [&]() {
            float epoll_mono, callback_mono, epoll_cpu, callback_cpu;
            _event_poller->getTimings(epoll_mono, callback_mono, epoll_cpu, callback_cpu);
            Info() << "Real Time: epoll" << epoll_mono << "callbacks" << callback_mono
                   << "ratio" << roundTo(epoll_mono / (epoll_mono + callback_mono) * 100, 0.1) << "%"
                   << "/" << roundTo(callback_mono / (epoll_mono + callback_mono) * 100, 0.1) << "%";
            Info() << "CPU Time: epoll" << epoll_cpu << "callbacks" << callback_cpu
                   << "ratio" << roundTo(epoll_cpu / (epoll_cpu + callback_cpu) * 100, 0.1) << "%"
                   << "/" << roundTo(callback_cpu / (epoll_cpu + callback_cpu) * 100, 0.1) << "%";
        };
        stats_timer.start(5000);

        return Application::_onStart();
    }

    virtual bool _onQuit() {
        Info() << "Cleanuping resources";

        stats_timer.stop();
        ms5611_timer.stop();
        ms5611.reset();

        return Application::_onQuit();
    }

};

int main(int argc, char **argv) {
    Main m;
    return m.run(argc, argv);
}
