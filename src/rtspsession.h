

#ifndef __H_RTSP_SESSION_H__
#define __H_RTSP_SESSION_H__

#include <huicpp.h>
#include <htime.h>
#include "rtprtcp/rtprtcpholder.h"
#include "mediasource/streambase.h"
#include "scheduleobjectbase.h"
#include "sessionobserver.h"


using namespace HUICPP;

class StreamBase;
class RtspConnect;
class RtspSession : public ScheduleOBjectbase,
    public SessionObserver {
private:
    using schedule_base_t = ScheduleOBjectbase;
    using session_base_t = SessionObserver;    

public:
    explicit RtspSession(RtspConnect* connection);

    ~RtspSession();

public:
    HCSTRR GetSessionId() const noexcept { return m_strSessionId; }

    void SetVideoRtpPorts(HUN rtp_port, HUN rtcp_port);

    void SetAudioRtpPorts(HUN rtp_port, HUN rtcp_port);

    void SetTcpRtpChannel(TRACK_TYPE TT, HUN channelid1, HUN channelid2);

    bool CheckStream(StreamBase* stream) noexcept;
    StreamBase* GetStream() noexcept { return m_stream; }
    const StreamBase* GetStream() const noexcept { return m_stream; }

    bool IsPlayed() const noexcept { return m_bPlay; }

    void Play();

    void StopPlay();

    void OnPacketComing(const MediaPacket& packet) override;

    void OnRtcpActive() override;

    RtpRtcpHolder& GetRtpRtcpHolder() noexcept { return m_rtp_rtcp; }
    const RtpRtcpHolder& GetRtpRtcpHolder() const noexcept { return m_rtp_rtcp; }

    bool IsInvalidChannelId(HUN channelId) const noexcept;

private:
    void sent_packet(const MediaPacket& packet);

private:
    RtspConnect* m_connection;
    HSTR m_strSessionId;
    HTime m_last_active;

    StreamBase* m_stream;

    RtpRtcpHolder m_rtp_rtcp;

    bool m_bPlay;

    // send info.
    bool m_bNeedIFrame;
    
};


#endif // __H_RTSP_SESSION_H__