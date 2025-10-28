

#ifndef __H_RTPRTCP_RTPSINK_H__
#define __H_RTPRTCP_RTPSINK_H__

#include <houtputpacketbuffer.h>

#include "rtprtcpbase.h"


using namespace HUICPP;

using rtp_sink_base = UdpSinkBase<tagRtp>;
class RtpSink : public rtp_sink_base {
protected:
    using sink_bass_t = rtp_sink_base;
    static constexpr const HN MAX_RTP_LENGTH = 1400;
    static unsigned char START_KEY4[];
    static unsigned char AUDIO_START4[];

public:
    RtpSink(TrackBase* tb, const HIp4Addr& addr, HUSN payload_type);

    virtual ~RtpSink() noexcept;

public:
    void SendPack(const MediaPacket& packet) override;

public:  
    HUN GetRtcpNtpRtpTimestamp() const;

public:
    virtual HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept = 0;

private:   
    const HUdpSock& getSocket() const override;
    void send_single_nalu_uint(const MediaPacket& packet);
    void send_fregmentation_nalu_uints(const MediaPacket& packet);

    void common_rtsp_header(const MediaPacket& packet, HIOOutputBuffer&, bool setMarket = true);
    HSocket::size_type send_buffer(HIOOutputBuffer&);

private:
    HUCH m_payload_type;

protected:
    HUN m_last_rtp;
    HULL m_last_rtp_time;
};


class H264RtpSink : public RtpSink {
private:
    using base_class_t = RtpSink;

public:
    H264RtpSink(TrackBase* tb, const HIp4Addr& addr);

    ~H264RtpSink() noexcept;

protected:
    HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept override;

    
};


class AacRtpSink : public RtpSink {
private:
    using base_class_t = RtpSink;

public:
    AacRtpSink(TrackBase* tb, const HIp4Addr& addr);

    ~AacRtpSink() noexcept;

protected:
    HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept override;
    
};


using tcp_rtp_sink_base = TcpSinkBase<tagRtp>;
class TcpRtpBase : public tcp_rtp_sink_base {
private:
    using base_class_t = tcp_rtp_sink_base;
    static unsigned char TCP_AUDIO_START4[];

public:
    TcpRtpBase(TrackBase* tb, const HTcpSocket& sock, HUSN payload_type) noexcept;

    virtual ~TcpRtpBase() noexcept;

public:
    void SendPack(const MediaPacket& packet);

public:
    HUN GetRtcpNtpRtpTimestamp() const override;

public:
    virtual HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept = 0;

private:
    void common_rtsp_header(const MediaPacket& packet, HIOOutputBuffer&, bool setMarket = true);
    void send_over_tcp(const MediaPacket& packet);
    HSocket::size_type send_buffer(HIOOutputBuffer&);

private:
    HUCH m_payload_type;

protected:
    HUN m_last_rtp;
    HULL m_last_rtp_time;
};


class H264TcpRtpSink : public TcpRtpBase {
private:
    using base_class_t = TcpRtpBase;

public:
    H264TcpRtpSink(TrackBase* tb, const HTcpSocket& sock);

    ~H264TcpRtpSink() noexcept;

protected:
    HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept override;

    
};


class AacTcpRtpSink : public TcpRtpBase {
private:
    using base_class_t = TcpRtpBase;

public:
    AacTcpRtpSink(TrackBase* tb, const HTcpSocket& sock);

    ~AacTcpRtpSink() noexcept;

protected:
    HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept override;
    
};





#endif //__H_RTPRTCP_RTPSINK_H__