#ifndef APPLICATION_H
#define APPLICATION_H

class Poller;
class Signal;

class Application
{
public:
    Application();
    virtual ~Application();

    int run(int argc, char **argv);

protected:
    Poller *_event_poller;
    Signal *_signal;
    int _exit_code;

    virtual bool _onStart();
    virtual bool _onQuit();
};

#endif // APPLICATION_H
