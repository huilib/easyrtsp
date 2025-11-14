

#include "rtprtcpbase.h"
#include <hlog.h>

const HUCH RtpBase::START_KEY4[] = {0x00, 0x00, 0x00, 0x00};

RtpBase::RtpBase(TrackBase* tb)
    : m_timebase(our_random32()), m_ssrc(our_random32()), 
    m_seq(static_cast<HUSN>(our_random())), 
    m_packet_count(0), m_byte_count(0),
    m_tb(tb) {
}


RtpBase::~RtpBase() noexcept {

}


RtcpBase::RtcpBase(RtpBase* rtp) noexcept 
    : m_rtp(rtp) {

}


RtcpBase::~RtcpBase() noexcept {

}

TRACK_TYPE RtcpBase::GetTrackType() const noexcept {

    if (m_rtp == nullptr) {
        return TRACK_TYPE::UNKNOWN;
    }

    return m_rtp->GetTrackType();
}
    

const TrackBase* RtcpBase::GetTrack() const noexcept {

    if (m_rtp == nullptr) {
        return nullptr;
    }

    return m_rtp->GetTrack();

}

SinkBase::SinkBase() noexcept {

}


SinkBase::~SinkBase() noexcept {

}

UdpSinkBase::UdpSinkBase(const HIp4Addr& addr)
    : base_class_t(), m_toaddr(addr) {

}


UdpSinkBase::~UdpSinkBase() noexcept {

}


void UdpSinkBase::connect() const {

    HIGNORE_RETURN(getSocket().Connect(getAddr()));

}


const HSocket& UdpSinkBase::connectedAndGetSocket() const {

    connect();

    return getSocket();

}

TcpSinkBase::TcpSinkBase(const HTcpSocket& sock, HUN channel_id) noexcept
    : base_class_t(), m_sock(sock), m_channel_id(channel_id) {

}


TcpSinkBase::~TcpSinkBase() noexcept {

}

const HSocket& TcpSinkBase::getSocket() const noexcept {

    return m_sock;

}
