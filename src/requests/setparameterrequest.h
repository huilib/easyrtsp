

#ifndef __H_SETPARAMETER_REQUEST_H__
#define __H_SETPARAMETER_REQUEST_H__

#include "safertsprequest.h"

class SetParameterRequest : public SafeRtspRequest {
private:
    using base_class_t = SafeRtspRequest;

public:
    SetParameterRequest(RtspRequestHeader&& header) noexcept;

    ~SetParameterRequest();
public:
    RTSP_REQUEST_CODE doWork(HIOBuffer&);
};



#endif // __H_SETPARAMETER_REQUEST_H__