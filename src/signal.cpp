#include "signal.h"
#include "eventpoller.h"
#include "log.h"

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <cassert>

Signal::Signal():
    Signal(EventPoller::getDefault())
{

}

Signal::Signal(EventPoller *event_poller):
    FDEvent(event_poller)
{
    sigset_t mask;
    sigfillset(&mask);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        Error() << "Can not set process sognal mask";
    }

    _fd = signalfd(-1, &mask, SFD_NONBLOCK);
    assert(_fd);
    _registerRead();

}

Signal::~Signal()
{
    _unregisterRead();
    close(_fd);
}

void Signal::_onRead()
{
    signalfd_siginfo siginfo;
    if (read(_fd, &siginfo, sizeof(siginfo)) == sizeof(siginfo)) {
        switch (siginfo.ssi_signo) {
        case SIGINT:
            if (onSigint) {
                onSigint();
            } else {
                Warn() << "Handler for" << strsignal(siginfo.ssi_signo) << "signal is not set.";
            }
            break;
        default:
            Warn() << "Handler for" << strsignal(siginfo.ssi_signo) << "signal is not implemented.";
            break;
        }
    } else {
        Error() << "Incomplete Signal data";
    }
}

void Signal::_onWrite()
{

}

