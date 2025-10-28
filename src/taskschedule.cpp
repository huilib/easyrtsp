

#include "taskschedule.h"


TaskScheduler::TaskScheduler() noexcept
    : m_token_counter(0), m_delay_queue(), m_fds(),
    m_bStop(false) {

}


TaskScheduler::~ TaskScheduler() {

}


intptr_t TaskScheduler::ScheduleDelayedTask(int64_t microseconds, task_func proc) {

    if (microseconds < 0) {
        microseconds = 0;
    }

    DelayInterval timeToDelay((long)(microseconds/1000000), (long)(microseconds%1000000));
    TaskHandler* task = new TaskHandler(proc, timeToDelay, ++m_token_counter);
    m_delay_queue.AddEntry(task);

    return task->token();

}


void TaskScheduler::UnscheduleDelayedTask(intptr_t prevTask) {

    TaskHandler* task = m_delay_queue.RemoveEntry((intptr_t)prevTask);
    HDELP(task);

}


void TaskScheduler::SetBackgroundHandling(int socketNum, int conditionSet, 
        fd_handler_callback_t handlerProc) {

    if (socketNum < 0) {
        return ;
    }

    m_fds.AddHandler(socketNum, conditionSet, handlerProc);

}


void TaskScheduler::RemoveBackgroundHandling(int socketNum) {

    m_fds.RemoveHandler(socketNum);

}


void TaskScheduler::DoEventLoop() {

    while (HLIKELY(not m_bStop)) {

        single_step();
        
    }

}


void TaskScheduler::single_step() {

    const DelayInterval& timeToDelay = m_delay_queue.TimeToNextAlarm();
    struct timeval tv_timeToDelay;
    tv_timeToDelay.tv_sec = timeToDelay.seconds();
    tv_timeToDelay.tv_usec = timeToDelay.useconds();
    // Very large "tv_sec" values cause select() to fail.
    // Don't make it any larger than 1 million seconds (11.5 days)
    const long MAX_TV_SEC = DelayInterval::MILLION;
    if (tv_timeToDelay.tv_sec > MAX_TV_SEC) {
        tv_timeToDelay.tv_sec = MAX_TV_SEC;
    }

    fd_set readSet;
    fd_set writeSet;
    fd_set exceptionSet;

    FD_T mfd = m_fds.SetupFdsets(readSet, writeSet, exceptionSet);
    int selectResult = select(mfd + 1, &readSet, &writeSet, &exceptionSet, &tv_timeToDelay);
    
    if (selectResult < 0) {
        printf("select return negative. Abort\n");
        abort();
    }

    if (selectResult > 0) {
        m_fds.CallHandler(readSet, writeSet, exceptionSet);
    }

    m_delay_queue.HandleAlarm();

}