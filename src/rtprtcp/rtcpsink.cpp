

#include "rtcpsink.h"
#include <hlog.h>

RtcpSink::RtcpSink(TrackBase* tb, const HIp4Addr& addr, rtp_class_t* rtp_sink)
    : base_class_t(tb, addr),
    m_rtp_sink(rtp_sink) {

}



RtcpSink::~RtcpSink() noexcept {

}


void RtcpSink::SendReport() {

    HIOOutputBuffer buffer;
    addSenderReport(buffer);

    //send_buffer(buffer);

}


const HUdpSock& RtcpSink::getSocket() const {

    return GetTrack()->GetRtcpSocket();

}


void RtcpSink::addSenderReport(HIOOutputBuffer& buffer) {

    HOutputPacketBuffer& header = buffer.GetHeader(); 

    equeue_common_prefix(RTCP_PADLOAD_TYPE::SR, 5, header);

    // Now, add the 'sender info' for our sink

    // Insert the NTP and RTP timestamps for the 'wallclock time':
    struct timeval timeNow;
    gettimeofday(&timeNow, nullptr);
    header.EnqueueWord(timeNow.tv_sec + 0x83AA7E80);
    // NTP timestamp most-significant word (1970 epoch -> 1900 epoch)
    double fractionalPart = (timeNow.tv_usec/15625.0)*0x04000000; // 2^32/10^6
    header.EnqueueWord((unsigned)(fractionalPart+0.5));

    // NTP timestamp least-significant word
    //unsigned rtpTimestamp = fSink->convertToRTPTimestamp(timeNow);
    HUN rtpTimestamp = m_rtp_sink->GetRtcpNtpRtpTimestamp();
    header.EnqueueWord(rtpTimestamp); // RTP ts

    // Insert the packet and byte counts:
    header.EnqueueWord(m_rtp_sink->GetPacketCount());
    header.EnqueueWord(m_rtp_sink->GetByteCount());

}



void RtcpSink::equeue_common_prefix(RTCP_PADLOAD_TYPE pt,  unsigned numExtraWords,
    HOutputPacketBuffer& buffer) {

    unsigned numReportingSources = 0;
    unsigned rtcpHdr = 0x80000000; // version 2, no padding
    rtcpHdr |= (numReportingSources<<24);
    rtcpHdr |= (static_cast<HUCH>(pt) << 16);
    rtcpHdr |= (1 + numExtraWords + 6 * numReportingSources);

    buffer.EnqueueWord(rtcpHdr);
    buffer.EnqueueWord(m_rtp_sink->GetSSRC());

}


HSocket::size_type RtcpSink::send_buffer(HIOOutputBuffer& buffer) {

    const HUdpSock& sock = connectedAndGetSocket();

    HUdpSock::size_type ret = buffer.WriteIo(sock);

    //LOG_NORMAL("send rtcp length[%d]", ret);

    return ret;

}










TcpRtcpSink::TcpRtcpSink(TrackBase* tb, const HTcpSocket& sock, TcpRtpBase* rtp_sink)
    : base_class_t(tb, sock),
    m_rtp_sink(rtp_sink) {

}



TcpRtcpSink::~TcpRtcpSink() noexcept {

}


void TcpRtcpSink::SendReport() {

    HIOOutputBuffer buffer;
    addSenderReport(buffer);

    //send_buffer(buffer);

}


void TcpRtcpSink::addSenderReport(HIOOutputBuffer& buffer) {

    HOutputPacketBuffer& header = buffer.GetHeader(); 

    equeue_common_prefix(RTCP_PADLOAD_TYPE::SR, 5, header);

    // Now, add the 'sender info' for our sink

    // Insert the NTP and RTP timestamps for the 'wallclock time':
    struct timeval timeNow;
    gettimeofday(&timeNow, nullptr);
    header.EnqueueWord(timeNow.tv_sec + 0x83AA7E80);
    // NTP timestamp most-significant word (1970 epoch -> 1900 epoch)
    double fractionalPart = (timeNow.tv_usec/15625.0)*0x04000000; // 2^32/10^6
    header.EnqueueWord((unsigned)(fractionalPart+0.5));

    // NTP timestamp least-significant word
    //unsigned rtpTimestamp = fSink->convertToRTPTimestamp(timeNow);
    HUN rtpTimestamp = m_rtp_sink->GetRtcpNtpRtpTimestamp();
    header.EnqueueWord(rtpTimestamp); // RTP ts

    // Insert the packet and byte counts:
    header.EnqueueWord(m_rtp_sink->GetPacketCount());
    header.EnqueueWord(m_rtp_sink->GetByteCount());

}



void TcpRtcpSink::equeue_common_prefix(RTCP_PADLOAD_TYPE pt,  unsigned numExtraWords,
    HOutputPacketBuffer& buffer) {

    unsigned numReportingSources = 0;
    unsigned rtcpHdr = 0x80000000; // version 2, no padding
    rtcpHdr |= (numReportingSources<<24);
    rtcpHdr |= (static_cast<HUCH>(pt) << 16);
    rtcpHdr |= (1 + numExtraWords + 6 * numReportingSources);

    buffer.EnqueueWord(rtcpHdr);
    buffer.EnqueueWord(m_rtp_sink->GetSSRC());

}


HSocket::size_type TcpRtcpSink::send_buffer(HIOOutputBuffer& buffer) {

    const HSocket& sock = getSocket();

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

    return ret;

}
