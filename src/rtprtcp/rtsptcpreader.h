
#ifndef __H_RTSPTCPREADER_H__
#define __H_RTSPTCPREADER_H__

#include <huicpp.h>
#include <hsocket.h>
#include <hiobuffer.h>
#include "../rtspsession.h"

using namespace HUICPP;

class RtspTcpReader {
public:
    enum class RTR_STATE {
        RTR_WAIT,
        RTR_OK,
        RTR_ERROR
    };

public:
    RtspTcpReader(HTcpSocket& sock, HIOBuffer& buffer, RtspSession* session) noexcept;

    ~ RtspTcpReader() noexcept;

public:
    RTR_STATE Read();

    HN GetChannelId() const noexcept { return m_channelid; }

    HN GetLength() const noexcept { return m_length; }

private:
    RTR_STATE read_rtsp_tcp_header();

    RTR_STATE read_rtsp_tcp_body();

    RTR_STATE return_state(RTR_STATE);

private:
    RTR_STATE m_state;
    HN m_channelid;
    HN m_length;
    HTcpSocket& m_sock;
    HIOBuffer& m_buffer;
    RtspSession* m_session;
};


#endif // __H_RTSPTCPREADER_H__

