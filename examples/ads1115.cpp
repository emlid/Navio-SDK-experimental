#include <poller.h>
#include <i2c.h>
#include <ads1115.h>
#include <timer.h>
#include <log.h>
#include <utils.h>
#include <application.h>

class Main: public Application
{
    I2C         i2c;
    ADS1115     ads1115;
    Timer       stats_timer;

protected:
    virtual bool _onStart() {
        Info() << "Initializing I2C";
        if (i2c.openDevice("/dev/i2c-1") < 0) {
            Error() << "Unable to open i2c device";
            return false;
        }

        Info() << "Initializing sensors";
        if (ads1115.initialize() < 0) {
            Error() << "Unable to initialize ADS1115";
            return false;
        }
        ads1115.onData = [&](float value) {
            Info() << "Voltage is" << value;
        };

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

        ads1115.startSampling(ADS1115::MS1G, ADS1115::G2048, ADS1115::SR8, false);

        return Application::_onStart();
    }

    virtual bool _onQuit() {
        Info() << "Cleanuping resources";

        stats_timer.stop();

        return Application::_onQuit();
    }
};

int main(int argc, char **argv) {
    Main m;
    return m.run(argc, argv);
}
