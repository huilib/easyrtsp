

#include "rtpinfo.h"
#include <hstr.h>

RtpTransportMode RtpTransportModeParse::ParseRtpTransportMode(HCSTRR src) {

    HStr str(src);
    str.Upper();

    HStr::size_type pos = str.find("RTP/AVP/TCP");
    if (pos != HStr::npos) {
        return RtpTransportMode::RTP_TCP;
    }

    pos = str.find("RTP/AVP");
    if (pos != HStr::npos) {
        return RtpTransportMode::RTP_UDP;
    }

    return RtpTransportMode::RTP_UNKNOWN;

}