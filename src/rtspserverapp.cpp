

#include "rtspserverapp.h"
#include <hlog.h>


RtspServerApp::RtspServerApp(HN argc, HCSZ argv[]) 
    : base_class_t(argc, argv) {

}



RtspServerApp::~RtspServerApp() {

    if (m_let != nullptr) {
        HDELP(m_let);
    }

}




HRET RtspServerApp::Init () {

    HNOTOK_MSG_RETURN(base_class_t::Init(), "HApp init failed");

    m_let = new ScheduleLet(this);
    NULLPOINTER_CHECK(m_let);

    HNOTOK_MSG_RETURN(m_let->InitServlet(), "schedule init failed");


    HRETURN_OK;
}



bool RtspServerApp::Run () {

    HIGNORE_RETURN(m_let->ServletRun());

    return true;
}


