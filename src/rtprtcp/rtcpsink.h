

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

class UdpRtcpSink : public RtcpBase, public UdpSinkBase {
protected:
    using rtcp_base_t = RtcpBase;
    using sink_base_t = UdpSinkBase;

public:
    UdpRtcpSink(RtpBase* rtp, const HIp4Addr& addr);

    virtual ~UdpRtcpSink() noexcept;

public:
    void SendReport() override;    

private:
    const HSocket& getSocket() const noexcept override;

    void addSenderReport(HIOOutputBuffer&);

    void equeue_common_prefix(RTCP_PADLOAD_TYPE pt,  unsigned numExtraWords, HOutputPacketBuffer&);

    HSocket::size_type send_buffer(HIOOutputBuffer&) const;
};


class TcpRtcpSink : public RtcpBase, public TcpSinkBase {
protected:
    using rtcp_base_t = RtcpBase;
    using sink_base_t = TcpSinkBase;

public:
    TcpRtcpSink(RtpBase* rtp, const HTcpSocket& sock, HUN channel_id);

    virtual ~TcpRtcpSink() noexcept;

public:
    void SendReport() override;

private:
    void addSenderReport(HIOOutputBuffer&);

    void equeue_common_prefix(RTCP_PADLOAD_TYPE pt,  unsigned numExtraWords, HOutputPacketBuffer&);

    HSocket::size_type send_buffer(HIOOutputBuffer&);
};

#endif //__H_RTPRTCP_RTCPSINK_H__

