
#ifndef __H_SCHEDULEOBJECTBASE_H__
#define __H_SCHEDULEOBJECTBASE_H__

#include "taskschedule.h"

class ScheduleOBjectbase {
public:
    ScheduleOBjectbase();

    virtual ~ ScheduleOBjectbase();

public:
    TaskScheduler& Schedule() noexcept { return m_scheduler; }

private:
    TaskScheduler& m_scheduler;
};

void init_schedule(TaskScheduler* p);


#endif // __H_SCHEDULEOBJECTBASE_H__