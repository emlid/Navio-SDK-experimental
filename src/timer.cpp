#include "timer.h"
#include "log.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include <cassert>
#include <errno.h>

Timer::Timer():
    Timer(EventPoller::getDefault())
{

}

Timer::Timer(EventPoller *event_poller):
    FDEvent(event_poller), _state(Idle)
{
    _fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    assert(_fd);
    _registerRead();
}

Timer::~Timer()
{
    _unregisterRead();
    close(_fd);
}

const char* Timer::name()
{
    return "Timer";
}

int Timer::start(uint64_t interval)
{
    timespec interval_spec;
    interval_spec.tv_sec = interval / 1000;
    interval_spec.tv_nsec = interval % 1000 * 1000000;

    return start(interval_spec, interval_spec);
}

int Timer::start(uint64_t timeout, uint64_t interval)
{
    timespec timeout_spec;
    timeout_spec.tv_sec = timeout / 1000;
    timeout_spec.tv_nsec = timeout % 1000 * 1000000;

    timespec interval_spec;
    interval_spec.tv_sec = interval / 1000;
    interval_spec.tv_nsec = interval % 1000 * 1000000;

    return start(timeout_spec, interval_spec);
}

int Timer::start(timespec interval)
{
    return start(interval, interval);
}

int Timer::start(timespec timeout, timespec interval)
{
    itimerspec spec;
    spec.it_interval = interval;
    spec.it_value = timeout;

    if (timerfd_settime(_fd, 0, &spec, nullptr) != 0) {
        Error() << "Error setting timer parameters. errno" << errno << strerror(errno);
        return -1;
    }

    _state = Running;

    return 0;
}

int Timer::stop()
{
    timespec timeout_spec;
    timeout_spec.tv_sec = 0;
    timeout_spec.tv_nsec = 0;

    timespec interval_spec;
    interval_spec.tv_sec = 0;
    interval_spec.tv_nsec = 0;

    itimerspec spec;
    spec.it_interval = interval_spec;
    spec.it_value = timeout_spec;

    if (timerfd_settime(_fd, 0, &spec, nullptr) != 0) {
        Error() << "Error setting timer parameters. errno" << errno << strerror(errno);
        return -1;
    }

    _state = Idle;
    return 0;
}

void Timer::_onRead()
{
    uint64_t expiration_count = 0;
    if (read(_fd, &expiration_count, sizeof(uint64_t)) == sizeof(uint64_t)) {
        if (expiration_count > 1) {
            Warn() << this << expiration_count << "timeout events was coalesced. Check CPU usage and application logic.";
        }

        while (expiration_count > 0 && _state == Running) {
            expiration_count--;
            callback();
        }
    } else {
        Error() << "Incomplete timer data";
    }
}

void Timer::_onWrite()
{
}
