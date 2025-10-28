

#ifndef __H_GETPARAMETER_REQUEST_H__
#define __H_GETPARAMETER_REQUEST_H__

#include "safertsprequest.h"

class GetParameterRequest : public SafeRtspRequest {
private:
    using base_class_t = SafeRtspRequest;

public:
    GetParameterRequest(RtspRequestHeader&& header) noexcept;

    ~GetParameterRequest();
public:
    RTSP_REQUEST_CODE doWork(HIOBuffer&);
};



#endif // __H_GETPARAMETER_REQUEST_H__