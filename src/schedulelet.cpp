

#include "scheduleobjectbase.h"
#include "schedulelet.h"

#include <happ.h>

#include <hlog.h>

ScheduleLet::ScheduleLet(HApp* app) noexcept 
    : base_class_t("ScheduleLet", app), m_task_schedule(),
    m_rtsp_server(nullptr) {
    
    init_schedule(&m_task_schedule);

}


ScheduleLet::~ScheduleLet() {

}


HRET ScheduleLet::InitServlet() {

    m_rtsp_server = new RtspServer("0.0.0.0", 6666);
    NULLPOINTER_CHECK(m_rtsp_server);

    m_rtsp_server->SetLet(this);

    HNOTOK_MSG_RETURN(m_rtsp_server->Init(), "rtsp init failed");

    HRETURN_OK;

}


HRET ScheduleLet::ServletRun () {

    m_task_schedule.DoEventLoop();

    HRETURN_OK;

}


void ScheduleLet::Stop() {

    m_task_schedule.Stop();

}