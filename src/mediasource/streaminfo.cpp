

#include "streaminfo.h"


MediaPacket::MediaPacket() noexcept
    : m_index(0), m_stream_index(0),
    m_pts(0), m_dts(0), m_duration(0), m_pos(), m_ffm_flags(-1),
    m_rtp_pts(0), m_real_duration(0), 
    m_track_type(TRACK_TYPE::UNKNOWN), m_timebase({0,0}),
    m_nalu_type(NaluType::NotNulu),
    m_ref() {

}


MediaPacket::~MediaPacket() noexcept {

}


void MediaPacket::Reset() {

    m_index = 0;
    m_stream_index = 0;
    m_pts = m_dts = m_duration = m_pos = 0;
    m_ffm_flags = -1;
    m_rtp_pts = 0;
    m_real_duration = 0;
    m_track_type = TRACK_TYPE::UNKNOWN;
    m_timebase.den = 1;
    m_timebase.num = 0;
    m_nalu_type = NaluType::NotNulu;

    m_ref.Reset();

}


void MediaPacket::SetTimeBase(HN num, HN den) noexcept { 

    m_timebase.num = num; 
    m_timebase.den = den;
    if (m_timebase.den == 0) {
        m_timebase.den = 1;
    }

}


void MediaPacket::SetData(HCPTR data, HN len) noexcept {

    m_ref.Assign(data, len);

}