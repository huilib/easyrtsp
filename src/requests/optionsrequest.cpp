
#include "optionsrequest.h"
#include "../mediasource/rtspstream.h"


OptionsRequest::OptionsRequest(RtspRequestHeader&& header) noexcept
    : base_class_t(std::move(header)) {

}


OptionsRequest::~OptionsRequest() {

}


RTSP_REQUEST_CODE OptionsRequest::doWork(HIOBuffer& buffer) {

    response_common(buffer);

    HSTR names(GetStream()->GetSupportMethods());
    buffer.Append(HStr::Format("Public: %s\r\n\r\n", names.c_str()));

    return RTSP_REQUEST_CODE::OK;
}



REGISTE_REQ_API(RTSP_METHOD::RM_OPTIONS, OptionsRequest);