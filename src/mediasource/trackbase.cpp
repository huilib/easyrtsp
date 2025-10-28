

#include "trackbase.h"
#include <hstr.h>
#include <hlog.h>

/*
TrackStasticsInfo::TrackStasticsInfo() noexcept
    : m_frame_count(0), 
    m_byte_count(0), m_begin_time() {


}


void TrackStasticsInfo::Begin() {

    m_frame_count = 0;
    m_byte_count = 0;
    m_begin_time.Touch();

}


void TrackStasticsInfo::InoPacket(const MediaPacket& packet) {

    ++m_frame_count;
    m_byte_count += packet.GetRefBuffer().GetLength();

}*/



TrackBase::TrackBase() noexcept
    : m_track_id(-1), m_strName(),
    m_track_type(TRACK_TYPE::UNKNOWN),
    m_codec_id(MediaCodecId::Unknown),
    m_codec_module_id(MediaCodecModuleId::None),
    m_bitrate(0), m_frequency(0), 
    m_foramt(0), m_profile(0), m_profile_level(0),
    m_timebase(), 
    m_rtp_socket(), m_rtcp_socket() {

}



TrackBase::~TrackBase() noexcept {

}


TrackBase& TrackBase::SetupName() {

    m_strName = HStr::Format("track%d", m_track_id);
    
    return *this;

}


HRET TrackBase::Init(HN port_rtp, HN port_rtcp) {

    //LOG_NORMAL("trackbase init rtpport[%d] rtcpport[%d]", port_rtp, port_rtcp);

    // initialize rtp socket
    HNOTOK_MSG_RETURN(m_rtp_socket.Init(), "init rtp socket failed");

    HNOTOK_MSG_RETURN(m_rtp_socket.SetReuseAddr(), "set rtp socket reuseaddr failed");

    HIp4Addr addrrtp("0.0.0.0", port_rtp);
    HNOTOK_MSG_RETURN(m_rtp_socket.Bind(addrrtp), "bind rtp socket failed");

    HNOTOK_MSG_RETURN(m_rtp_socket.SetupSendBufLength(256 * 1024), "setup rtp socket sned buffer failed");


    // initialize rtcp socket
    HNOTOK_MSG_RETURN(m_rtcp_socket.Init(), "init rtcp socket failed");

    HNOTOK_MSG_RETURN(m_rtcp_socket.SetReuseAddr(), "set rtcp socket reuseaddr failed");

    HIp4Addr addrrtcp("0.0.0.0", port_rtcp);
    HNOTOK_MSG_RETURN(m_rtcp_socket.Bind(addrrtcp), "bind rtcp socket failed");

    HRETURN_OK;

}


void TrackBase::SetTimeBase(HN num, HN den) noexcept {

    m_timebase.num = num;
    m_timebase.den = den;

    if (m_timebase.den == 0) {
        m_timebase.den = 1;
    }

}
    