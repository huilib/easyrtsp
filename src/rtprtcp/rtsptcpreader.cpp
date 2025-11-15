
#include "rtsptcpreader.h"
#include <hlog.h>

RtspTcpReader::RtspTcpReader(HTcpSocket& sock, HIOBuffer& buffer, RtspSession* session) noexcept
    : m_state(RTR_STATE::RTR_OK), m_channelid(-1), m_length(-1),
    m_sock(sock), m_buffer(buffer), m_session(session) {

}


RtspTcpReader::~ RtspTcpReader() {

}


RtspTcpReader::RTR_STATE RtspTcpReader::Read() {

    // handle incoming request over the tcp channel.
    // a '$' character
    // a 1-byte channel id
    // a 2-bytes packet length (in network byte order).

    if (m_state == RTR_STATE::RTR_ERROR) {
        return RTR_STATE::RTR_ERROR;
    }

    if (m_state == RTR_STATE::RTR_WAIT) {
        return read_rtsp_tcp_body();
    }

    if (m_state == RTR_STATE::RTR_OK) {
        return read_rtsp_tcp_header();
    }

    return RTR_STATE::RTR_ERROR;

}

#define CHECK_CLOSED(res)  do { \
    if (res == 0) { \
        LOG_WARNING("client connection is closed with return[%d]", res);    \
        return return_state(RTR_STATE::RTR_ERROR);  \
    }   \
} while(0)


RtspTcpReader::RTR_STATE RtspTcpReader::read_rtsp_tcp_header() {

    HUCH uc;
    size_t res = m_sock.Read(&uc, 1);
    CHECK_CLOSED(res);
    if (uc != '$') {
        // invalid tcp-rtsp command.
        LOG_ERROR("rtcp-over-tcp command must begin by a '$'");
        return return_state(RTR_STATE::RTR_ERROR);
    }

    res = m_sock.Read(&uc, 1);
    CHECK_CLOSED(res);
    m_channelid = HTO_INT(uc);
    if (not m_session->IsInvalidChannelId(m_channelid)) {
        LOG_ERROR("rtcp-over-tcp command with an invalid channel id[%d]", m_channelid);
        return return_state(RTR_STATE::RTR_ERROR);
    }
        
    HUSN snlength = 0;
    res = m_sock.Read(&uc, 1);
    CHECK_CLOSED(res);
    snlength = static_cast<HUSN>(uc);

    res = m_sock.Read(&uc, 1);
    CHECK_CLOSED(res);
    snlength = (snlength << 8) | uc;

    m_length = HTO_INT(snlength);

    return return_state(RTR_STATE::RTR_WAIT);

}


RtspTcpReader::RTR_STATE RtspTcpReader::read_rtsp_tcp_body() {

    HUSZ buf = m_buffer.PosWrite();

    HN len = m_buffer.AvalableLength();

    if (m_length >= len) {

        LOG_ERROR("invalid rtsp-tcp command length[%d] avalable length is [%d]", m_length, len);
        return return_state(RTR_STATE::RTR_ERROR);

    }

    HOFF res = m_sock.ReadAllWithTimeout(buf, m_length, 3);
    CHECK_CLOSED(res);

    if (res == static_cast<HOFF>(m_length)) {

        m_buffer.SetPos(res); 
        return return_state(RTR_STATE::RTR_OK);

    }         

    return return_state(RTR_STATE::RTR_ERROR);
}


RtspTcpReader::RTR_STATE RtspTcpReader::return_state(RTR_STATE ss) {

    m_state = ss;

    return m_state;

}

