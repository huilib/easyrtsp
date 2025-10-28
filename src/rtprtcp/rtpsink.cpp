

#include <hsocket.h>
#include "rtpsink.h"

#include <hlog.h>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/time.h>
}

unsigned char RtpSink::START_KEY4[] = {0x00, 0x00, 0x0, 0x01};
unsigned char RtpSink::AUDIO_START4[] = {0x00, 0x00, 0x00, 0x00};


RtpSink::RtpSink(TrackBase* tb, const HIp4Addr& addr, HUSN payload_type)
    :sink_bass_t(tb, addr),
    m_payload_type(payload_type),
    m_last_rtp(0), m_last_rtp_time(0) {


}


RtpSink::~RtpSink() noexcept {

}


void RtpSink::SendPack(const MediaPacket& packet) {    

    m_last_rtp = GetRtpTimestamp(packet);

    if (packet.GetRefBuffer().GetLength() < MAX_RTP_LENGTH) {
        send_single_nalu_uint(packet);        
    } else {
        send_fregmentation_nalu_uints(packet);
    }

    m_last_rtp_time = av_gettime();

}



HUN RtpSink::GetRtcpNtpRtpTimestamp() const {

    if (GetTrack() == nullptr) {
        return 0;
    }
    
    HULL nowav = av_gettime();
    HUN time_dis = static_cast<HUN>(nowav - m_last_rtp_time);

    HUN twdst = static_cast<HUN>(av_rescale_q(time_dis, AVRational{1, 1000000}, 
        AVRational{GetTrack()->GetTimebase().num, GetTrack()->GetFrequency()}));

    /*LOG_NORMAL("trackid[%d] sample_rate[%u] num[%d] den[%d] twdst[%u] timedis[%u]",
        GetTrack()->GetTrackId(), 
        GetTrack()->GetFrequency(),
        GetTrack()->GetTimebase().num, 
        GetTrack()->GetTimebase().den, twdst, time_dis);*/

    return m_last_rtp + twdst;

}


const HUdpSock& RtpSink::getSocket() const {

    return GetTrack()->GetRtpSocket();

}


void RtpSink::send_single_nalu_uint(const MediaPacket& packet) {

    HIOOutputBuffer buffer;

    HN naluLen = packet.GetRefBuffer().GetLength();
    HCUSZ naluBuf = static_cast<HCUSZ>(packet.GetRefBuffer().Data());

    common_rtsp_header(packet, buffer);

    if (packet.GetTrackType() == TRACK_TYPE::AUDIO) {
        // 4 bytes for audio
        AUDIO_START4[0] = 0x00;
        AUDIO_START4[1] = 0x10;
        AUDIO_START4[2] = (naluLen & 0x1FE0) >> 5;
        AUDIO_START4[3] = (naluLen & 0x1F) << 3;
        buffer.AddIo(AUDIO_START4, 4);
    }

    if (packet.GetTrackType() == TRACK_TYPE::VIDEO) {
        if (packet.GetNaluType() != NaluType::SPS and 
                packet.GetNaluType() != NaluType::PPS) {
            naluBuf += 4;
            naluLen -= 4;
        }
    }

    buffer.AddIo(naluBuf, naluLen);

    connect();

    send_buffer(buffer);

    IncreSequence();

}


