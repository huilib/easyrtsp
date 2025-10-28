

#include "rtprtcpholder.h"
#include <hlog.h>

TrackSink::TrackSink(TrackBase* tb, const HTcpSocket& sock, HUN channel_id) noexcept 
    : m_rtp_mode(RtpTransportMode::RTP_TCP),
    m_rtcp_mode(RtcpTransportMode::INDENPENDENT),
    m_rtp_sink(nullptr), m_rtcp_sink(nullptr){
    
    TcpRtpBase* tcp_rtp = nullptr;
    if (tb->GetTrackType() == TRACK_TYPE::VIDEO) {
        tcp_rtp = new H264TcpRtpSink(tb, sock);
    }else {
        tcp_rtp = new AacTcpRtpSink(tb, sock);
    }

    tcp_rtp->SetChannelId(channel_id);
    
    m_rtp_sink = tcp_rtp;

    m_rtcp_sink = new TcpRtcpSink(tb, sock, tcp_rtp);

}

TrackSink::TrackSink(TrackBase* tb, HSTR strIp, HN rtp_port) noexcept
    : m_rtp_mode(RtpTransportMode::RTP_UDP),
    m_rtcp_mode(RtcpTransportMode::INDENPENDENT),
    m_rtp_sink(nullptr), m_rtcp_sink(nullptr) {

    HIp4Addr rtp_addr(strIp, rtp_port);
    if (tb->GetTrackType() == TRACK_TYPE::VIDEO) {
        m_rtp_sink = new H264RtpSink(tb, rtp_addr);
    }else {
        m_rtp_sink = new AacRtpSink(tb, rtp_addr);
    }

    HIp4Addr rtcp_addr(strIp, rtp_port+1);
    m_rtcp_sink = new RtcpSink(tb, rtcp_addr, m_rtp_sink);

}


TrackSink::~TrackSink() noexcept {

    HDELP(m_rtcp_sink);

    HDELP(m_rtp_sink);

}


TRACK_TYPE TrackSink::GetTrackType() const noexcept {


    if (m_rtp_sink == nullptr) {
        return TRACK_TYPE::UNKNOWN;
    }

    return m_rtp_sink->GetTrackType();

}


void TrackSink::SendRtpPack(const MediaPacket& packet) {

    m_rtp_sink->SendPack(packet);

}


void TrackSink::SendRtcpReport() {

    m_rtcp_sink->SendReport();

}


RtpRtcpHolder::RtpRtcpHolder() noexcept
    : m_client_addr(),
    m_track_mode(TrackTransportMode::INDEPENDENT),
    m_rtp_tran_mode(RtpTransportMode::RTP_UNKNOWN),
    m_track_sinks() {

}


RtpRtcpHolder::~RtpRtcpHolder() noexcept {

    for(std::vector<TrackSink*>::size_type i = 0; i < m_track_sinks.size(); ++i) {

        TrackSink* ts = m_track_sinks[i];

        HDELP(ts);

    }

    m_track_sinks.clear();

}


void RtpRtcpHolder::SendPack(const MediaPacket& packet) {

    TrackSink* ts = get_sink(packet.GetTrackType());

    if (ts == nullptr) {
        LOG_WARNING("ts is nullptr");
    }

    ts->SendRtpPack(packet);

}


void RtpRtcpHolder::SendSRSD() {

    //LOG_NORMAL("track sink size[%d]", m_track_sinks.size());

    for(std::vector<TrackSink*>::size_type i = 0; i < m_track_sinks.size(); ++i) {

        TrackSink* ts = m_track_sinks[i];

        if(HUNLIKELY(ts == nullptr)) {
            continue;
        }

        ts->SendRtcpReport();

    }

    //FUN_END;
}


void RtpRtcpHolder::SetClientIp(const HIp4Addr& addr) { 

    HIGNORE_RETURN(m_client_addr.Assign(addr)); 

}


bool RtpRtcpHolder::AddTrackSink(TrackBase* tb, HN port) {

    for(std::vector<TrackSink*>::size_type i = 0; i < m_track_sinks.size(); ++i) {

        const TrackSink* ts = m_track_sinks[i];

        if (ts->GetTrackType() == tb->GetTrackType()) {
            return false;
        }

    }

    HSTR strIP;
    HN client_port;
    HIGNORE_RETURN(m_client_addr.GetAddrInfo(strIP, client_port));
    TrackSink* newsink = new TrackSink(tb, strIP, port);
    m_track_sinks.push_back(newsink);

    return true;

}


bool RtpRtcpHolder::AddTrackSink(TrackBase* tb, const HTcpSocket& sock, HUN channel_id) {

    for(std::vector<TrackSink*>::size_type i = 0; i < m_track_sinks.size(); ++i) {

        const TrackSink* ts = m_track_sinks[i];

        if (ts->GetTrackType() == tb->GetTrackType()) {
            return false;
        }

    }

    TrackSink* newsink = new TrackSink(tb, sock, channel_id);
    m_track_sinks.push_back(newsink);

    return true;

}


void RtpRtcpHolder::SentSenderReport() {



}


TrackSink* RtpRtcpHolder::get_sink(TRACK_TYPE tt) {

    for(std::vector<TrackSink*>::size_type i = 0; i < m_track_sinks.size(); ++i) {

        TrackSink* ts = m_track_sinks[i];

        if (ts->GetTrackType() == tt) {
            return ts;
        }

    }

    return nullptr;

}


