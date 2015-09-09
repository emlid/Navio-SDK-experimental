#ifndef TIMER_H
#define TIMER_H

#include "poller.h"
#include "descriptor.h"
#include <time.h>
#include <stdint.h>
#include <functional>

/** Linux timerfd wrapper.
 *  Class provides event driven timers.
 */
class Timer: public Descriptor
{
    enum State {
        Idle,
        Running
    };

public:
    /** Timer constructor with default eventloop. */
    Timer();
    /** Timer constructor.
     * @param event_poller - EventPoller instance which will be used to process events.
     */
    Timer(Poller *event_poller);
    Timer(const Timer& that) = delete;  /**< Copy contructor not allowed because of internal state and file descriptor. */
    virtual ~Timer();

    virtual const char* name();

    /** User set callback. Will be called on timer timeout. */
    std::function<void(void)> onTimeout;

    /** Start one shot timer.
     * @param timeout - interval in msec.
     * @return 0 on success or negative value on error
     */
    int singleShot(uint64_t timeout);

    /** Start timer.
     * @param interval - interval in msec.
     * @return 0 on success or negative value on error
     */
    int start(uint64_t interval);

    /** Start timer.
     * @param timeout - initial timeout in msec.
     * @param interval - interval in msec.
     * @return 0 on success or negative value on error
     */
    int start(uint64_t timeout, uint64_t interval);

    /** Start timer with timespec.
     * Allows to create sub millisecond timers.
     * @param interval - interval timespec.
     * @return 0 on success or negative value on error
     */
    int start(timespec interval);

    /** Start timer with timespec.
     * Allows to create sub millisecond timers.
     * @param timeout - timeout timespec.
     * @param interval - interval timespec.
     * @return 0 on success or negative value on error
     */
    int start(timespec timeout, timespec interval);

    /**
     * stop timer
     * @return 0 on success or negative value on error
     */
    int stop();

protected:
    virtual void _onRead();
    virtual void _onWrite();

private:
    State _state;
};

#endif
