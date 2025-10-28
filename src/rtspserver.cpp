

#include "rtspserver.h"
#include "rtspconnect.h"
#include "globalconfig.h"
#include "mediasource/rtspstream.h"
#include <hlog.h>
#include <utils/json_utils.h>
#include <hfilestr.h>


RtspServer::RtspServer(HCSTRR strIp, HUN nPort) noexcept
    : m_strIp(strIp), m_port(nPort), m_sock(),
    m_connections(), m_task_check_clients(0), m_let(nullptr),
    m_streams() {

}


RtspServer::~ RtspServer() {

    uninit();

}


HRET RtspServer::Init() {

    HNOTOK_MSG_RETURN(init_streams(), "init stream failed");

    HNOTOK_MSG_RETURN(init_network(), "init network failed");

    m_task_check_clients = Schedule().ScheduleDelayedTask(0, std::bind(&RtspServer::check_clients, this));

    Schedule().SetBackgroundHandling(m_sock.Fd(), FIT_READ, std::bind(
        &RtspServer::acceptIncommingConnectionHandler, this, std::placeholders::_1));

    HRETURN_OK;

}


StreamBase* RtspServer::GetStream(HCSTRR strName) noexcept {

    stream_map_t::iterator it = m_streams.find(strName);
    if (it == m_streams.end()) {
        return nullptr;
    }

    return it->second;

}


const StreamBase* RtspServer::GetStream(HCSTRR strName) const noexcept {

    stream_map_t::const_iterator it = m_streams.find(strName);
    if (it == m_streams.cend()) {
        return nullptr;
    }

    return it->second;

}


void RtspServer::check_clients() {

    //LOG_NORMAL("rtsp server check clients. client count[%d]", m_connections.size());

    for (connections_t::iterator it = m_connections.begin(); it != m_connections.end();) {
        
        RtspConnect* conn = *it;
        if (not conn->IsActive()) {

            HDELP(conn);            
            it = m_connections.erase(it);

            LOG_NORMAL("remove a client connection");

            continue;
        }

        ++it;

    }

    Schedule().ScheduleDelayedTask(0, std::bind(&RtspServer::check_clients, this));

}


void RtspServer::acceptIncommingConnectionHandler(HUN) {

    LOG_NORMAL("RtspServer accept");

    HTcpSocket client_sock;
    HIp4Addr client_addr;
    IF_NOTOK(m_sock.Accept(client_sock, client_addr)) {
        LOG_ERROR("accept new client connect failed");
        return;
    }

    HIGNORE_RETURN(client_sock.SetNonblocked());
    HIGNORE_RETURN(client_sock.SetupSendBufLength(SOCKET_BUFFER_LENGTH));

    RtspConnect * newconnect = new RtspConnect(*this, std::move(client_sock), client_addr);
    CHECK_NEWPOINT(newconnect);

    m_connections.push_back(newconnect);

}


void RtspServer::uninit() {

    // unregiste task
    LOG_WARNING("RtspServer UnscheduleDelayedTask task[%d]", m_task_check_clients);
    Schedule().UnscheduleDelayedTask(m_task_check_clients);

    // clear connections.
    for (connections_t::iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
        RtspConnect* conn = *it;
        HDELP(conn);        
    }

    m_connections.clear();

    for (stream_map_t::const_iterator cfit = m_streams.cbegin(); cfit != m_streams.cend(); ++cfit) {        
        const StreamBase* sb = cfit->second;
        HDELP(sb);
    }
    m_streams.clear();

}


HRET RtspServer::init_streams() {

    HCSTRR strStreamConfigName = m_let->GetApp()->GetConfValue("MEDIA_FILE");
    HASSERT_MSG_RETURN(not strStreamConfigName.empty(), "stream cofnig filename is empty", INVL_PARA);

    HSTR strStreamConfigContent;
    HNOTOK_MSG_RETURN(HFileStr(strStreamConfigName).ReadToString(strStreamConfigContent),
        "read stream config content to string failed");
    
    HASSERT_MSG_RETURN(not strStreamConfigContent.empty(), "stream json config file content is empty", INVL_PARA);

    json_t jsconfig;

    try {

        jsconfig = json_t::parse(strStreamConfigContent);

    } catch(...) {

        LOG_ERROR("stream json config file [%s] content is invalid", strStreamConfigName.c_str());

        HRETURN(INVL_PARA);

    }

    HASSERT_MSG_RETURN(jsconfig.contains("streams"), "jsconfig miss streams item", SRC_ERR);

    json_t stream_js = jsconfig["streams"];

    HASSERT_MSG_RETURN(stream_js.is_array(), "jsconfig stream item is not array", SRC_ERR);

    for (json_t::size_type i = 0; i < stream_js.size(); ++i) {

        const json_t& sj = stream_js[i];

        if (not sj.contains("name")) {
            LOG_WARNING("miss type in stream entity.");
            continue;
        }

        if (not sj.contains("type")) {
            LOG_WARNING("miss type in stream entity.");
            continue;
        }

        if (not sj.contains("filename")) {
            LOG_WARNING("miss filename in stream entity.");
            continue;
        }

        if (not sj.contains("port")) {
            LOG_WARNING("miss port in stream entity.");
            continue;
        }

        HSTR strName = sj["name"].get<HSTR>();
        HStr strType = sj["type"].get<HSTR>();
        HSTR strFilePath = sj["filename"].get<HSTR>();
        HN port = sj["port"].get<HN>();

        if (strType.Upper().compare("MP4") == 0) {
            
            RtspStream* new_stream = new RtspStream(strFilePath);
            NULLPOINTER_CHECK(new_stream);   

            new_stream->SetPort(port); 

            IF_NOTOK(new_stream->Init()) {
                HDELP(new_stream);
            }

            m_streams[strName] = new_stream;
            LOG_NORMAL("stream[%s] ptr[%p] setup to server", strName.c_str(), new_stream);

        }
    }
    
    HRETURN_OK;
}


HRET RtspServer::init_network() {

    HNOTOK_RETURN(m_sock.Init());

    HNOTOK_RETURN(m_sock.SetReuseAddr());

    HNOTOK_RETURN(m_sock.SetReusePort());

    HIp4Addr addr(m_strIp, m_port);
    HNOTOK_RETURN(m_sock.Bind(addr));

    HNOTOK_RETURN(m_sock.SetNonblocked());

    HN keepAlive = 1;
    HN keepIdle = 200;
    HN keepInterval = 5;
    HN keepCount = 20;

    HNOTOK_RETURN(m_sock.SetKeepAlive(keepAlive, keepIdle, keepInterval, keepCount));

    HNOTOK_RETURN(m_sock.SetupSendBufLength(SOCKET_BUFFER_LENGTH));

    HNOTOK_RETURN(m_sock.Listen(20));

    HRETURN_OK;

}