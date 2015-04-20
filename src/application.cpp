#include "application.h"
#include "eventpoller.h"

Application::Application(int argc, char **argv):
    _event_poller(new EventPoller)
{

}

Application::~Application()
{
    delete _event_poller; _event_poller = nullptr;
}

