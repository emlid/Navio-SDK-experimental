#ifndef SIGNAL_H
#define SIGNAL_H

#include "descriptor.h"
#include <functional>
#include <sys/signalfd.h>
#include <signal.h>

/** Linux signalfd wrapper.
 *  Class provides event driven signal handling.
 *  More documentation can be found in signalfd man.
 */
class Signal: public Descriptor
{

public:
    std::function<void(signalfd_siginfo&)> onSignal;

    /** Signal constructor with default eventloop. */
    Signal();
    /** Signal constructor.
     * @param event_poller - EventPoller instance which will be used to process events.
     */
    Signal(Poller *event_poller);
    Signal(const Signal& that) = delete;  /**< Copy contructor not allowed because of internal state and file descriptor. */
    virtual ~Signal();
    virtual const char* name();

    int subscribe(int signum);
    int unsubscribe(int signum);
    int issubscribed(int signum);

protected:
    virtual void _onRead();
    virtual void _onWrite();

private:
    sigset_t _sigset;
};

#endif // SIGNAL_H
