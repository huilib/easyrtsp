

#ifndef __H_TRACKBASE_H__
#define __H_TRACKBASE_H__

#include <huicpp.h>
#include <hsocket.h>

#include "mediatypes.h"

using namespace HUICPP;


enum class TRACK_TYPE {
    VIDEO           = 0,
    AUDIO           = 1,
    SUBTITLE,
    DATA,
    UNKNOWN
};


enum class MediaCodecId {
    H264,
    H265,
    Vp8,
    Vp9,
    Flv,
    Aac,
    Mp3,
    Opus,
    Jpeg,
    Png,
    Webp,
    Unknown
};


enum class MediaCodecModuleId {
    None = 0,
    DEFAULT,	// SW
    OPENH264,	// SW
    BEAMR,		// SW
    X264,		// SW		
    NVENC,		// HW
    QSV,		// HW
    XMA,		// HW
    NILOGAN,	// HW
    LIBVPX,		// SW
    FDKAAC,		// SW
    LIBOPUS,	// SW
    NB
};


/*
class TrackStasticsInfo{
public:
    TrackStasticsInfo() noexcept;
    ~ TrackStasticsInfo() noexcept = default;

public:
    void Begin();

    void InoPacket(const MediaPacket& packet);

private:
    HUN m_frame_count;
    HULL m_byte_count;
    HTime m_begin_time;
};*/


/*
    description for a stream.
*/
class TrackBase {
public:
    TrackBase() noexcept;

    virtual ~ TrackBase() noexcept;

public:
    virtual HSTR GenerateSDP() const = 0;

    TrackBase& SetupName();

public:
    HRET Init(HN port_rtp, HN port_rtcp);

public:
    void SetTrackId(HN track_id) noexcept { m_track_id = track_id; }
    void SetTrackType(TRACK_TYPE tt) noexcept { m_track_type = tt; }
    void SetCodecId(MediaCodecId mci) noexcept { m_codec_id = mci; }
    void SetCodecModuleId(MediaCodecModuleId mcmi) noexcept { m_codec_module_id = mcmi; }
    void SetBitRate(HN br) noexcept { m_bitrate = br; } 
    void SetFrequency(HN frequency) noexcept { m_frequency = frequency; } 
    void SetFormat(HN format) noexcept { m_foramt = format; }
    void SetProfile(HN profile) noexcept { m_profile = profile; }
    void SetProfileLevel(HN profile_level) noexcept { m_profile_level = profile_level; }
    void SetTimeBase(HN num, HN den) noexcept;

    HN GetTrackId() const noexcept { return m_track_id; } 
    HCSTRR GetName() const noexcept { return m_strName; }   
    TRACK_TYPE GetTrackType () const noexcept { return m_track_type; }
    MediaCodecId GetCodecId() const noexcept { return m_codec_id; }
    MediaCodecModuleId GetCodecModuleId() const noexcept { return m_codec_module_id; }
    HN GetBitRate() const noexcept { return m_bitrate; }    
    HN GetFrequency() const noexcept { return m_frequency; } 
    HN GetFormat() const noexcept { return m_foramt; }
    HN GetProfile() const noexcept { return m_profile; }
    HN GetProfileLevel() const noexcept { return m_profile_level; }
    const TimeBase& GetTimebase() const noexcept { return m_timebase; }

public:
    const HUdpSock& GetRtpSocket() const noexcept { return m_rtp_socket; }
    const HUdpSock& GetRtcpSocket() const noexcept { return m_rtcp_socket; }

public:
    HN m_track_id;                          // stream id.
    HSTR m_strName;
    TRACK_TYPE m_track_type;    
    MediaCodecId m_codec_id;
    MediaCodecModuleId m_codec_module_id;    
    HN m_bitrate;
    HN m_frequency;
    HN m_foramt;
    HN m_profile;
    HN m_profile_level;
    TimeBase m_timebase;

    // stastic information.
    //TrackStasticsInfo m_stastics;

    // for rtp
    HUdpSock m_rtp_socket;
    HUdpSock m_rtcp_socket;
};


#endif //__H_TRACKBASE_H__