

#ifndef __H_PLAY_REQUEST_H__
#define __H_PLAY_REQUEST_H__


#include "safertsprequest.h"

class PlayRequest : public SafeRtspRequest {
private:
    using base_class_t = SafeRtspRequest;

public:
    PlayRequest(RtspRequestHeader&& header) noexcept;

    ~PlayRequest();


public:
    RTSP_REQUEST_CODE doWork(HIOBuffer&) override;
};



#endif // __H_SETUP_REQUEST_H__