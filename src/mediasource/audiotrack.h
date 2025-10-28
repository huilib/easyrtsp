

#ifndef __H_AUDIO_TRACK_H__
#define __H_AUDIO_TRACK_H__

#include "trackbase.h"

class AudioTrack: public TrackBase {
private:
    using track_base_t = TrackBase;

public:
    AudioTrack() noexcept;
    virtual ~ AudioTrack() noexcept;

public:
    HSTR GenerateSDP() const;

public:
    HN GetChannelNumber() const noexcept { return m_channel_number; }
    HN GetSampleRate() const noexcept { return m_sample_rate; }
    HN GetBlockAlign () const noexcept { return m_block_align; } 
    HN GetFrameSize() const noexcept { return m_frame_size; }
    HN GetInitialPadding() const noexcept { return m_initial_padding; }
    HN GetTrailingPadding() const noexcept { return m_trailing_padding; }

    void SetChannelNumber(HN channel_number) noexcept { m_channel_number = channel_number; }
    void SetSampleRate(HN sample_rate) noexcept { m_sample_rate = sample_rate; }
    void SetBlockAlign(HN block_align) noexcept { m_block_align = block_align; }
    void SetFrameSize(HN frame_size) noexcept { m_frame_size = frame_size; }
    void SetInitialPadding(HN initial_padding) noexcept { m_initial_padding = initial_padding; }
    void SetTrailingPadding(HN trailing_padding) noexcept { m_trailing_padding = trailing_padding; }


private:
    HN m_channel_number;
    HN m_sample_rate;
    HN m_block_align;
    HN m_frame_size;
    HN m_initial_padding;
    HN m_trailing_padding;
};


#endif // __H_AUDIO_TRACK_H__

