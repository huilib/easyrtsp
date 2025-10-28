
#include "rtspstream.h"
#include "mediaslotmng.h"
#include <sstream>
#include <hstr.h>
#include <hlog.h>
#include <hfilename.h>
#include <haddr.h>


RtspStream::RtspStream(HCSTRR strName) 
    : stream_base_t(HFileName(strName).JustFileName()),
    m_strFileName(strName),
    m_suport_methods() {

    m_suport_methods.push_back(RTSP_METHOD::RM_OPTIONS);
    m_suport_methods.push_back(RTSP_METHOD::RM_DESCRIBE);
    m_suport_methods.push_back(RTSP_METHOD::RM_SETUP);
    m_suport_methods.push_back(RTSP_METHOD::RM_PLAY);
    m_suport_methods.push_back(RTSP_METHOD::RM_TEARDOWN);
    m_suport_methods.push_back(RTSP_METHOD::RM_PAUSE);
    m_suport_methods.push_back(RTSP_METHOD::RM_GET_PARAMETER);
    m_suport_methods.push_back(RTSP_METHOD::RM_SET_PARAMETER);

}


RtspStream::~RtspStream() noexcept {


}


HRET RtspStream::Init() {

    return media_slot_mng::Instance()->CreateMp4Slot(m_strFileName, this);

}


HSTR RtspStream::GenerateSDP() const {

    std::stringstream ss;
    struct timeval creationTime;
    gettimeofday(&creationTime, nullptr);

    HSTR strOurIP = HIp4Addr::GetLocalIp();

    ss << "v=0\r\n"         
        << "o=- " << creationTime.tv_sec << creationTime.tv_usec << " 1 IN IP4 " << strOurIP << "\r\n"
        << "s=Session streamed by Huicpp Joseph\r\n"
        << "i=" << GetName() << "\r\n"
        << "t=0 0\r\n"
        << "a=tool:HUICPP RtspServer v0\r\n"
        << "a=type:broadcast\r\n"
        << "a=control:*\r\n"
        << "a=range:npt=now-\r\n"
        << "a=x-qt-text-nam:Session streamed by RtspServer \r\n"
        << "a=x-qt-text-inf:" << GetName() << "\r\n";

    const std::vector<TrackBase*>& tracks = GetTracks();
    for (std::vector<TrackBase*>::size_type i = 0; i < tracks.size(); ++i) {
        const TrackBase* ctb = tracks[i];
        ss << ctb->GenerateSDP();
    }

    HSTR res = ss.str();
    return res;

}


HN RtspStream::GetTrackRtpPort(TRACK_TYPE tt) const noexcept {

    return m_port + HTO_INT(tt) * 2;

}


HSTR RtspStream::GetSupportMethods() const noexcept {
    
    std::stringstream ss;
    for (std::vector<RTSP_METHOD>::size_type i = 0; i < m_suport_methods.size(); ++i) {
        RTSP_METHOD rm = m_suport_methods[i];
        ss << RtspProtocol::RtspMethodString(rm) <<",";
    }

    return HStr(ss.str()).Rtrim(",");

}



