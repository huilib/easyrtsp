

#ifndef __H_SDP_BASE_H__
#define __H_SDP_BASE_H__

#include <huicpp.h>

using namespace HUICPP;

class SdpBase {
private:
    static constexpr const HN BASE_RTP_PT = 96;

public:
    SdpBase() = default;

    virtual ~SdpBase() = default;

public:
    virtual HSTR MediaDescript() const = 0;

    HN GetRtpPayploadType(HN track_id) const noexcept { return BASE_RTP_PT + track_id; }

};

#endif // __H_SDP_BASE_H__