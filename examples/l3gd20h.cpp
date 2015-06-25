#include <application.h>
#include <poller.h>
#include <i2c.h>
#include <l3gd20h.h>
#include <timer.h>
#include <log.h>
#include <math.h>
#include <utils.h>

class Main: public Application
{
    I2C         i2c;
    L3GD20H     l3gd20h;
    Timer       l3gd20h_timer, stats_timer;

    float       xa, ya, za;
    uint32_t    gc;

protected:
    virtual bool _onStart() {
        xa=0, ya=0, za=0;
        gc=0;

        Info() << "Initializing I2C";
        if (i2c.openDevice("/dev/i2c-1") < 0) {
            Error() << "Unable to open i2c device";
            return false;
        }

        Info() << "Initializing sensors";
        if (l3gd20h.initialize() < 0) {
            Error() << "Unable to initialize gyroscope";
            return 255;
        }

        l3gd20h.onData = [&](float x, float y, float z) {
            xa += x; ya += y; za +=z;
            gc++;
        };

        if (l3gd20h.start(L3GD20H_RATE_OCTA) < 0) {
            Error() << "Unable to start gyroscope data colection";
            return 255;
        }

        Info() << "Initializing timers";
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

        l3gd20h_timer.callback = [&]() {
            Info() << "Gyroscope XYZ" << xa / gc << ya / gc << za / gc
                   << "samples: " << gc;
            xa = ya = za = 0;
            gc = 0;
        };
        l3gd20h_timer.start(1000);

        return Application::_onStart();
    }

    virtual bool _onQuit() {
        Info() << "Cleanuping resources";

        stats_timer.stop();
        l3gd20h_timer.stop();
        l3gd20h.stop();

        return Application::_onQuit();
    }
};

int main(int argc, char **argv) {
    Main m;
    return m.run(argc, argv);
}
