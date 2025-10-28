
#ifndef __H_MP4_MEDIA_SLOT_H__
#define __H_MP4_MEDIA_SLOT_H__

#include "mediaslotbase.h"
#include "ffmpegdev.h"
#include "streaminfo.h"
#include "../scheduleobjectbase.h"

class Mp4MediaSlot : public MediaSlotBase,
    public FfmpegMediaDev,
    public ScheduleOBjectbase {
private:
    using slot_base_t = MediaSlotBase;
    using dev_base_t = FfmpegMediaDev;
    using schedule_base_t = ScheduleOBjectbase;

    typedef struct tagSleepTime {
        HLL v_needs;
        HLL a_needs;
    } sleep_time_t;

public:
    Mp4MediaSlot(HCSTRR strFileName) noexcept;

    ~ Mp4MediaSlot();

public:
    HRET Init() override;

private:
    HRET InComing(HPTR ptr) override;

    void pull_packet();

    void onFmtExtraData(const uint8_t*, int) override;

    void onBsfExtraData(const uint8_t* data, int len) override;

    HRET do_incoming_video_packet(HPTR ptr);

    HRET do_incoming_audio_packet(HPTR ptr);

    void setup_sps_pps_avcc(const uint8_t*, int);

    void setup_schedule();

private:  
    HMEM m_sps;
    HMEM m_pps;

    // control information.
    intptr_t m_task_id;
    sleep_time_t m_sleep_time;
};

#endif // __H_MP4_MEDIA_SLOT_H__