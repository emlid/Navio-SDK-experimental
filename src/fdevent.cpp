#include "fdevent.h"
#include "eventpoller.h"
#include <cassert>

FDEvent::FDEvent(EventPoller *event_poller):
    _ep(event_poller), _fd(-1)
{
    assert(_ep);
}

FDEvent::~FDEvent()
{

}

void FDEvent::_registerRead()
{
    _ep->_registerFDRead(_fd, this);
}

void FDEvent::_registerWrite()
{
    _ep->_registerFDWrite(_fd, this);
}

void FDEvent::_unregisterRead()
{
    _ep->_unregisterFDRead(_fd);
}

void FDEvent::_unregisterWrite()
{
    _ep->_unregisterFDWrite(_fd);
}
