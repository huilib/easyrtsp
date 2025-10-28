

#ifndef __H_DESCRIBE_REQUEST_H__
#define __H_DESCRIBE_REQUEST_H__

#include "safertsprequest.h"

class DescribeRequest : public SafeRtspRequest {
private:
    using base_class_t = SafeRtspRequest;

public:
    DescribeRequest(RtspRequestHeader&& header) noexcept;

    ~DescribeRequest();

public:
    RTSP_REQUEST_CODE doWork(HIOBuffer&);

private:
    HSTR generateSDP() const;

};



#endif // __H_DESCRIBE_REQUEST_H__