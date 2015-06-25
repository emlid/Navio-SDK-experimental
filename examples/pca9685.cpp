#include <application.h>
#include <poller.h>
#include <i2c.h>
#include <pca9685.h>
#include <timer.h>
#include <log.h>
#include <math.h>
#include <utils.h>

#define PI 3.14159265

class Main: public Application
{
    I2C         i2c;
    PCA9685     pwm;
    Timer       pwm_timer, stats_timer;

    float       pwm_value = 0;

protected:
    virtual bool _onStart() {
        pwm_value = 0;
        Info() << "Initializing I2C";
        if (i2c.openDevice("/dev/i2c-1") < 0) {
            Error() << "Unable to open i2c device";
            return false;
        }

        Info() << "Initializing sensors";
        if (pwm.initialize() < 0) {
            Error() << "Unable to initialize PCA9685";
            return 255;
        }

        pwm.setFrequency(1000);
        Info() << "PWM frequency is" << pwm.getFreqeuncy();

        Info() << "Initializing timers";
        pwm_timer.callback = [&](){
            pwm_value += 2;
            pwm.setPWM(0, (powf((sinf(pwm_value * PI/180)+1)/2, 2)) * 4096);
            pwm.setPWM(1, (powf((sinf((pwm_value+45) * PI/180)+1)/2, 2)) * 4096);
            pwm.setPWM(2, (powf((sinf((pwm_value+90) * PI/180)+1)/2, 2)) * 4096);
            pwm.setPWM(3, (powf((sinf((pwm_value+130) * PI/180)+1)/2, 2)) * 4096);
            if (pwm_value > 360) pwm_value -= 360;
        };
        pwm_timer.start(16);

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

        pwm.setAllPWM(0);

        return Application::_onQuit();
    }
};

int main(int argc, char **argv) {
    Main m;
    return m.run(argc, argv);
}
