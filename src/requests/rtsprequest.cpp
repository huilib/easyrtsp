

#include "rtsprequest.h"
#include "../utils/rtspparser.h"

#include <hlog.h>
#include <hstr.h>
#include <time.h>

RtspRequest::RtspRequest(RtspRequestHeader&& header) noexcept
    : m_header(std::move(header)), m_strPayload(),
    m_connection(nullptr) {

}


RtspRequest::~RtspRequest() {

}


RtspRequest* RtspRequest::ParseRtspRequest(HCSZ data, HN data_len) {

    HN header_len = rtsp_parser::find_rtsp_header_pos(data, data_len);
    HSTR strHeader(data, data + header_len);
    LOG_NORMAL("parse request-text length[%d][%s] find_rtsp_header_pos return: [%d]", 
        data_len, data, header_len);
    
    RtspRequestHeader header;
    IF_NOTOK(header.Parse(strHeader)) {
        LOG_ERROR("parse rtsp header failed");
        return nullptr;
    }

    RtspRequest* req = req_map_ton::Instance()->MakeRequest(std::move(header));
    if (req == nullptr) {
        return nullptr;
    }
    HCSZ cszPayload = data + header_len + 2;
    HSTR strPayload(cszPayload, cszPayload + (data_len - header_len - 2));
    req->SetPayload(std::move(strPayload));

    return req;

}



RtspRequest* RequestMng::MakeRequest(RtspRequestHeader&& header) {

    req_map_t::iterator fit = m_req_map.find(header.GetMethod());
    if (fit == m_req_map.end()) {
        return nullptr;
    }

    req_maker req = fit->second;
    if (req) {
        return req(std::move(header));
    }

    return nullptr;

}


void RequestMng::RegisteMaker(RTSP_METHOD method, req_maker req) {

    m_req_map[method] = req;

}

