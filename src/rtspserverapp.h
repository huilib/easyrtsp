
#ifndef __H_RTSPSERVER_APP_H__
#define __H_RTSPSERVER_APP_H__


#include <happ.h>

#include "schedulelet.h"

using namespace HUICPP;

class RtspServerApp : public HApp {
private:
    using base_class_t = HApp;

public:
    RtspServerApp(HN argc, HCSZ argv[]);

    ~RtspServerApp();

public:
    HRET Init () override;

    bool Run () override;    

private:
    ScheduleLet* m_let;
};


#endif // __H_RTSPSERVER_APP_H__

