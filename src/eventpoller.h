#ifndef EVENTPOLLER_H
#define EVENTPOLLER_H

#include <map>

class FDEvent;

/** Linux epoll wrapper.
 *  Class provides event loop based on linux epoll.
 */
class EventPoller
{
    friend class FDEvent;
public:
    EventPoller();
    EventPoller(const EventPoller& that) = delete;  /**< Copy contructor not allowed because of the file descriptor. */
    ~EventPoller();

    /** Run event loop.
     */
    void loop();

    /** Stop event processing.
     * And return from pollEvents(); after callback execution finish.
     */
    void stop();

    /** Request poller runtime statistics.
     * @param epoll_mono variable where will be stored amount of real time spent in epoll syscall
     * @param callback_mono variable where will be stored amount of real time spent in descriptor callback
     * @param epoll_cpu variable where will be stored amount of cpu time spent in epoll syscall
     * @param callback_cpu variable where will be stored amount of cpu time spent in descriptor callback
     */
    void getTimings(float &epoll_mono, float &callback_mono, float &epoll_cpu, float &callback_cpu);

    /** Get default event poller instance.
     * @return default event poller instance or nullptr.
     */
    static EventPoller* getDefault();

private:
    float _epoll_mono_time;
    float _callback_mono_time;
    float _epoll_cpu_time;
    float _callback_cpu_time;

    int _fd;
    bool _run;
    std::map<int, FDEvent*> _fd_read_pool;
    std::map<int, FDEvent*> _fd_write_pool;

    bool _registerFDRead(int fd, FDEvent *fd_event);
    bool _registerFDWrite(int fd, FDEvent *fd_event);

    bool _unregisterFDRead(int fd);
    bool _unregisterFDWrite(int fd);
};

#endif
