

#ifndef __H_SETUP_REQUEST_H__
#define __H_SETUP_REQUEST_H__

#include "safertsprequest.h"

class SetupRequest : public SafeRtspRequest {
private:
    using base_class_t = SafeRtspRequest;

public:
    SetupRequest(RtspRequestHeader&& header) noexcept;

    ~SetupRequest();


public:
    RTSP_REQUEST_CODE doWork(HIOBuffer&) override;

private:
    RTSP_REQUEST_CODE do_tcp(HIOBuffer&);

    RTSP_REQUEST_CODE do_udp(HIOBuffer&);

};



#endif // __H_SETUP_REQUEST_H__