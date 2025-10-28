
#include "streambase.h"
#include <hlog.h>
#include <algorithm>


StreamBase::StreamBase(HCSTRR strName) noexcept
    : schedule_base_t(), obverser_base_t(),
    m_id(-1), m_strName(strName), m_isReady(false),
    m_start_time(0), m_duration(0),
    m_bit_rate(0), m_start_realtime(0),
    m_state(StreamState::CREATING), 
    m_tracks(),
    m_obs(),
    m_task_id(0) {

}


StreamBase::~StreamBase() noexcept {

    for (std::vector<TrackBase*>::size_type i = 0; i < m_tracks.size(); ++i) {
        TrackBase* tb = m_tracks[i];
        
        //LOG_WARNING("StreamBase handing fd[%d]", tb->GetRtcpSocket().Fd());
        schedule_base_t::Schedule().RemoveBackgroundHandling(tb->GetRtcpSocket().Fd());

    }

    if (m_task_id != 0) {
        LOG_WARNING("StreamBase UnscheduleDelayedTask task[%d]", m_task_id);
        schedule_base_t::Schedule().UnscheduleDelayedTask(m_task_id);
    }

    for (std::vector<TrackBase*>::size_type i = 0; i < m_tracks.size(); ++i) {

        TrackBase* cte = m_tracks[i];

        HDELP(cte);

    }

}


void StreamBase::OnStreamConnected(const StreamInfo& info, 
    const std::vector<TrackBase*>& tracks) {

    handleConnected(info, tracks);

    m_task_id = schedule_base_t::Schedule().ScheduleDelayedTask(5000000,
        std::bind(&StreamBase::rtcpWork, this));

    for (std::vector<TrackBase*>::size_type i = 0; i < m_tracks.size(); ++i) {
        TrackBase* tb = m_tracks[i];
        HN port = GetTrackRtpPort(tb->GetTrackType());
        IF_NOTOK(tb->Init(port, port + 1)) {
            LOG_ERROR("track index[%d] init failed", i);
            continue;
        }

        schedule_base_t::Schedule().SetBackgroundHandling(tb->GetRtcpSocket().Fd(),
            FIT_READ,
            std::bind(&StreamBase::client_rtcp_coming, this, tb->GetTrackType(), std::placeholders::_1));
    }

    LOG_NORMAL("stream[%s] has connected...", GetName().c_str());

}


void StreamBase::OnStreamBeginPlay() {

    LOG_NORMAL("stream[%s] begin play...", GetName().c_str());

    SetState(StreamState::PLAYING);

}


void StreamBase::OnStreamPlayingContinue(const MediaPacket& packet) {
    
    //LOG_NORMAL("OnStreamPlayingContinue packetlength[%d]", packet.GetRefBuffer().GetLength());

    for(ssobs_t::size_type i = 0; i < m_obs.size(); ++i) {

        session_ob_t ob = m_obs[i];
        
        ob->OnPacketComing(packet);

    }

}


void StreamBase::OnStreamStop() {

    LOG_NORMAL("stream[%s] stop...", GetName().c_str());

}


void StreamBase::OnStreamTerminal() {

    LOG_NORMAL("stream[%s] terminal...", GetName().c_str());

}


void StreamBase::OnStreamError() {

    LOG_NORMAL("stream[%s] ERROR...", GetName().c_str());

}


void StreamBase::AddOb(session_ob_t ob) {

    m_obs.push_back(ob);

}


void StreamBase::RemoveOb(session_ob_t ob) {

    ssobs_t::iterator fit = std::find(m_obs.begin(), m_obs.end(), ob);
    m_obs.erase(fit);

}


void StreamBase::handleConnected(const StreamInfo& info, 
    const std::vector<TrackBase*>& tracks) {

    m_start_time = info.GetStartTime();
    m_duration = info.GetDuration();
    m_bit_rate = info.GetBitRate();
    m_start_realtime = info.GetStartRealTime();

    for (std::vector<TrackBase*>::size_type i = 0; i < tracks.size(); ++i) {
        const TrackBase* ctb = tracks[i];
        TrackBase* tb = const_cast<TrackBase*>(ctb);
        addTrack(tb);
    }

    m_state = StreamState::CONNECTED;

    SetAsReady();

}


void StreamBase::handleBegin() {

}


void StreamBase::client_rtcp_coming(TRACK_TYPE tt, HUN) {

    // 在这里实现bitrate limit.
    // TODO: 这里需要区别客户端

    HSTRING_BUF buf = {0};
    TrackBase* tb = GetTrack(tt);
    const HUdpSock& sock = tb->GetRtcpSocket();
    /*HOFF ret =*/ sock.Read(buf, HSTRING_LEN);
    //LOG_NORMAL("client rtcp coming [%d]bytes type[%d]", ret, HTO_INT(tt));

}


void StreamBase::rtcpWork() {

    for(ssobs_t::size_type i = 0; i < m_obs.size(); ++i) {
        session_ob_t ob = m_obs[i];
        ob->OnRtcpActive();
    }

    m_task_id = schedule_base_t::Schedule().ScheduleDelayedTask(5000000,
        std::bind(&StreamBase::rtcpWork, this));

}


TrackBase* StreamBase::GetTrack(TRACK_TYPE tt) noexcept {

    for (std::vector<TrackBase*>::size_type i = 0; i < m_tracks.size(); ++i) {

        TrackBase* cte = m_tracks[i];

        if (cte->GetTrackType() == tt) {

            return cte;

        }

    }

    return nullptr;

}


const TrackBase* StreamBase::GetTrack(TRACK_TYPE tt) const noexcept {

    for (std::vector<TrackBase*>::size_type i = 0; i < m_tracks.size(); ++i) {
        
        const TrackBase* cte = m_tracks[i];

        if (cte->GetTrackType() == tt) {

            return cte;
            
        }
    }

    return nullptr;

}
