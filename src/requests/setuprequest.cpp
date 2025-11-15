
#include "../mediasource/rtspstream.h"
#include "setuprequest.h"
#include "../rtspconnect.h"
#include "../rtspsession.h"
#include "../utils/rtspparser.h"
#include "../rtprtcp/rtpinfo.h"
#include <hlog.h>

SetupRequest::SetupRequest(RtspRequestHeader&& header) noexcept
    : base_class_t(std::move(header)) {

}


SetupRequest::~SetupRequest() {

}


RTSP_REQUEST_CODE SetupRequest::doWork(HIOBuffer& buffer) {

    if (not GetConnection()->CheckSession(GetHeader().GetSessionId())) {
        return RTSP_REQUEST_CODE::SESSION_NOT_FOUND;
    }

    if (GetConnection()->GetSession() == nullptr) {
        return RTSP_REQUEST_CODE::SESSION_NOT_FOUND;
    }

    if (not GetConnection()->GetSession()->CheckStream(GetStream())) {
        return RTSP_REQUEST_CODE::PRECONDITION_FAILED;
    }

    RtpTransportMode rtm = RtpTransportModeParse::ParseRtpTransportMode(
        GetHeader().GetAttribute("Transport"));
    if (rtm == RtpTransportMode::RTP_UNKNOWN) {
        return RTSP_REQUEST_CODE::UNSUPPORT_TRANSPORT; 
    }

    RtpRtcpHolder& holder = GetConnection()->GetSession()->GetRtpRtcpHolder();

    RTSP_REQUEST_CODE res = RTSP_REQUEST_CODE::UNKOWN;

    if (holder.GetTransportMode() == RtpTransportMode::RTP_UNKNOWN) {
        // setup the type of transport at first time.
        holder.SetTransportMode(rtm);
    }

    if (holder.GetTransportMode() != RtpTransportMode::RTP_UNKNOWN
        and holder.GetTransportMode() != rtm) {
        LOG_ERROR("different transport type. request[%d] holder[%d]",
            HTO_INT(holder.GetTransportMode()), HTO_INT(rtm));
        return RTSP_REQUEST_CODE::UNSUPPORT_TRANSPORT; 
    }

    if (rtm == RtpTransportMode::RTP_TCP) {
        res = do_tcp(buffer);
    }

    if (rtm == RtpTransportMode::RTP_UDP) {
        res = do_udp(buffer);
    }

    return res;    

}


RTSP_REQUEST_CODE SetupRequest::do_tcp(HIOBuffer& buffer) {

    HUN channel0, channel1;
    if (not rtsp_parser::parse_setup_rtp_channels(GetHeader().GetAttribute("Transport"), 
        channel0, channel1)) {
        return RTSP_REQUEST_CODE::UNSUPPORT_TRANSPORT;   
    }

    LOG_NORMAL("request streamname[%s] track[%s] channel0[%d] channel1[%d]", 
        GetHeader().GetUrl().GetRequestStreamName().c_str(),
        GetHeader().GetUrl().GetTrack().c_str(),
        channel0, channel1);
    
    TRACK_TYPE tt = TRACK_TYPE::UNKNOWN;
    if (HStr(GetHeader().GetUrl().GetTrack()).Upper() == "TRACK1") {
        tt = TRACK_TYPE::VIDEO;
    } 
    if (HStr(GetHeader().GetUrl().GetTrack()).Upper() == "TRACK2") {
        tt = TRACK_TYPE::AUDIO;
    }
    GetConnection()->GetSession()->SetTcpRtpChannel(tt, channel0, channel1);

    HCSTRR strSessionId = GetConnection()->GetSession()->GetSessionId();

    HSTR strOutIP = HIp4Addr::GetLocalIp();
    HSTR strClientIp;
    HN port;
    HIGNORE_RETURN(GetConnection()->GetClientIp().GetAddrInfo(strClientIp, port));

    response_common(buffer);
    buffer.Append(HStr::Format("Transport: RTP/AVP/TCP;unicast;destination=%s;source=%s;interleaved=%d-%d\r\n",
        strClientIp.c_str(), strOutIP.c_str(), channel0, channel1));
    buffer.Append(HStr::Format("Session: %s;timeout=65\r\n\r\n", strSessionId.c_str()));
    
    return RTSP_REQUEST_CODE::OK;

}


RTSP_REQUEST_CODE SetupRequest::do_udp(HIOBuffer& buffer) {

    HUN port0, port1;
    if (not rtsp_parser::parse_setup_rtp_ports(GetHeader().GetAttribute("Transport"), port0, port1)) {
        return RTSP_REQUEST_CODE::UNSUPPORT_TRANSPORT;   
    }

    LOG_NORMAL("request streamname[%s] track[%s] port0[%d] port1[%d]", 
        GetHeader().GetUrl().GetRequestStreamName().c_str(),
        GetHeader().GetUrl().GetTrack().c_str(),
        port0, port1);
    
    HUN our_rtp_port = 0;
    if (HStr(GetHeader().GetUrl().GetTrack()).Upper() == "TRACK1") {
        GetConnection()->GetSession()->SetVideoRtpPorts(port0, port1);
        our_rtp_port = GetStream()->GetTrackRtpPort(TRACK_TYPE::VIDEO);
    } 
    if (HStr(GetHeader().GetUrl().GetTrack()).Upper() == "TRACK2") {
        GetConnection()->GetSession()->SetAudioRtpPorts(port0, port1);
        our_rtp_port = GetStream()->GetTrackRtpPort(TRACK_TYPE::AUDIO);
    }
    
    HCSTRR strSessionId = GetConnection()->GetSession()->GetSessionId();

    HSTR strOutIP = HIp4Addr::GetLocalIp();
    HSTR strClientIp;
    HN port;
    HIGNORE_RETURN(GetConnection()->GetClientIp().GetAddrInfo(strClientIp, port));

    response_common(buffer);
    buffer.Append(HStr::Format("Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n",
        strClientIp.c_str(), strOutIP.c_str(), port0, port1, our_rtp_port, our_rtp_port + 1));
    buffer.Append(HStr::Format("Session: %s;timeout=65\r\n\r\n", strSessionId.c_str()));
    
    return RTSP_REQUEST_CODE::OK;

}


REGISTE_REQ_API(RTSP_METHOD::RM_SETUP, SetupRequest);