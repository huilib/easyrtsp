

#ifndef __H_VIDEO_TRACK_H__
#define __H_VIDEO_TRACK_H__

#include "trackbase.h"

class VideoTrack : public TrackBase {
private:
    using base_class_t = TrackBase;

public:
    VideoTrack() noexcept;

    virtual ~ VideoTrack() noexcept;

public:
    void SetWidth(HN width) noexcept { m_width = width; }
    void SetHeight(HN height) noexcept { m_height = height; }
    void SetFps(HN fps) noexcept { m_fps = fps; }
    void SetFieldOrder(HN field_order) noexcept { m_field_order = field_order; }

    HN GetWidth() const noexcept { return m_width; }
    HN GetHeight() const noexcept { return m_height; }
    HN GetFps() const noexcept { return m_fps; }
    HN GetFieldOrder() const noexcept { return m_field_order; }

public:
    HN m_width;
    HN m_height;
    HN m_fps;
    HN m_field_order;
};

class H264or265Track : public VideoTrack {
private:
    using base_class_t = VideoTrack;
    static constexpr const HN BUFFER_LENGTH = 128;

public:
    H264or265Track() noexcept;
    virtual ~ H264or265Track() noexcept;

public:
    HCMEMR GetSps() const noexcept { return m_sps; }
    HCMEMR GetPps() const noexcept { return m_pps; }
    HSTR GetProfileLevel() const noexcept;

    HRET SetupSpsAndPps(HCMEMR memSps, HCMEMR memPps) noexcept;

protected:
    void setupWeb();

private:
    void uninit();

private:
    HMEM m_sps;
    HMEM m_pps;
    HUSZ m_sps_web;
    HN m_sps_web_len;
};

class H264VideoTrack : public H264or265Track {
private:
    using base_class_t = H264or265Track;
public:
    H264VideoTrack() noexcept;

    ~ H264VideoTrack() noexcept;

public:
    HSTR GenerateSDP() const override;

};


#endif //__H_VIDEO_TRACK_H__