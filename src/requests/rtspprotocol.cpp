

#include "rtspprotocol.h"
#include <hstr.h>


RtspUrl::RtspUrl() noexcept
    : m_strUrl(), 
    m_strRequestStreamName(), m_strSubfixStreamName(),
    m_strTrack() {

}


RtspUrl::RtspUrl(RtspUrl&& url) noexcept 
    : m_strUrl(std::move(url.m_strUrl)), 
    m_strRequestStreamName(std::move(url.m_strRequestStreamName)),
    m_strSubfixStreamName(std::move(url.m_strSubfixStreamName)),
    m_strTrack(std::move(url.m_strTrack)) {

}


HRET RtspUrl::Parse(HCSTRR src) {

    HStr str(src);
    str.Trim();

    m_strUrl = str;

    HVSTR strs;
    HNOTOK_RETURN(str.Split("/", strs, 8));

    if (strs.size() > 1) {
        m_strRequestStreamName = strs[1];
    }

    if (strs.size() > 2) {
        m_strSubfixStreamName = strs[2];
    }

    if (strs.size() > 3) {
        m_strTrack = strs[3];
    }

    if (m_strTrack.empty() and strs.size() == 3) {
        m_strTrack = m_strSubfixStreamName;
    }

    HRETURN_OK;

}


HSTR RtspProtocol::s_methods[METHOD_LENGTH] = {
    HSTR("OPTIONS"), HSTR("DESCRIBE"), HSTR("SETUP"), HSTR("PLAY"),
    HSTR("TEARDOWN"), HSTR("PAUSE"), HSTR("GET_PARAMETER"), HSTR("SET_PARAMETER")
};


RTSP_METHOD RtspProtocol::RtspMethod(HCSTRR strMethod) noexcept {

    for (HN i = 0; i < METHOD_LENGTH; ++i) {
        HCSTRR str = s_methods[i];
        if (str == strMethod) {
            return static_cast<RTSP_METHOD>(i);
        }
    }

    return RTSP_METHOD::RM_END;
}


HCSTRR RtspProtocol::RtspMethodString(RTSP_METHOD method) noexcept {

    static HCSTR strUnknownMethod("UNKNOWN");
    HN nMethod = HTO_INT(method);
    if (nMethod >= METHOD_LENGTH) {
        return strUnknownMethod;
    }

    return s_methods[nMethod];

}


HSTR RtspProtocol::RtspRequestCodeString(RTSP_REQUEST_CODE code) noexcept {

    switch (code) {
        case RTSP_REQUEST_CODE::OK:
            return HSTR("OK");
            break;
        case RTSP_REQUEST_CODE::MOVED:
            return HSTR("Moved");
            break;
        case RTSP_REQUEST_CODE::BAD:
            return HSTR("Bad Request");
            break;
        case RTSP_REQUEST_CODE::NOT_FOUND:
            return HSTR("Stream Not Found");
            break;
        case RTSP_REQUEST_CODE::NOT_SUPPORT:
            return HSTR("Method Not Allowed");
            break;
        case RTSP_REQUEST_CODE::UNACCEPTABLE:
            return HSTR("Not Acceptable");
            break;
        case RTSP_REQUEST_CODE::PRECONDITION_FAILED:
            return HSTR("Precondition Failed");
            break;
        case RTSP_REQUEST_CODE::INVALID_PARAMETER:
            return HSTR("Invalid Parameter");
            break;
        case RTSP_REQUEST_CODE::SESSION_NOT_FOUND:
            return HSTR("Session Not Found");
            break;
        case RTSP_REQUEST_CODE::UNSUPPORT_TRANSPORT:
            return HSTR("Unsupported Transport");
            break;
    default:
        return HSTR("Unknown");
        break;
    }

}


RtspProtocolVersion::RtspProtocolVersion() noexcept
    : m_strRtsp(), 
    m_strVersionCode() {

}


RtspProtocolVersion::RtspProtocolVersion(RtspProtocolVersion&& rtsp) noexcept 
    : m_strRtsp(std::move(rtsp.m_strRtsp)), 
    m_strVersionCode(std::move(rtsp.m_strVersionCode)) {

}


HRET RtspProtocolVersion::Parse(HCSTRR src) {

    HStr str(src);
    str.Trim();

    HVSTR strs;
    HNOTOK_RETURN(str.Split("/", strs));

    HASSERT_RETURN(strs.size() == 2, INVL_PARA);

    m_strRtsp = strs[0];
    m_strVersionCode = strs[1];

    HRETURN_OK;

}


RtspRequestHeader::RtspRequestHeader() noexcept 
    : m_method(RTSP_METHOD::RM_END),
    m_url(), m_version(), 
    m_attributes() {

}


RtspRequestHeader::RtspRequestHeader(RtspRequestHeader&& header) noexcept 
    : m_method(header.m_method),
    m_url(std::move(header.m_url)), m_version(std::move(header.m_version)), 
    m_attributes(std::move(header.m_attributes)) {


}



HRET RtspRequestHeader::Parse(HCSTRR src) {

    HStr str(src);

    HVSTR lines;
    HNOTOK_RETURN(str.Split("\r\n", lines));

    HASSERT_RETURN(lines.size() > 2, INVL_PARA);

    HCSTRR strTopLine = lines[0];
    HNOTOK_RETURN(parse_topline(strTopLine));

    for (HVSTR::size_type i = 1; i < lines.size(); ++i) {
        HStr line(lines[i]);

        if (line.Trim().empty()) {
            continue;
        }

        HVSTR ss;

        HNOTOK_RETURN(line.Split(":", ss));

        HASSERT_RETURN(ss.size() == 2, INVL_PARA);

        m_attributes.push_back(std::pair<HSTR, HSTR>(HStr(ss[0]).Trim().Upper(), 
            HStr(ss[1]).Trim()));
    }

    HRETURN_OK;

}


HCSTRR RtspRequestHeader::GetCSeq() const noexcept {

    return GetAttribute("CSEQ");

}


HCSTRR RtspRequestHeader::GetUserAgent() const noexcept {

    return GetAttribute("USER-AGENT");

}


HCSTRR RtspRequestHeader::GetSessionId() const noexcept {

    return GetAttribute("SESSION");

}


HN RtspRequestHeader::GetContentLength() const noexcept {

    return HStr(GetAttribute("CONTENT-LENGTH")).ToN();

}


HCSTRR RtspRequestHeader::GetAttribute(HCSTRR strSrcKey) const noexcept {

    HStr strKey(strSrcKey);
    strKey.Upper();

    static HSTR empty_str;

    for (attributes_t::size_type i = 0; i < m_attributes.size(); ++i) {
        const std::pair<HSTR, HSTR>& kv = m_attributes[i];

        if (kv.first == strKey) {
            return kv.second;
        }
    }

    return empty_str;

}


HRET RtspRequestHeader::parse_topline(HCSTRR src) {

    HStr str(src);
    str.Trim();

    HVSTR strs;
    HNOTOK_RETURN(str.Split(" ", strs));
    
    HASSERT_RETURN(strs.size() == 3, INVL_PARA);

    HCSTRR strMethod = strs[0];

    m_method = RtspProtocol::RtspMethod(strMethod);
    HASSERT_RETURN(m_method != RTSP_METHOD::RM_END, INVL_PARA);

    HCSTRR strUrl = strs[1];
    HNOTOK_RETURN(m_url.Parse(strUrl));

    HCSTRR strVersion = strs[2];
    HNOTOK_RETURN(m_version.Parse(strVersion));

    HRETURN_OK;

}