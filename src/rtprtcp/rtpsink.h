

#ifndef __H_RTPRTCP_RTPSINK_H__
#define __H_RTPRTCP_RTPSINK_H__

#include <houtputpacketbuffer.h>

#include "rtprtcpbase.h"


using namespace HUICPP;

class UdpRtpSink : public RtpBase, public UdpSinkBase {
protected:
    using rtp_base_t = RtpBase;
    using sink_base_t = UdpSinkBase;
    static constexpr const HN MAX_RTP_LENGTH = 1400;

public:
    UdpRtpSink(TrackBase* tb, const HIp4Addr& addr, HUSN payload_type);

    virtual ~UdpRtpSink() noexcept;

public:
    void SendPack(const MediaPacket& packet) override;

public:  
    HUN GetRtcpNtpRtpTimestamp() const;

public:
    virtual HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept = 0;

private:   
    const HSocket& getSocket() const noexcept override;
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


class H264UdpRtpSink : public UdpRtpSink {
private:
    using base_class_t = UdpRtpSink;

public:
    H264UdpRtpSink(TrackBase* tb, const HIp4Addr& addr);

    ~H264UdpRtpSink() noexcept;

protected:
    HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept override;

    
};


class AacUdpRtpSink : public UdpRtpSink {
private:
    using base_class_t = UdpRtpSink;

public:
    AacUdpRtpSink(TrackBase* tb, const HIp4Addr& addr);

    ~AacUdpRtpSink() noexcept;

protected:
    HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept override;
    
};



class TcpRtpSink : public RtpBase, public TcpSinkBase {
private:
    using rtp_base_t = RtpBase;
    using tcp_sink_base_t = TcpSinkBase;

public:
    TcpRtpSink(TrackBase* tb, const HTcpSocket& sock, HUN channel_id, HUSN payload_type) noexcept;

    virtual ~TcpRtpSink() noexcept;

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


class H264TcpRtpSink : public TcpRtpSink {
private:
    using base_class_t = TcpRtpSink;

public:
    H264TcpRtpSink(TrackBase* tb, const HTcpSocket& sock, HUN channel_id);

    ~H264TcpRtpSink() noexcept;

protected:
    HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept override;

    
};


class AacTcpRtpSink : public TcpRtpSink {
private:
    using base_class_t = TcpRtpSink;

public:
    AacTcpRtpSink(TrackBase* tb, const HTcpSocket& sock, HUN channel_id);

    ~AacTcpRtpSink() noexcept;

protected:
    HUN GetRtpTimestamp(const MediaPacket& packet) const noexcept override;
    
};




#endif //__H_RTPRTCP_RTPSINK_H__