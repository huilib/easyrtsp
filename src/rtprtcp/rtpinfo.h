
#ifndef __H_RTP_INFO_H__
#define __H_RTP_INFO_H__

#include <huicpp.h>

using namespace HUICPP;

// TODO: for now, we only support independent mode.
enum class TrackTransportMode : HUCH{
    INDEPENDENT,
    BUNDLE,
    TS
};


enum class RtpTransportMode: HUCH {
    RTP_UNKNOWN,
    RTP_TCP,
    RTP_UDP,
    RTP_RAW
};

struct RtpTransportModeParse {
    static RtpTransportMode ParseRtpTransportMode(HCSTRR src);
};

enum class RtcpTransportMode : HUCH {
    INDENPENDENT,
    REUSE_RTP
};

class RtpInfo {
private:
    static constexpr const HN BASE_RTP_PT = 96;

public:
    static HN GetRtpPayploadType(HN track_id) noexcept { return BASE_RTP_PT + track_id; }
};

enum class RTP_PAYTYPE : HUCH {
    H264_PT = 96,
    AAC_PT = 97
};


#endif //__H_RTP_INFO_H__