

#ifndef __H_SAFE_RTSP_REQUEST_H__
#define __H_SAFE_RTSP_REQUEST_H__

#include "rtsprequest.h"
#include "rtspprotocol.h"


class RtspStream;
class SafeRtspRequest : public RtspRequest{
private:
    using base_class_t = RtspRequest;

public:
    SafeRtspRequest(RtspRequestHeader&& header) noexcept;

    virtual ~SafeRtspRequest();

public:
    void Handle(HIOBuffer&) override;

protected:
    virtual RTSP_REQUEST_CODE doWork(HIOBuffer&) = 0;

protected:
    void response_common(HIOBuffer& buffer) const;

    void response_code(HIOBuffer&, RTSP_REQUEST_CODE);

private:
    void setRtspResponse(HCSTRR str, HIOBuffer& buffer) const;

protected:
    HSTR dateHeader() const;

public:
    const RtspStream* GetStream() const noexcept { return m_stream; }
    RtspStream* GetStream() noexcept { return m_stream; }

private:
    RtspStream* m_stream;
};


#endif //__H_SAFE_RTSP_REQUEST_H__