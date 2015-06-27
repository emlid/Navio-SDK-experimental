#include <poller.h>
#include <i2c.h>
#include <timer.h>
#include <log.h>
#include <utils.h>
#include <application.h>
#include <ssd1306.h>

class Main: public Application
{
    I2C         i2c;
    SSD1306     ssd1306;
    Timer       ssd1306_timer, stats_timer;

protected:
    virtual bool _onStart() {
        Info() << "Initializing I2C";
        if (i2c.openDevice("/dev/i2c-1") < 0) {
            Error() << "Unable to open i2c device";
            return false;
        }

        Info() << "Initializing sensors";
        if (ssd1306.initialize() < 0) {
            Error() << "Unable to initialize SSD1306";
            return false;
        }

        ssd1306_timer.callback = [&]() {
            ssd1306.clear();
            time_t rawtime;
            struct tm * timeinfo;
            char buffer [256];

            time(&rawtime);
            timeinfo = localtime(&rawtime);
            size_t size = strftime(buffer, 256, "%H:%M:%S\n%A\n%Y-%m-%d\n", timeinfo);

            ssd1306.drawText(0, 0, std::string(buffer, size), COLOR_WHITE, FONT_TERMINUS_v22n);
            ssd1306.refresh();
        };
        ssd1306_timer.start(250);

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
        ssd1306.clear();
        ssd1306.refresh();

        return Application::_onQuit();
    }

};

int main(int argc, char **argv) {
    Main m;
    return m.run(argc, argv);
}
