

#include "videotrack.h"
#include "../utils/h264packets.h"
#include "../rtprtcp/rtpinfo.h"
#include <hlog.h>
#include <hstr.h>
#include <crypto/base64.h>

VideoTrack::VideoTrack() noexcept
    : base_class_t(),
    m_width(0), m_height(0),
    m_fps(0), m_field_order(0) {

}


VideoTrack::~ VideoTrack() noexcept {

}


H264or265Track::H264or265Track() noexcept
    : base_class_t(),
    m_sps(), m_pps(), m_sps_web(nullptr), m_sps_web_len(0) {

}


H264or265Track::~H264or265Track() noexcept {

    uninit();

}


HSTR H264or265Track::GetProfileLevel() const noexcept {

    HUN profileLevelId = (m_sps_web[1]<<16) | (m_sps_web[2]<<8) | m_sps_web[3];
    return HStr::Format("%06X", profileLevelId);

}


HRET H264or265Track::SetupSpsAndPps(HCMEMR memSps, HCMEMR memPps) noexcept {

    m_sps = memSps;
    m_pps = memPps;
    setupWeb();

    HRETURN_OK;

}


void H264or265Track::setupWeb() {

    uninit();

    m_sps_web = static_cast<HUSZ>(malloc(BUFFER_LENGTH));
    m_sps_web_len = BUFFER_LENGTH;
    
    m_sps_web_len = utils::removeH264or5EmulationBytes(m_sps_web, m_sps_web_len,
        m_sps.data(), m_sps.size());

}


void H264or265Track::uninit() {

    HFREE(m_sps_web);

    m_sps_web_len = 0;

}


H264VideoTrack::H264VideoTrack() noexcept {

}


H264VideoTrack::~ H264VideoTrack() noexcept{

}


HSTR H264VideoTrack::GenerateSDP() const {

    HSTR strBase64Sps, strBase64Pps;
    HIGNORE_RETURN(CRYPTO::Base64::Encode(GetSps(), strBase64Sps));
    HIGNORE_RETURN(CRYPTO::Base64::Encode(GetPps(), strBase64Pps));

    std::stringstream ss;
    // for now, we only support UDP-RTP.
    ss << "m=video 0 RTP/AVP " << RtpInfo::GetRtpPayploadType(GetTrackId()) << "\r\n"
        << "c=IN IP4 0.0.0.0\r\n"
        << "b=AS:500\r\n"
        << "a=rtpmap:" << RtpInfo::GetRtpPayploadType(GetTrackId()) << " H264/90000\r\n"
        << "a=fmtp:" << RtpInfo::GetRtpPayploadType(GetTrackId()) 
        << " packetization-mode=1;profile-level-id=" << GetProfileLevel() 
        << ";sprop-parameter-sets=" << strBase64Sps << "," << strBase64Pps << "\r\n"
        << "a=control:track" << GetTrackId() + 1 << "\r\n";

    return ss.str();

}