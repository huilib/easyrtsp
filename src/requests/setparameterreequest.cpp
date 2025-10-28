

#include "setparameterrequest.h"


SetParameterRequest::SetParameterRequest(RtspRequestHeader&& header) noexcept
    : base_class_t(std::move(header)) {

}


SetParameterRequest::~SetParameterRequest() {

}


RTSP_REQUEST_CODE SetParameterRequest::doWork(HIOBuffer& buffer) {

    HSTR strContent("EasyRtsp V1.0 2026.02.02");

    response_common(buffer);
    buffer.Append(HStr::Format("Content-Length: %d\r\n\r\n", strContent.length()));
    buffer.Append(strContent);

    return RTSP_REQUEST_CODE::OK;

}


REGISTE_REQ_API(RTSP_METHOD::RM_SET_PARAMETER, SetParameterRequest);