void RtpSink::send_fregmentation_nalu_uints(const MediaPacket& packet) {

    HIOOutputBuffer buffer;

    uint  naluLen = packet.GetRefBuffer().GetLength() - 5;  // start code 0x00 00 00 01
    HCUSZ naluBuf = static_cast<HCUSZ>(packet.GetRefBuffer().Data());
    const uint8_t* pos = naluBuf + 1;
    uint unit_len = MAX_RTP_LENGTH;
    uint8_t nalu_hdr = naluBuf[0];
    uint8_t fu_indicator = 0, fu_header = 0;
    uint8_t fu_buffer[2] = {0};
    bool bFirst = true;
    bool bLast = false;

    connect();

    while (naluLen > 0) {

        if (naluLen > MAX_RTP_LENGTH) {
            common_rtsp_header(packet, buffer, false);
            unit_len = MAX_RTP_LENGTH;
            naluLen -= unit_len;
        } else {
            common_rtsp_header(packet, buffer, true);
            unit_len = naluLen;
            naluLen = 0;
            bLast = true;
        }

        do {
            if (bFirst) {
                fu_indicator = (nalu_hdr & 0xE0) | 28;
                fu_header = 0x80 | (nalu_hdr & 0x1F);
                fu_buffer[0] = fu_indicator;
                fu_buffer[1] = fu_header;
                bFirst = false;
                break;
            }

            if (bLast) {
                fu_header = (nalu_hdr & 0x1F) | 0x40;// FU Header. (set the E bit)
                fu_buffer[0] = fu_indicator;
                fu_buffer[1] = fu_header;   
                break;
            }

            fu_header = (nalu_hdr & 0x1F) | 0x00;// FU Header. (no S bit)
            fu_buffer[0] = fu_indicator;
            fu_buffer[1] = fu_header;

        } while(0);

        // fu packet.        
        buffer.AddIo(fu_buffer, 2);
        buffer.AddIo(pos, unit_len);
        pos += unit_len;

        send_buffer(buffer);

        IncreSequence();

        buffer.Reset();

    }    

}


void RtpSink::common_rtsp_header(const MediaPacket& packet,  
    HIOOutputBuffer& buffer, bool setMarket) {

    HOutputPacketBuffer& header = buffer.GetHeader(); 

    unsigned rtpHdr = 0x80000000; // RTP version 2; marker ('M') bit not set (by default; it can be set later)
    if (setMarket) {
        rtpHdr |= 0x00800000;  // set marker 'M' bit.
    }
    rtpHdr |= (m_payload_type<<16);
    rtpHdr |= GetSequence();
    header.EnqueueWord(rtpHdr);

    header.EnqueueWord(GetRtpTimestamp(packet));

    header.EnqueueWord(GetSSRC());

}


HSocket::size_type RtpSink::send_buffer(HIOOutputBuffer& buffer) {
   
    const HUdpSock& sock = getSocket();
    IncrePacketCount();
    HUdpSock::size_type ret = buffer.WriteIo(sock);
    IncreByteCount(ret);

    HUN data_len = buffer.GetDataLength();
    if (data_len != ret) {
        LOG_WARNING("all length[%u] send length[%u]", data_len, ret);
    }

    return ret;
}


H264RtpSink::H264RtpSink(TrackBase* tb, const HIp4Addr& addr)
    : base_class_t(tb, addr, static_cast<HUSN>(RTP_PAYTYPE::H264_PT)) {

}


H264RtpSink::~H264RtpSink() noexcept {

}


HUN H264RtpSink::GetRtpTimestamp(const MediaPacket& packet) const noexcept {

    return GetTimebase() + static_cast<HUN>(packet.GetRtpPts());
    //return GetTimebase() + static_cast<HUN>(packet.GetPts());

}


AacRtpSink::AacRtpSink(TrackBase* tb, const HIp4Addr& addr)
    : base_class_t(tb, addr, static_cast<HUSN>(RTP_PAYTYPE::AAC_PT)) {

}


AacRtpSink::~AacRtpSink() noexcept {
    
}


HUN AacRtpSink::GetRtpTimestamp(const MediaPacket& packet) const noexcept {

    return GetTimebase() + static_cast<HUN>(packet.GetPts());

}



unsigned char TcpRtpBase::TCP_AUDIO_START4[] = {0x00, 0x00, 0x00, 0x00};

TcpRtpBase::TcpRtpBase(TrackBase* tb, const HTcpSocket& sock, HUSN payload_type) noexcept 
    : base_class_t(tb, sock) ,
    m_payload_type(payload_type),
    m_last_rtp(0), m_last_rtp_time(0) {

}


TcpRtpBase::~TcpRtpBase() noexcept {

}


void TcpRtpBase::SendPack(const MediaPacket& packet) {
    
    m_last_rtp = GetRtpTimestamp(packet);

    send_over_tcp(packet);        

    m_last_rtp_time = av_gettime();

}


