#include "signal.h"
#include "poller.h"
#include "log.h"

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <cassert>

Signal::Signal():
    Signal(Poller::getDefault())
{

}

Signal::Signal(Poller *event_poller):
    Descriptor(event_poller), onSignal(nullptr), _sigset()
{
    sigemptyset(&_sigset);
    assert(sigprocmask(SIG_SETMASK, &_sigset, NULL) == 0);
    _descriptor = signalfd(-1, &_sigset, SFD_NONBLOCK);
    assert(_descriptor);
    _registerRead();
}

Signal::~Signal()
{
    _unregisterRead();
    close(_descriptor);
}

const char* Signal::name()
{
    return "Signal";
}

int Signal::subscribe(int signum)
{
    auto ret = sigaddset(&_sigset, signum);
    assert(sigprocmask(SIG_SETMASK, &_sigset, NULL) == 0);
    assert(signalfd(_descriptor, &_sigset, SFD_NONBLOCK) > 0);
    return ret;
}

int Signal::unsubscribe(int signum)
{
    auto ret = sigdelset(&_sigset, signum);
    assert(sigprocmask(SIG_SETMASK, &_sigset, NULL) == 0);
    assert(signalfd(_descriptor, &_sigset, SFD_NONBLOCK) > 0);
    return ret;
}

int Signal::issubscribed(int signum)
{
    return sigismember(&_sigset, signum);
}

void Signal::_onRead()
{
    signalfd_siginfo siginfo;
    if (read(_descriptor, &siginfo, sizeof(siginfo)) == sizeof(siginfo)) {
        if (onSignal) {
            onSignal(siginfo);
        } else {
            Warn() << "Signal handler is not set";
        }
    } else {
        Error() << "Incomplete Signal data";
    }
}

void Signal::_onWrite()
{

}

