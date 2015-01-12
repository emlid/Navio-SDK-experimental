#ifndef FDEVENT_H
#define FDEVENT_H

class EventPoller;

/** Abstract fd event.
 *  Class provides EventPoller interaction layer.
 */
class FDEvent
{
    friend class EventPoller;
public:
    /** FD event constructor.
     * @param event_poller - EventPoller instance which will be used to process events.
     */
    FDEvent(EventPoller *event_poller);
    virtual ~FDEvent();

protected:
    EventPoller *_ep;   /**< Pointer to event poller. */
    int _fd;            /**< file descriptor to poll. */

    /**
     * Register read callback on read event for fd.
     */
    void _registerRead();

    /**
     * Register write callback on read event for fd.
     */
    void _registerWrite();

    /**
     * Unregister read callback on read event for fd.
     */
    void _unregisterRead();

    /**
     * Unregister write callback on read event for fd.
     */
    void _unregisterWrite();

    /**
     * Read event callback.
     * This method must be redefined in your class.
     */
    virtual void _onRead() = 0;

    /**
     * Write event callback.
     * This method must be redefined in your class.
     */
    virtual void _onWrite() = 0;
};

#endif
