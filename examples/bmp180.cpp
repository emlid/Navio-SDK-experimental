#include <poller.h>
#include <i2c.h>
#include <bmp180.h>
#include <timer.h>
#include <log.h>
#include <utils.h>
#include <application.h>

class Main: public Application
{
    I2C         i2c;
    BMP180      bmp180;
    Timer       bmp180_timer, stats_timer;

protected:
    virtual bool _onStart() {
        Info() << "Initializing I2C";
        if (i2c.openDevice("/dev/i2c-1") < 0) {
            Error() << "Unable to open i2c device";
            return false;
        }

        Info() << "Initializing sensors";
        if (bmp180.initialize() < 0) {
            Error() << "Unable to initialize BMP180";
            return false;
        }
        bmp180.setOversampling(BMP180_OVERSAMPLING_OCTA);
        bmp180.onTemperatureAndPressure = [&](float temperature, float pressure) {
            Info() << "Temperature" << temperature << "Pressure" << pressure;
        };

        Info() << "Initializing timers";
        bmp180_timer.callback = [&]() {
            if (bmp180.getTemperatureAndPressure()<0) {
                Error() << "Unable to get temperature and pressure";
            }
        };
        bmp180_timer.start(1000);

        stats_timer.callback = [&]() {
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
        bmp180_timer.stop();
        bmp180.reset();

        return Application::_onQuit();
    }

};

int main(int argc, char **argv) {
    Main m;
    return m.run(argc, argv);
}
