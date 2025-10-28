

#include "scheduleobjectbase.h"


TaskScheduler* g_golbal_schedule = nullptr;

ScheduleOBjectbase::ScheduleOBjectbase() 
    : m_scheduler(*g_golbal_schedule) {

}


ScheduleOBjectbase::~ScheduleOBjectbase() {

}

/*
    NOTE: make sure all ScheduleOBjectbase object deleted before ScheduleLet release.
*/
void init_schedule(TaskScheduler* p) {
    g_golbal_schedule = p;
}