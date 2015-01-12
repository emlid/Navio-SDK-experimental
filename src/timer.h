#ifndef TIMER_H
#define TIMER_H

#include "fdevent.h"
#include <time.h>
#include <stdint.h>
#include <functional>

/** FDTimer wrapper.
 *  Class provides event driven timers.
 */
class Timer: public FDEvent
{
    enum State {
        Idle,
        Running
    };

public:
    /** Timer constructor.
     * @param event_poller - EventPoller instance which will be used to process events.
     */
    Timer(EventPoller *event_poller);
    virtual ~Timer();

    /** User set callback. Will be called on timer timeout. */
    std::function<void(void)> callback;

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
