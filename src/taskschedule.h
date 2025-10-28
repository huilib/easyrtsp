

#ifndef __H_EVENT_SCHEDULE_H__
#define __H_EVENT_SCHEDULE_H__

#include <huicpp.h>
#include "delayqueue.h"
#include "fdhandler.h"

using namespace HUICPP;

class TaskScheduler {
public:
    TaskScheduler() noexcept;

    virtual ~TaskScheduler();

public:
    intptr_t ScheduleDelayedTask(int64_t microseconds, task_func proc);

    void UnscheduleDelayedTask(intptr_t prevTask);

    void SetBackgroundHandling(int socketNum, int conditionSet, fd_handler_callback_t handlerProc);

    void RemoveBackgroundHandling(int socketNum);

    void DoEventLoop();

    void Stop() { m_bStop = true; }

private:
    void single_step();

private:
    intptr_t m_token_counter;
    DelayQueue m_delay_queue;
    FdHandlerMap m_fds;
    bool volatile m_bStop;
};


#endif // __H_EVENT_SCHEDULE_H__

