

#ifndef __H_RTSP_PROTOCOL_H_
#define __H_RTSP_PROTOCOL_H_

#include <huicpp.h>

using namespace HUICPP;

class RtspUrl {
public:
    RtspUrl() noexcept;
    RtspUrl(RtspUrl&&) noexcept;
    RtspUrl(const RtspUrl&) = delete;

public:
    HRET Parse(HCSTRR str);

public:
    HCSTRR GetUrl() const noexcept { return m_strUrl; }
    HCSTRR GetRequestStreamName() const noexcept { return m_strRequestStreamName; }
    HCSTRR GetSubfixStreamName() const noexcept { return m_strSubfixStreamName; }
    HCSTRR GetTrack() const noexcept { return m_strTrack; }

private:
    HSTR m_strUrl;
    HSTR m_strRequestStreamName;
    HSTR m_strSubfixStreamName;
    HSTR m_strTrack;
};

enum class RTSP_METHOD : HN {
    RM_OPTIONS,
    RM_DESCRIBE,
    RM_SETUP,
    RM_PLAY,
    RM_TEARDOWN,
    RM_PAUSE,
    RM_GET_PARAMETER,
    RM_SET_PARAMETER,
    RM_END
};


enum class RTSP_REQUEST_CODE {
    NORMAL                  = 0,
    OK                      = 200,
    MOVED                   = 301,
    BAD                     = 400,
    NOT_FOUND               = 404,
    NOT_SUPPORT             = 405,
    UNACCEPTABLE            = 406,
    PRECONDITION_FAILED     = 412,
    INVALID_PARAMETER       = 451,
    SESSION_NOT_FOUND       = 454,
    UNSUPPORT_TRANSPORT     = 461,
    UNKOWN                  = 666
};


class RtspProtocol {
public:
    static constexpr const HN METHOD_LENGTH = HTO_INT(RTSP_METHOD::RM_END);

    static HSTR s_methods[METHOD_LENGTH];

    static RTSP_METHOD RtspMethod(HCSTRR strMethod) noexcept;

    static HCSTRR RtspMethodString(RTSP_METHOD method) noexcept;

    static HSTR RtspRequestCodeString(RTSP_REQUEST_CODE code) noexcept;
};


class RtspProtocolVersion {
public:
    RtspProtocolVersion() noexcept;
    RtspProtocolVersion(RtspProtocolVersion&&) noexcept;
    RtspProtocolVersion(const RtspProtocolVersion&) = delete;

public:
    HRET Parse(HCSTRR str);    

public:
    HCSTRR GetRtsp() const noexcept { return m_strRtsp; }
    HCSTRR GetVersionCode() const noexcept { return m_strVersionCode; }

private:
    HSTR m_strRtsp;
    HSTR m_strVersionCode;
};

class RtspRequestHeader {
private:
    using attributes_t = std::vector<std::pair<HSTR, HSTR> >;

public:
    RtspRequestHeader() noexcept;
    RtspRequestHeader(RtspRequestHeader&&) noexcept;
    RtspRequestHeader(const RtspRequestHeader&) = delete;

public:
    HRET Parse(HCSTRR src);

public:
    RTSP_METHOD GetMethod() const noexcept { return m_method; }
    const RtspUrl& GetUrl() const noexcept { return m_url; }
    const RtspProtocolVersion& GetRtspProtocolVersion() const noexcept { return m_version; }
    HCSTRR GetCSeq() const noexcept;
    HCSTRR GetUserAgent() const noexcept;
    HCSTRR GetSessionId() const noexcept;
    HN GetContentLength() const noexcept;
    HCSTRR GetAttribute(HCSTRR strKey) const noexcept;

private:
    HRET parse_topline(HCSTRR src);

private:
    RTSP_METHOD m_method;
    RtspUrl m_url;
    RtspProtocolVersion m_version;
    attributes_t m_attributes;
};


#endif // __H_RTSP_PROTOCOL_H_
