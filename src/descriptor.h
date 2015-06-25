#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

class Poller;

/** Abstract fd event.
 *  Class provides EventPoller interaction layer.
 */
class Descriptor
{
    friend class Poller;
public:
    /** Descriptor constructor.
     * @param event_poller - EventPoller instance which will be used to process events.
     */
    Descriptor(Poller *event_poller);
    Descriptor(const Descriptor& that) = delete;  /**< Copy contructor is not allowed. */
    virtual ~Descriptor();
    virtual const char* name()=0;

protected:
    Poller *_ep;   /**< Pointer to event poller. */
    int _descriptor;    /**< Descriptor that we will use for event polling. */

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
