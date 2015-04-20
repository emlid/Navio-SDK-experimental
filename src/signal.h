#ifndef SIGNAL_H
#define SIGNAL_H

#include "fdevent.h"
#include <functional>

/** Linux signalfd wrapper.
 *  Class provides event driven signal handling.
 *  More documentation can be found in signalfd man.
 */
class Signal: public FDEvent
{

public:
    std::function<void(void)> onSigint;

    /** Timer constructor with default eventloop. */
    Signal();
    /** Timer constructor.
     * @param event_poller - EventPoller instance which will be used to process events.
     */
    Signal(EventPoller *event_poller);
    Signal(const Signal& that) = delete;  /**< Copy contructor not allowed because of internal state and file descriptor. */

    virtual ~Signal();

protected:
    virtual void _onRead();
    virtual void _onWrite();

};

#endif // SIGNAL_H
