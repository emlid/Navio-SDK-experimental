
#include <poller.h>
#include <i2c.h>
#include <timer.h>
#include <log.h>
#include <utils.h>
#include <application.h>
#include <vz89.h>

class Main: public Application
{
    I2C         i2c;
    VZ89        vz89;
    Timer       vz89_timer, stats_timer;

protected:
    virtual bool _onStart() {
        Info() << "Initializing I2C";
        if (i2c.openDevice("/dev/i2c-1") < 0) {
            Error() << "Unable to open i2c device";
            return false;
        }

        vz89_timer.onTimeout = [&]() {
            float co2, tvoc;
            uint8_t reactivity;
            if (vz89.getStatus(co2, reactivity, tvoc) < 0) {
                Warn() << "Sensor is not ready";
            } else {
            Info() << "co2:"<< co2 << "ppm,"
                   << "tvoc:" << tvoc << "ppb"
                   << "reactivity" << reactivity;
            }
        };
        vz89_timer.start(10000);

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
//        stats_timer.start(5000);

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
