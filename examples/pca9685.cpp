
#include <eventpoller.h>
#include <i2c.h>
#include <pca9685.h>
#include <timer.h>
#include <log.h>
#include <math.h>

#define PI 3.14159265

float wtfround(float number, float precision) {
    return (float) (floor(number * (1.0f/precision) + 0.5)/(1.0f/precision));
}

int main(int argc, char **argv) {
    EventPoller ep;
    I2C         i2c;
    PCA9685     pwm;
    Timer       pwm_timer, stats_timer;

    float       pwm_value = 0;

    Info() << "Initializing I2C";
    if (i2c.openDevice("/dev/i2c-1") < 0) {
        Error() << "Unable to open i2c device";
        return 255;
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
