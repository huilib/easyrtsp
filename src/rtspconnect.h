

#ifndef __H_RTSP_CONNECT_H__
#define __H_RTSP_CONNECT_H__

#include <hiobuffer.h>

#include "schedulelet.h"
#include "rtspserver.h"


class RtspSession;
class RtspConnect : public ScheduleOBjectbase {
private:
    using base_class_t = ScheduleOBjectbase;

    static constexpr const HN ACTIVE_INTERVAL = 5;

public:
    RtspConnect(RtspServer& server, HTcpSocket&& sock, HIp4Addr& addr) noexcept;

    ~ RtspConnect();

public:
    bool IsActive() const noexcept;

    const RtspServer& Server() const noexcept {return m_server;}
    RtspServer& Server() noexcept {return m_server;}

    bool CheckSession(HCSTRR strSessionId);

    RtspSession* GetSession() noexcept { return m_session; }

    void SetDeathActive() noexcept { m_bActive = false; }

    const HIp4Addr& GetClientIp() const noexcept { return m_client_addr; }

    const HTcpSocket& GetRtspSocket() const noexcept { return m_socket; }

private:
    void incomingRequestHander(HUN );

    size_t readClientRequest();

    void handleRequest();

    void removeSchedule();

private:
    RtspServer& m_server;
    HTcpSocket m_socket;
    HIp4Addr m_client_addr;
    HIOBuffer m_request_buffer;
    HIOBuffer m_response_buffer;
    bool m_bActive;
    HTime m_last_active_time;
    RtspSession* m_session;
};


#endif // __H_RTSP_CONNECT_H__