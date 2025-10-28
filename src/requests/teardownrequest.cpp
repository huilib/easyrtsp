

#include "teardownrequest.h"
#include "../rtspconnect.h"
#include "../rtspsession.h"
#include <hlog.h>

TearRequest::TearRequest(RtspRequestHeader&& header) noexcept
    : base_class_t(std::move(header)) {

}


TearRequest::~TearRequest() {

}


RTSP_REQUEST_CODE TearRequest::doWork(HIOBuffer& buffer) {

    if (not GetConnection()->CheckSession(GetHeader().GetSessionId())) {
        return RTSP_REQUEST_CODE::SESSION_NOT_FOUND;
    }

    GetConnection()->GetSession()->StopPlay();

    response_common(buffer);
    buffer.Append("\r\n");
    
    GetConnection()->SetDeathActive();    
    
    return RTSP_REQUEST_CODE::OK;

}


REGISTE_REQ_API(RTSP_METHOD::RM_TEARDOWN, TearRequest);