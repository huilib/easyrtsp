

#ifndef __H_STREAMBASE_H__
#define __H_STREAMBASE_H__

#include "streaminfo.h"
#include "trackbase.h"
#include "../sessionobserver.h"
#include "../scheduleobjectbase.h"



class StreamBase : public ScheduleOBjectbase ,public StreamObserver { 
private:
    using schedule_base_t = ScheduleOBjectbase;  
    using obverser_base_t = StreamObserver;
    using session_ob_t = SessionObserver*;
    using ssobs_t = std::vector<session_ob_t>;

public:
    StreamBase(HCSTRR strName) noexcept;
    virtual ~StreamBase() noexcept;

public:
    virtual HRET Init() = 0;

    virtual HSTR GenerateSDP() const = 0;

    void OnStreamConnected(const StreamInfo&, const std::vector<TrackBase*>&) override;

    void OnStreamBeginPlay() override;

    void OnStreamPlayingContinue(const MediaPacket& packet) override; 

    void OnStreamStop() override;

    void OnStreamTerminal() override;

    void OnStreamError() override;

    virtual HN GetTrackRtpPort(TRACK_TYPE tt) const noexcept = 0;

public:
    void AddOb(session_ob_t ob);

    void RemoveOb(session_ob_t ob);

protected:
    void handleConnected(const StreamInfo&, const std::vector<TrackBase*>&);

    void handleBegin();

    virtual void client_rtcp_coming(TRACK_TYPE, HUN);

    virtual void rtcpWork();

public:
    HN GetId() const noexcept { return m_id; }
    HCSTRR GetName() const noexcept { return m_strName; }
    StreamState GetState() const noexcept { return m_state; }    
    const std::vector<TrackBase*>& GetTracks() const noexcept { return m_tracks; }
    std::vector<TrackBase*>& GetTracks() noexcept { return m_tracks; }
    void SetState(StreamState state) noexcept { m_state = state; }
    void SetAsReady()  noexcept { m_isReady = true; }
    bool IsReady() const noexcept { return m_isReady; }    

    TrackBase* GetTrack(TRACK_TYPE tt) noexcept;
    const TrackBase* GetTrack(TRACK_TYPE tt) const noexcept;

private:
    void addTrack(TrackBase* tb) { m_tracks.push_back(tb); }

private:
    HN m_id;
    HSTR m_strName;
    bool m_isReady;

    HULL m_start_time;
    HULL m_duration;
    HULL m_bit_rate;
    HULL m_start_realtime;

    StreamState m_state;    

    std::vector<TrackBase*> m_tracks;
    ssobs_t m_obs;

    intptr_t m_task_id;
    
};


#endif // __H_STREAMBASE_H__