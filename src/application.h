#ifndef APPLICATION_H
#define APPLICATION_H

class EventPoller;

class Application
{
public:
    Application(int argc, char **argv);
    virtual ~Application();

protected:
    EventPoller *_event_poller;

    virtual void onStart();
    virtual void onStop();
    virtual void onSignal();
};

#endif // APPLICATION_H
