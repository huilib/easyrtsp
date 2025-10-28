

#include "safertsprequest.h"
#include "../mediasource/streambase.h"
#include <hlog.h>


SafeRtspRequest::SafeRtspRequest(RtspRequestHeader&& header) noexcept
    : base_class_t(std::move(header)) {

}


SafeRtspRequest::~SafeRtspRequest() {

}


void SafeRtspRequest::Handle(HIOBuffer& buffer) {

    HSTR strRequestStreamName = GetHeader().GetUrl().GetRequestStreamName();
    if (strRequestStreamName.empty()) {
        LOG_WARNING("request stream name is null");
        response_code(buffer, RTSP_REQUEST_CODE::BAD);
        return ;
    }

    StreamBase* sb = GetConnection()->Server().GetStream(strRequestStreamName);
    if (sb == nullptr) {
        LOG_WARNING("source stream is null");
        response_code(buffer, RTSP_REQUEST_CODE::NOT_FOUND);
        return;
    }

    m_stream = reinterpret_cast<RtspStream*>(sb);

    RTSP_REQUEST_CODE res_code = doWork(buffer);
    if (res_code != RTSP_REQUEST_CODE::OK) {
        response_code(buffer, res_code);
    }
}



void SafeRtspRequest::response_common(HIOBuffer& buffer) const {

    buffer.Reset();

    buffer.Append("RTSP/1.0 200 OK\r\n");
    buffer.Append(HStr::Format("CSeq: %s\r\n", GetHeader().GetCSeq().c_str()));
    buffer.Append(dateHeader());

}


void SafeRtspRequest::response_code(HIOBuffer& buffer, RTSP_REQUEST_CODE code) {

    HSTR strResponse = RtspProtocol::RtspRequestCodeString(code);
    setRtspResponse(HStr::Format("%d %s", HTO_INT(code), strResponse.c_str()), buffer);

}


void SafeRtspRequest::setRtspResponse(HCSTRR str, HIOBuffer& buffer) const {

    buffer.Reset();

    buffer.Append(HStr::Format("RTSP/1.0 %s\r\n", str.c_str()));
    buffer.Append(HStr::Format("CSeq: %s\r\n", GetHeader().GetCSeq().c_str()));
    buffer.Append(dateHeader());
    buffer.Append("\r\n");

}


HSTR SafeRtspRequest::dateHeader() const {
    static char buf[200];
    memset(buf, 0, 200);
    time_t tt = time(nullptr);
    tm time_tm;
    if (gmtime_r(&tt, &time_tm) == nullptr) {
        time_tm = tm();
    }
    static const char* day[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char* month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    snprintf(buf, sizeof buf, "Date: %s, %s %02d %04d %02d:%02d:%02d GMT\r\n",
            day[time_tm.tm_wday], month[time_tm.tm_mon], time_tm.tm_mday,
            1900 + time_tm.tm_year,
            time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec);
    return HSTR(buf);        
}

