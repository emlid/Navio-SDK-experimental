#include "descriptor.h"
#include "poller.h"
#include <cassert>

Descriptor::Descriptor(Poller *event_poller):
    _ep(event_poller), _descriptor(-1)
{
    assert(_ep);
}

Descriptor::~Descriptor()
{
}

void Descriptor::_registerRead()
{
    _ep->_registerDescriptorRead(_descriptor, this);
}

void Descriptor::_registerWrite()
{
    _ep->_registerDescriptorWrite(_descriptor, this);
}

void Descriptor::_unregisterRead()
{
    _ep->_unregisterDescriptorRead(_descriptor);
}

void Descriptor::_unregisterWrite()
{
    _ep->_unregisterDescriptorWrite(_descriptor);
}
