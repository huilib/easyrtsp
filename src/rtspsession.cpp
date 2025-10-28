
#include "rtspserver.h"
#include "rtspconnect.h"
#include "rtspsession.h"
#include "utils/ranapi.h"
#include "utils/h264packets.h"
#include "mediasource/rtspstream.h"
#include "rtprtcp/rtpinfo.h"

#include <hsocket.h>
#include <hstr.h>
#include <hlog.h>


RtspSession::RtspSession(RtspConnect* connection)
    : schedule_base_t(), session_base_t(),
    m_connection(connection), m_strSessionId(), m_last_active(),
    m_stream(nullptr),
    m_rtp_rtcp(),
    m_bPlay(false), 
    m_bNeedIFrame(true)  {

    HUN session_id = our_random32();
    m_strSessionId = HStr::Format("%08X", session_id);    

    m_rtp_rtcp.SetClientIp(m_connection->GetClientIp());

}


RtspSession::~RtspSession() {

    //m_stream->RemoveOb(this);

}


void RtspSession::SetVideoRtpPorts(HUN rtp_port, HUN) {

    m_rtp_rtcp.AddTrackSink(m_stream->GetTrack(TRACK_TYPE::VIDEO), rtp_port);

}


void RtspSession::SetAudioRtpPorts(HUN rtp_port, HUN) {

    m_rtp_rtcp.AddTrackSink(m_stream->GetTrack(TRACK_TYPE::AUDIO), rtp_port);


}


void RtspSession::SetTcpRtpChannel(TRACK_TYPE tt, HUN channelid) {

    m_rtp_rtcp.AddTrackSink(m_stream->GetTrack(tt), m_connection->GetRtspSocket(), 
        channelid);

}


bool RtspSession::CheckStream(StreamBase* stream) noexcept {

    if (m_stream == nullptr) {
        m_stream = stream;
        return true;
    }

    if (m_stream != nullptr) {
        return m_stream->GetName() == stream->GetName();
    }

    return false;

}


void RtspSession::Play() {

    m_bPlay = true;

    // bind: session => stream;
    m_stream->AddOb(this);

}


void RtspSession::StopPlay() {

    m_bPlay = false;

    m_stream->RemoveOb(this);

}


void RtspSession::OnPacketComing(const MediaPacket& packet) {

    sent_packet(packet);

}


void RtspSession::OnRtcpActive() {

    if (not m_bPlay) {
        return;
    }

    m_rtp_rtcp.SendSRSD();

}


void RtspSession::sent_packet(const MediaPacket& packet) {

    if (packet.GetTrackType() != TRACK_TYPE::VIDEO and
        packet.GetTrackType() != TRACK_TYPE::AUDIO) {
        return;
    }

    if (m_bNeedIFrame) {
        if (packet.GetTrackType() == TRACK_TYPE::VIDEO) {
            if (packet.GetNaluType() == NaluType::SPS or 
                packet.GetNaluType() == NaluType::IDR_SLICE) {
                m_bNeedIFrame = false;
            } else {
                return ;
            }
        } else {
            return ;
        }
    }

    m_rtp_rtcp.SendPack(packet);
    //sent_nalu(packet);    

}





