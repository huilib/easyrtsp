

#include "getparameterrequest.h"


GetParameterRequest::GetParameterRequest(RtspRequestHeader&& header) noexcept
    : base_class_t(std::move(header)) {

}


GetParameterRequest::~GetParameterRequest() {

}


RTSP_REQUEST_CODE GetParameterRequest::doWork(HIOBuffer& buffer) {

    HSTR strContent("EasyRtsp V1.0 2026.02.02");

    response_common(buffer);
    buffer.Append(HStr::Format("Content-Length: %d\r\n\r\n", strContent.length()));
    buffer.Append(strContent);

    return RTSP_REQUEST_CODE::OK;

}

REGISTE_REQ_API(RTSP_METHOD::RM_GET_PARAMETER, GetParameterRequest);