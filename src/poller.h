#ifndef POLLER_H
#define POLLER_H

#include <map>

class Descriptor;

/** Linux epoll wrapper.
 *  Class provides event loop based on linux epoll.
 */
class Poller
{
    friend class Descriptor;
public:
    Poller();
    Poller(const Poller& that) = delete;  /**< Copy contructor not allowed because of the file descriptor. */
    ~Poller();

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
    static Poller* getDefault();

private:
    float _epoll_mono_time;
    float _callback_mono_time;
    float _epoll_cpu_time;
    float _callback_cpu_time;

    int _fd;
    bool _run;
    std::map<int, Descriptor*> _fd_read_pool;
    std::map<int, Descriptor*> _fd_write_pool;

    bool _registerDescriptorRead(int fd, Descriptor *fd_event);
    bool _registerDescriptorWrite(int fd, Descriptor *fd_event);

    bool _unregisterDescriptorRead(int fd);
    bool _unregisterDescriptorWrite(int fd);
};

#endif
