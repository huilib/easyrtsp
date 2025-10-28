
#ifndef __H_RTPRTCPHOLDER_H__
#define __H_RTPRTCPHOLDER_H__

#include "rtpsink.h"
#include "rtcpsink.h"
#include "rtpinfo.h"
#include "../mediasource/trackbase.h"
#include "../mediasource/streambase.h"

using rtp_class_t = mid_sink_base<tagRtp>;
using rtcp_class_t = mid_sink_base<tagRtcp>;

class TrackSink {    
public:
    TrackSink(TrackBase*, const HTcpSocket& sock, HUN channel_id) noexcept;
    TrackSink(TrackBase*, HSTR strIp, HN rtp_port) noexcept;

    ~TrackSink() noexcept;

public:
    TRACK_TYPE GetTrackType() const noexcept;

    void SendRtpPack(const MediaPacket& packet);

    const TrackBase* GetTrackInfo() const noexcept { return m_rtp_sink->GetTrack(); }

    const rtp_class_t* GetRtpSink() const noexcept { return m_rtp_sink; }

    void SendRtcpReport();

private:
    RtpTransportMode m_rtp_mode;
    RtcpTransportMode m_rtcp_mode;
    rtp_class_t* m_rtp_sink;
    rtcp_class_t* m_rtcp_sink;
};

// 有可能是tcp-rtp复用, 有可能是rtp-rtcp复用.
// TODO: holder需要根据setup设置是否需要rtcp, 以及复用方式
class RtpRtcpHolder {
public:
    RtpRtcpHolder() noexcept;

    ~RtpRtcpHolder() noexcept;

public:
    void SendPack(const MediaPacket& packet);

    void SendSRSD();

    RtpTransportMode GetTransportMode() const noexcept { return m_rtp_tran_mode; }

    void SetTransportMode(RtpTransportMode mode) noexcept { m_rtp_tran_mode = mode; }

public:
    void SetClientIp(const HIp4Addr& addr);

    bool AddTrackSink(TrackBase*, HN port);

    bool AddTrackSink(TrackBase*, const HTcpSocket& sock, HUN channel_id);

    void SentSenderReport();

    const std::vector<TrackSink*>& GetTrackSinks() const noexcept { return m_track_sinks; }

private:
    TrackSink* get_sink(TRACK_TYPE);

private:
    HIp4Addr m_client_addr;
    TrackTransportMode m_track_mode;
    RtpTransportMode m_rtp_tran_mode;
    std::vector<TrackSink*> m_track_sinks;
};

#endif //__H_RTPRTCPHOLDER_H__
