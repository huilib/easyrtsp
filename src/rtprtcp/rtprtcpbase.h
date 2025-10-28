

#ifndef __H_RTPRTCP_BASE_H__
#define __H_RTPRTCP_BASE_H__

#include <huicpp.h>
#include "../mediasource/trackbase.h"
#include "rtpinfo.h"
#include "../mediasource/streaminfo.h"
#include "../utils/ranapi.h"


using namespace HUICPP;

class SinkBase {
public:
    SinkBase(TrackBase* tb) noexcept;

    virtual ~SinkBase() noexcept;

public:
    TRACK_TYPE GetTrackType() const noexcept { return m_tb->GetTrackType(); }

    const TrackBase* GetTrack() const noexcept { return m_tb; }

private:
    TrackBase* m_tb;
};

struct tagRtp  {};
struct tagRtcp {};

template<typename _Ty>
class mid_sink_base : public SinkBase {
private:
    using base_class_t = SinkBase;
public:
    mid_sink_base(TrackBase* tb) noexcept 
        : base_class_t(tb) {

    }
    virtual ~mid_sink_base() noexcept = default;

};

template<> 
class mid_sink_base<tagRtp> : public SinkBase{
private:
    using base_class_t = SinkBase;
public:
    mid_sink_base(TrackBase* tb) noexcept 
        : base_class_t(tb),
        m_timebase(0), m_ssrc(0), m_seq(0), 
        m_packet_count(0), m_byte_count(0) {
        m_timebase = our_random32();
        m_ssrc = our_random32();
        m_seq = static_cast<HUSN>(our_random());
    }
    virtual ~mid_sink_base() noexcept = default;

public:
    virtual void SendPack(const MediaPacket& packet) = 0;

public:
    HUN GetTimebase() const noexcept { return m_timebase; }
    HUN GetSSRC() const noexcept { return m_ssrc; }
    HUSN GetSequence() const noexcept { return m_seq; }

    void IncreSequence() noexcept { ++m_seq; }
    void SetSequence(HUSN seq) noexcept { m_seq = seq; }

    HUN GetPacketCount() const noexcept { return m_packet_count; }
    HUN GetByteCount() const noexcept { return m_byte_count; }
    void IncrePacketCount() noexcept { ++m_packet_count; }
    void IncreByteCount(HN nbytes) noexcept { m_byte_count += nbytes; }

    virtual HUN GetRtcpNtpRtpTimestamp() const = 0;

private:
    HUN m_timebase;
    HUN m_ssrc;
    HUSN m_seq;

    // statistics information
    HUN m_packet_count;
    HUN m_byte_count;
};

template<>
class mid_sink_base<tagRtcp> : public SinkBase{
private:
    using base_class_t = SinkBase;
public:
    mid_sink_base(TrackBase* tb) noexcept 
        : base_class_t(tb) {

    }
    virtual ~mid_sink_base() noexcept = default;

public:
    virtual void SendReport() = 0;
};

struct tagTcpMethod {};
struct tagUdpMethod {};

template<typename _tagname>
class UdpSinkBase: public mid_sink_base<_tagname> {
private:
    using base_class_t = mid_sink_base<_tagname>;

public:
    UdpSinkBase(TrackBase* tb, const HIp4Addr& addr)
        : base_class_t(tb), m_toaddr(addr) {

    }

    virtual ~UdpSinkBase() noexcept = default;
    
protected:
    void connect() const {
        HIGNORE_RETURN(getSocket().Connect(getAddr()));
    }
    const HUdpSock& connectedAndGetSocket() const {
        connect();
        return getSocket();
    }

    virtual const HUdpSock& getSocket() const = 0;
    
    const HIp4Addr& getAddr() const noexcept { return m_toaddr; }

private:
    HIp4Addr m_toaddr;
};

template<typename _tagname>
class TcpSinkBase : public mid_sink_base<_tagname>{
private:
    using base_class_t = mid_sink_base<_tagname>;

public:
    TcpSinkBase(TrackBase* tb, const HTcpSocket& sock) noexcept
        : base_class_t(tb), m_sock(sock), m_channel_id(0) { }

    virtual ~TcpSinkBase() noexcept { }

public:
    HUN GetChannelId() const noexcept { return m_channel_id; }
    void SetChannelId(HUN channel_id) noexcept { m_channel_id = channel_id; }
    
protected:
    const HTcpSocket& getSocket() const noexcept {return m_sock; }
    

private:
    const HTcpSocket& m_sock;
    HUN m_channel_id;
};


#endif // __H_RTPRTCP_BASE_H__