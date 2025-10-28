
#ifndef __H_RTSP_SERVER_H__
#define __H_RTSP_SERVER_H__

#include <list>

#include <hsocket.h>

#include "scheduleobjectbase.h"

using namespace HUICPP;

class RtspConnect;
class ScheduleLet;
class StreamBase;
class RtspServer : public ScheduleOBjectbase{
private:
    using base_class_t = ScheduleOBjectbase;
    using connections_t = std::list<RtspConnect*>;
    using stream_map_t = std::map<HSTR, StreamBase*>;

public:
    RtspServer(HCSTRR strIp, HUN nPort) noexcept;

    ~ RtspServer();

public:
    HRET Init();

    StreamBase* GetStream(HCSTRR strName) noexcept;
    const StreamBase* GetStream(HCSTRR strName) const noexcept;

    void SetLet(ScheduleLet* let) noexcept { m_let = let; }
    ScheduleLet* GetLet() noexcept { return m_let; }
    const ScheduleLet* GetLet() const noexcept { return m_let; }

private:
    void check_clients();

    void acceptIncommingConnectionHandler(HUN);

private:
    void uninit();

    HRET init_streams();

    HRET init_network();


private:
    HSTR m_strIp;
    HUN m_port;
    HTcpSocket m_sock;
    connections_t m_connections;
    intptr_t m_task_check_clients;
    ScheduleLet* m_let;
    stream_map_t m_streams;
};


#endif // __H_RTSP_SERVER_H__