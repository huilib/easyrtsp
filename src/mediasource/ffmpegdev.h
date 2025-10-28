

#ifndef __H_FFMPEG_MEDIA_SLOT_H__
#define __H_FFMPEG_MEDIA_SLOT_H__

#include <hdevobjectbase.h>

using namespace HUICPP;

class FfmpegMediaDev: public HSourceDevBase {
private:
    using base_class_t = HSourceDevBase;

public:
    FfmpegMediaDev(HCSTRR strName) noexcept;

    virtual ~FfmpegMediaDev();

public:
    virtual HRET Init() override;

    HRET GetPacket();

protected:
    HCPTR getFmtCtx() const noexcept { return m_fmt; }

    HN getVideoIndex() const noexcept { return m_video_index; }
    HN getAudioIndex() const noexcept { return m_audio_index; }

    HN getSequence() const noexcept { return m_sequence; }

    virtual void onFmtExtraData(const uint8_t*, int) = 0;

    virtual void onBsfExtraData(const uint8_t* data, int len) = 0;

private:
    void uninit() override;

private:
    HSTR m_strName;
    HPTR m_fmt;   
    HPTR m_bsf_ctx;
    HPTR m_src_pkt;
    HPTR m_out_pkt; 
    HN m_video_index;
    HN m_audio_index;
    HN m_sequence;
};


#endif //__H_FFMPEG_MEDIA_SLOT_H__

