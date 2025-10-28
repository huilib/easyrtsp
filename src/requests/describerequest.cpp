
#include "describerequest.h"
#include "../rtspconnect.h"
#include "../mediasource/rtspstream.h"
#include <haddr.h>
#include <hlog.h>
#include <sstream>

DescribeRequest::DescribeRequest(RtspRequestHeader&& header) noexcept
    : base_class_t(std::move(header)) {

}


DescribeRequest::~DescribeRequest() {

}


RTSP_REQUEST_CODE DescribeRequest::doWork(HIOBuffer& buffer) {

    HSTR strSdp = generateSDP();
    response_common(buffer);
    HSTR strLocalIP = HIp4Addr::GetLocalIp();
    buffer.Append(HStr::Format("Content-Base: rtsp://%s/%s/\r\n", 
        strLocalIP.c_str(), 
        GetHeader().GetUrl().GetRequestStreamName().c_str()));
    buffer.Append("Content-Type: application/sdp\r\n");
    buffer.Append(HStr::Format("Content-Length: %d\r\n\r\n", strSdp.length()));
    buffer.Append(strSdp);

    return RTSP_REQUEST_CODE::OK;
}


HSTR DescribeRequest::generateSDP() const{

    HSTR strSdp = GetStream()->GenerateSDP();

    return strSdp;
    
}

REGISTE_REQ_API(RTSP_METHOD::RM_DESCRIBE, DescribeRequest);

