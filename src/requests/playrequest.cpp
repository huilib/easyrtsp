

#include "playrequest.h"
#include "../rtspconnect.h"
#include "../rtspsession.h"
#include "../utils/rtspparser.h"
#include "../rtprtcp/rtprtcpholder.h"
#include <hlog.h>
#include <sstream>

PlayRequest::PlayRequest(RtspRequestHeader&& header) noexcept
    : base_class_t(std::move(header)) {

}


PlayRequest::~PlayRequest() {

}


RTSP_REQUEST_CODE PlayRequest::doWork(HIOBuffer& buffer) {

    if (not GetConnection()->CheckSession(GetHeader().GetSessionId())) {
        return RTSP_REQUEST_CODE::SESSION_NOT_FOUND;
    }

    if (GetStream() == nullptr) {
        return RTSP_REQUEST_CODE::INVALID_PARAMETER;
    }

    if (GetConnection()->GetSession()->IsPlayed()) {
        return RTSP_REQUEST_CODE::UNACCEPTABLE;
    }

    response_common(buffer);
    buffer.Append("Range: npt=0.000-\r\n");
    buffer.Append(HStr::Format("Session: %s\r\n", GetHeader().GetSessionId().c_str()));

    const RtpRtcpHolder& rtp_rtcp =  GetConnection()->GetSession()->GetRtpRtcpHolder();
    const std::vector<TrackSink*>& sinks = rtp_rtcp.GetTrackSinks();

    std::stringstream ss;
    ss << "RTP-Info: ";

    HSTR strLocalIP = HIp4Addr::GetLocalIp();

    for (std::vector<TrackSink*>::size_type i = 0; i < sinks.size(); ++i) {
        const TrackSink* ts = sinks[i];
        ss << "url=rtsp://" << strLocalIP << "/normal_h264/track"
            << ts->GetTrackInfo()->GetTrackId() << ";seq="
            << ts->GetRtpSink()->GetSequence() << ";rtptime="
            << ts->GetRtpSink()->GetTimebase() << ",";
    }

    HStr strinfo(ss.str());
    strinfo.RemoveRight(1);
    strinfo.append("\r\n\r\n");

    buffer.Append(strinfo);

    GetConnection()->GetSession()->Play();
    
    return RTSP_REQUEST_CODE::OK;

}


REGISTE_REQ_API(RTSP_METHOD::RM_PLAY, PlayRequest);