

#ifndef __H_RTPRTCP_BASE_H__
#define __H_RTPRTCP_BASE_H__

#include <huicpp.h>
#include "../mediasource/trackbase.h"
#include "rtpinfo.h"
#include "../mediasource/streaminfo.h"
#include "../utils/ranapi.h"

using namespace HUICPP;

class RtpBase {
public:
    static const HUCH START_KEY4[];
public:
    RtpBase(TrackBase* tb);

    virtual ~RtpBase() noexcept;

public:
    virtual void SendPack(const MediaPacket& packet) = 0;

public:
    TRACK_TYPE GetTrackType() const noexcept { return m_tb->GetTrackType(); }
    const TrackBase* GetTrack() const noexcept { return m_tb; }

    HUN GetTimebase() const noexcept { return m_timebase; }
    HUN GetSSRC() const noexcept { return m_ssrc; }
    HUSN GetSequence() const noexcept { return m_seq; }

    void IncreSequence() noexcept { ++m_seq; }
    void SetSequence(HUSN seq) noexcept { m_seq = seq; }

    HUN GetPacketCount() const noexcept { return m_packet_count; }
    HUN GetByteCount() const noexcept { return m_byte_count; }
    void IncrePacketCount() noexcept { ++m_packet_count; }
    void IncreByteCount(HN nbytes) noexcept { m_byte_count += nbytes; }

    virtual HUN GetRtcpNtpRtpTimestamp() const = 0;

private:
    const HUN m_timebase;
    const HUN m_ssrc;
    HUSN m_seq;

    // statistics information
    HUN m_packet_count;
    HUN m_byte_count;

    TrackBase* m_tb;
};

class RtcpBase {
protected:
    RtcpBase(RtpBase* rtp) noexcept;

public:
    virtual ~RtcpBase() noexcept;

public:
    virtual void SendReport() = 0;

public:
    TRACK_TYPE GetTrackType() const noexcept;
    const TrackBase* GetTrack() const noexcept;
    const RtpBase* GetRtp() const noexcept { return m_rtp; }

private:
    RtpBase* m_rtp;
};

class SinkBase {
protected:
    SinkBase() noexcept;

public:
    virtual ~SinkBase() noexcept;

public:
    virtual const HSocket& getSocket() const noexcept = 0;
    
};

class UdpSinkBase: public SinkBase {
private:
    using base_class_t = SinkBase;

protected:
    UdpSinkBase(const HIp4Addr& addr);

public:
    virtual ~UdpSinkBase() noexcept;
    
protected:
    void connect() const;

    const HSocket& connectedAndGetSocket() const;

    const HIp4Addr& getAddr() const noexcept { return m_toaddr; }

private:
    HIp4Addr m_toaddr;
};


class TcpSinkBase : public SinkBase {
private:
    using base_class_t = SinkBase;

public:
    TcpSinkBase(const HTcpSocket& sock, HUN channel_id) noexcept;

    virtual ~TcpSinkBase() noexcept;

public:
    HUN GetChannelId() const noexcept { return m_channel_id; }
    void SetChannelId(HUN channel_id) noexcept { m_channel_id = channel_id; }
    
protected:
    const HSocket& getSocket() const noexcept override;

private:
    const HTcpSocket& m_sock;
    HUN m_channel_id;
};




#endif // __H_RTPRTCP_BASE_H__