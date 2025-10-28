
#ifndef __H_STREAM_INFO_H__
#define __H_STREAM_INFO_H__

#include <huicpp.h>
#include <hrefbuffer.h>

#include "mediatypes.h"
#include "trackbase.h"

using namespace HUICPP;

// TODO: support in V1.6
enum class StreamRtpTransportType : HUCH {
    BOARDCAST,
    INDEPENDENT
};

enum class StreamState {
        CREATING,
        CONNECTED,
        PLAYING,
        STOPPED,
        TERMINAL,
        ERROR,
        UNKNOWN
};


enum class NaluType{
    NotNulu,
    Slice,
    DPA,
    DPB,
    DPC,
    IDR_SLICE,
    SEI,
    SPS,
    PPS,
    AUD,
    SEQUENCE,

    END
};


class MediaPacket {
public:
    MediaPacket() noexcept;

    ~MediaPacket() noexcept;

public:
    void Reset();

public:
    void SetIndex(HN index) noexcept { m_index = index; }
    void SetStreamIndex(HN sid) noexcept { m_stream_index = sid; }
    void SetPts(HULL pts) noexcept { m_pts = pts; }
    void SetDts(HULL dts) noexcept { m_dts = dts; }
    void SetDuration(HULL dura) noexcept { m_duration = dura; }
    void SetPos(HULL pos) noexcept { m_pos = pos; }
    void SetFfmFlags(HULL ffm_flags) noexcept { m_ffm_flags = ffm_flags; }
    void SetRtpPts(HULL pts) noexcept { m_rtp_pts = pts; }
    void SetRealDuration(HULL real_duration) noexcept { m_real_duration = real_duration; }
    void SetTrackType(TRACK_TYPE tt) noexcept { m_track_type = tt; }
    void SetTimeBase(HN num, HN den) noexcept;
    void SetNaluType(NaluType nt) noexcept { m_nalu_type = nt; }
    void SetData(HCPTR data, HN len) noexcept;

    HN GetIndex() const noexcept { return m_index; }
    HN GetStreamIndex() const noexcept { return m_stream_index; }
    HULL GetPts() const noexcept { return m_pts; }
    HULL GetDts() const noexcept { return m_dts; }
    HULL GetDuration() const noexcept { return m_duration; }
    HULL GetPos() const noexcept { return m_pos; }
    HN GetFfmFlags() const noexcept { return m_ffm_flags; }
    HULL GetRtpPts() const noexcept { return m_rtp_pts; }
    HULL GetRealDuration() const noexcept { return m_real_duration; }
    TRACK_TYPE GetTrackType() const noexcept { return m_track_type; }
    const TimeBase& GetTimebase() const noexcept { return m_timebase; }
    NaluType GetNaluType() const noexcept { return m_nalu_type; }
    const HRefBuffer& GetRefBuffer() const noexcept { return m_ref; }

private:
    HN m_index;
    HN m_stream_index;
    HULL m_pts;
    HULL m_dts;
    HULL m_duration;
    HULL m_pos;
    HN m_ffm_flags;

    HULL m_rtp_pts;
    HULL m_real_duration;
    TRACK_TYPE m_track_type;
    TimeBase m_timebase;
    NaluType m_nalu_type;
    HRefBuffer m_ref;    
};


class StreamInfo{
public:
    StreamInfo() noexcept = default;
    ~StreamInfo() noexcept = default;

public:
    HCSTRR GetName() const noexcept { return m_strName; }
    HULL GetStartTime() const noexcept { return m_start_time; }
    HULL GetDuration() const noexcept { return m_duration; }
    HULL GetBitRate() const noexcept { return m_bit_rate; }
    HULL GetStartRealTime() const noexcept { return m_start_realtime; }

    void SetName(HCSTRR strName) noexcept { m_strName = strName; }
    void SetStartTime(HULL start_time) noexcept { m_start_time = start_time; }
    void SetDuration(HULL duration) noexcept { m_duration = duration; }
    void SetBitRate(HULL bit_rate) noexcept { m_bit_rate = bit_rate; }
    void SetStartRealTime(HULL startRealtime) noexcept { m_start_realtime = startRealtime; }

private:
    HSTR m_strName;
    HULL m_start_time;
    HULL m_duration;
    HULL m_bit_rate;
    HULL m_start_realtime;
};


class StreamObserver {
public:
    StreamObserver() noexcept = default;

    virtual ~StreamObserver() noexcept = default;

public:
    virtual void OnStreamConnected(const StreamInfo&, const std::vector<TrackBase*>&) = 0;

    virtual void OnStreamBeginPlay() = 0;

    virtual void OnStreamPlayingContinue(const MediaPacket& packet) = 0;

    virtual void OnStreamStop() { }

    virtual void OnStreamTerminal() {  }

    virtual void OnStreamError() {  }

};


#endif //__H_STREAM_INFO_H__