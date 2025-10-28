

#ifndef __H_RTPRTCP_RTCPSINK_H__
#define __H_RTPRTCP_RTCPSINK_H__

#include "rtprtcpbase.h"
#include "rtpsink.h"
#include "../scheduleobjectbase.h"
#include <type_traits>

enum class RTCP_PADLOAD_TYPE : HUCH {
    SR = 200,
    RR = 201,
    SDES = 202,
    BYE = 203,
    APP = 204,
    RTPFB = 205,        // Generic RTP Feedback [RFC4585]
    PSFB = 206,         // Payload-specific [RFC4585]
    XR = 207,           // extended report [RFC3611]
    AVB = 208,          // AVB RTCP packet ["Standard for Layer 3 Transport Protocol for Time Sensitive Applications in Local Area Networks." Work in progress.]
    RSI = 209,          // Receiver Summary Information [RFC5760]
    TOKEN = 210,        // Port Mapping [RFC6284]
    IDMS = 211,         // IDMS Settings [RFC7272]
};


using rtcp_sink_base = UdpSinkBase<tagRtcp>;

class RtcpSink : public rtcp_sink_base {
protected:
    using base_class_t = rtcp_sink_base;
    using rtp_class_t = mid_sink_base<tagRtp>;

public:
    RtcpSink(TrackBase* tb, const HIp4Addr& addr, rtp_class_t* rtp_sink);

    virtual ~RtcpSink() noexcept;

public:
    void SendReport() override;    

private:
    const HUdpSock& getSocket() const override;

    void addSenderReport(HIOOutputBuffer&);

    void equeue_common_prefix(RTCP_PADLOAD_TYPE pt,  unsigned numExtraWords, HOutputPacketBuffer&);

    HSocket::size_type send_buffer(HIOOutputBuffer&);

private:    
    rtp_class_t* m_rtp_sink;
};


using tcp_rtcp_sink_base = TcpSinkBase<tagRtcp>;
class TcpRtcpSink : public tcp_rtcp_sink_base {
protected:
    using base_class_t = tcp_rtcp_sink_base;

public:
    TcpRtcpSink(TrackBase* tb, const HTcpSocket& sock, TcpRtpBase* rtp_sink);

    virtual ~TcpRtcpSink() noexcept;

public:
    void SendReport() override;

private:
    void addSenderReport(HIOOutputBuffer&);

    void equeue_common_prefix(RTCP_PADLOAD_TYPE pt,  unsigned numExtraWords, HOutputPacketBuffer&);

    HSocket::size_type send_buffer(HIOOutputBuffer&);

private:
    TcpRtpBase* m_rtp_sink;
};

#endif //__H_RTPRTCP_RTCPSINK_H__