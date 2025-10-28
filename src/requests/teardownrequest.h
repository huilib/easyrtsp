

#ifndef __H_TEAR_REQUEST_H__
#define __H_TEAR_REQUEST_H__

#include "safertsprequest.h"

class TearRequest : public SafeRtspRequest {
private:
    using base_class_t = SafeRtspRequest;

public:
    TearRequest(RtspRequestHeader&& header) noexcept;

    ~TearRequest();


public:
    RTSP_REQUEST_CODE doWork(HIOBuffer&) override;
};



#endif // __H_TEAR_REQUEST_H__