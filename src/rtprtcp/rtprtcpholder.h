
#ifndef __H_RTPRTCPHOLDER_H__
#define __H_RTPRTCPHOLDER_H__

#include "rtpsink.h"
#include "rtcpsink.h"
#include "rtpinfo.h"
#include "../mediasource/trackbase.h"
#include "../mediasource/streambase.h"


class TrackSink {    
public:
    TrackSink(TrackBase*, const HTcpSocket& sock, HUN channel_id1, HUN channel_id2) noexcept;
    TrackSink(TrackBase*, HSTR strIp, HN rtp_port) noexcept;

    ~TrackSink() noexcept;

public:
    TRACK_TYPE GetTrackType() const noexcept;

    void SendRtpPack(const MediaPacket& packet);

    const TrackBase* GetTrackInfo() const noexcept { return m_rtp_sink->GetTrack(); }

    const RtpBase* GetRtpSink() const noexcept { return m_rtp_sink; }

    void SendRtcpReport();

    bool IsInvalidChannelId(HUN channelId) const noexcept;

private:
    RtpTransportMode m_rtp_mode;
    RtcpTransportMode m_rtcp_mode;
    RtpBase* m_rtp_sink;
    RtcpBase* m_rtcp_sink;
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

    bool IsInvalidChannelId(HUN channelId) const noexcept;

public:
    void SetClientIp(const HIp4Addr& addr);

    bool AddTrackSink(TrackBase*, HN port);

    bool AddTrackSink(TrackBase*, const HTcpSocket& sock, HUN channel_id1, HUN channel_id2);

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
