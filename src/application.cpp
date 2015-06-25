#include "application.h"
#include "poller.h"
#include "signal.h"
#include "log.h"

Application::Application():
    _event_poller(new Poller),
    _signal(new Signal),
    _exit_code(EXIT_SUCCESS)
{
    _signal->onSignal = [&](signalfd_siginfo &siginfo) {
        if (_onQuit()) {
            _event_poller->stop();
        }
    };
    _signal->subscribe(SIGINT);
    _signal->subscribe(SIGTERM);
}

Application::~Application()
{
    delete _signal; _signal = nullptr;
    delete _event_poller; _event_poller = nullptr;
}

int Application::run(int argc, char **argv)
{
    if (!_onStart()) {
        return _exit_code == 0 ? 255 : _exit_code;
    }

    _event_poller->loop();

    return _exit_code;
}

bool Application::_onStart()
{
    return true;
}

bool Application::_onQuit()
{
    return true;
}