HUN TcpRtpBase::GetRtcpNtpRtpTimestamp() const {

    if (GetTrack() == nullptr) {
        return 0;
    }
    
    HULL nowav = av_gettime();
    HUN time_dis = static_cast<HUN>(nowav - m_last_rtp_time);

    HUN twdst = static_cast<HUN>(av_rescale_q(time_dis, AVRational{1, 1000000}, 
        AVRational{GetTrack()->GetTimebase().num, GetTrack()->GetFrequency()}));

    return m_last_rtp + twdst;

}


void TcpRtpBase::common_rtsp_header(const MediaPacket& packet,  
    HIOOutputBuffer& buffer, bool setMarket) {

    HOutputPacketBuffer& header = buffer.GetHeader(); 

    unsigned rtpHdr = 0x80000000; // RTP version 2; marker ('M') bit not set (by default; it can be set later)
    if (setMarket) {
        rtpHdr |= 0x00800000;  // set marker 'M' bit.
    }
    rtpHdr |= (m_payload_type<<16);
    rtpHdr |= GetSequence();
    header.EnqueueWord(rtpHdr);

    header.EnqueueWord(GetRtpTimestamp(packet));

    header.EnqueueWord(GetSSRC());

}


void TcpRtpBase::send_over_tcp(const MediaPacket& packet) {

    HIOOutputBuffer buffer;

    HN naluLen = packet.GetRefBuffer().GetLength();
    HCUSZ naluBuf = static_cast<HCUSZ>(packet.GetRefBuffer().Data());

    common_rtsp_header(packet, buffer);

    if (packet.GetTrackType() == TRACK_TYPE::AUDIO) {
        // 4 bytes for audio
        TCP_AUDIO_START4[0] = 0x00;
        TCP_AUDIO_START4[1] = 0x10;
        TCP_AUDIO_START4[2] = (naluLen & 0x1FE0) >> 5;
        TCP_AUDIO_START4[3] = (naluLen & 0x1F) << 3;
        buffer.AddIo(TCP_AUDIO_START4, 4);
    }

    if (packet.GetTrackType() == TRACK_TYPE::VIDEO) {
        if (packet.GetNaluType() != NaluType::SPS and 
                packet.GetNaluType() != NaluType::PPS) {
            naluBuf += 4;
            naluLen -= 4;
        }
    }

    buffer.AddIo(naluBuf, naluLen);

    send_buffer(buffer);

    IncreSequence();

}


HSocket::size_type TcpRtpBase::send_buffer(HIOOutputBuffer& buffer) {

    const HTcpSocket& sock = getSocket();
    IncrePacketCount();

    HUN data_len = buffer.GetDataLength();
    {
        HUCH framingheader [4] = {0};
        framingheader[0] = '$';
        framingheader[1] = GetChannelId();
        framingheader[2] = (u_int8_t) ((data_len&0xFF00)>>8);
        framingheader[3] = (u_int8_t) (data_len&0xFF);
        sock.Write(framingheader, 4);
    }
    
    HUdpSock::size_type ret = buffer.WriteIo(sock);
    IncreByteCount(ret);
    
    if (data_len != ret) {
        LOG_WARNING("all length[%u] send length[%u]", data_len, ret);
    }

    return ret;

}


H264TcpRtpSink::H264TcpRtpSink(TrackBase* tb, const HTcpSocket& sock)
    :base_class_t(tb, sock, static_cast<HUSN>(RTP_PAYTYPE::H264_PT)) {

}


H264TcpRtpSink::~H264TcpRtpSink() noexcept {

}


HUN H264TcpRtpSink::GetRtpTimestamp(const MediaPacket& packet) const noexcept {

    return GetTimebase() + static_cast<HUN>(packet.GetRtpPts());

}


AacTcpRtpSink::AacTcpRtpSink(TrackBase* tb, const HTcpSocket& sock)
    : base_class_t(tb, sock, static_cast<HUSN>(RTP_PAYTYPE::AAC_PT)) {

}


AacTcpRtpSink::~AacTcpRtpSink() noexcept {

}


HUN AacTcpRtpSink::GetRtpTimestamp(const MediaPacket& packet) const noexcept {
    return GetTimebase() + static_cast<HUN>(packet.GetPts());
}