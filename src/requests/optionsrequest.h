

#ifndef __H_OPTIONS_REQUEST_H__
#define __H_OPTIONS_REQUEST_H__

#include "safertsprequest.h"

class OptionsRequest final: public SafeRtspRequest {
private:
    using base_class_t = SafeRtspRequest;

public:
    OptionsRequest(RtspRequestHeader&& header) noexcept;

    ~OptionsRequest();

public:
    RTSP_REQUEST_CODE doWork(HIOBuffer& buffer) override;
};



#endif // __H_OPTIONS_REQUEST_H__