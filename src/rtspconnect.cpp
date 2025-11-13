

#include "rtspconnect.h"
#include "rtspsession.h"
#include "requests/rtsprequest.h"

#include <htodoholder.h>
#include <hlog.h>

#include <iostream>
#include <signal.h>


RtspConnect::RtspConnect(RtspServer& server, HTcpSocket&& sock, HIp4Addr& addr) noexcept
    : base_class_t(),
    m_server(server), m_socket(std::move(sock)), m_client_addr(addr),
    m_request_buffer(), m_response_buffer(),
    m_bActive(true), m_last_active_time(),
    m_session(nullptr) {
        
    signal(SIGPIPE, SIG_IGN);

    Schedule().ScheduleDelayedTask(0, [this]() { 
        this->Schedule().SetBackgroundHandling(m_socket.Fd(), FIT_READ | FIT_EXCEP, 
        std::bind(&RtspConnect::incomingRequestHander, this, std::placeholders::_1));
    });
    //Schedule().SetBackgroundHandling(m_socket.Fd(), FIT_READ | FIT_EXCEP, 
    //    std::bind(&RtspConnect::incomingRequestHander, this, std::placeholders::_1));

}


RtspConnect::~ RtspConnect() {

    removeSchedule();

    HDELP(m_session);

}


bool RtspConnect::IsActive() const noexcept {

    /*HTime nowt;
    HTimeSpan ts = nowt - m_last_active_time;
    if (ts.GetSeconds() > ACTIVE_INTERVAL) {
        return 
    }*/

    return m_bActive;

}


bool RtspConnect::CheckSession(HCSTRR strSessionId) {

    if (not strSessionId.empty()) {

        if (m_session == nullptr) {
            return false;
        }

        if (strSessionId != m_session->GetSessionId()) {
            return false;
        }

        return true;

    }


    if (m_session != nullptr) {
        return false;
    }

    m_session = new RtspSession(this);
    return true;

}


void RtspConnect::incomingRequestHander(HUN ) {

    if (readClientRequest() > 0) {

        handleRequest();

    } 
    // remove select fd when delete connection.
    /*else {

        Schedule().ScheduleDelayedTask(0, std::bind(&RtspConnect::removeSchedule, this));

    }*/

}


size_t RtspConnect::readClientRequest() {

    HUSZ buf = m_request_buffer.PosWrite();
    HN len = m_request_buffer.AvalableLength();
    size_t res = m_socket.Read(buf, len);
    LOG_NORMAL("read [%d] bytes from client", res);
    if (res > 0) {
        m_last_active_time.Touch();
    } else {
        m_bActive = false;
        return res;
    }
    m_request_buffer.SetPos(res);    
    return res;

}


void RtspConnect::handleRequest() {

    HCSZ data = reinterpret_cast<HCSZ>(m_request_buffer.PopData());
    HN data_len = m_request_buffer.DataLength();

    HSCOPE_EXIT { m_request_buffer.Reset(); };

    RtspRequest* req = RtspRequest::ParseRtspRequest(data, data_len);
    if (req == nullptr) {
        LOG_WARNING("not found request handler.");

        if (m_session == nullptr) {
            // the session was not created yet.
            // or the RTSP communicate is not success yet.
            // It easy to happen on telnet test.
            m_bActive = false;
            return ;
        }

        // TODO:: close.....
        if (m_session->GetRtpRtcpHolder().GetTransportMode() == RtpTransportMode::RTP_TCP) {
            return;
        }
        
        m_bActive = false;
        return ;
    }
    // auto release RtspRequest* req.
    std::shared_ptr<RtspRequest> ptr(req);

    req->SetConnection(this);
    m_response_buffer.Reset();
    req->Handle(m_response_buffer);
    LOG_NORMAL("[%s] request response[%s] length[%d]", 
        RtspProtocol::RtspMethodString(req->GetHeader().GetMethod()).c_str(), 
        m_response_buffer.PopData(), m_response_buffer.DataLength());
    m_socket.Write(m_response_buffer.PopData(), m_response_buffer.DataLength());
    //LOG_NORMAL("fd[%d] request[%s]", m_socket.Fd(), data);

}

void RtspConnect::removeSchedule() {

    Schedule().RemoveBackgroundHandling(m_socket.Fd());

}

