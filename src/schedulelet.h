

#ifndef __H_SCHEDULE_LET_H__
#define __H_SCHEDULE_LET_H__

#include <huicpp.h>
#include <hservlet.h>

#include "taskschedule.h"
#include "rtspserver.h"

using namespace HUICPP;

class ScheduleLet : public HServletBase {
private:
    using base_class_t = HServletBase;

public:
    ScheduleLet(HApp* app) noexcept;

    ~ScheduleLet();

public:
    HRET InitServlet() override;

    HRET ServletRun () override;

    void Stop();

private:
    TaskScheduler m_task_schedule;
    RtspServer* m_rtsp_server;
};


#endif // __H_SCHEDULE_LET_H__

