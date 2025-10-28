

#ifndef __H_MEDIA_SLOT_BASE_H__
#define __H_MEDIA_SLOT_BASE_H__

#include "streaminfo.h"


class MediaSlotBase {
private:
    using observer_ptr = StreamObserver*;
    using observers_t = std::vector<observer_ptr>;

public:
    MediaSlotBase(HCSTRR strName) noexcept;

    virtual ~MediaSlotBase() noexcept;

public:
    virtual HRET Init() = 0;

    virtual void Stop() { m_bStop = true; }  

protected:
    void handleInited(const StreamInfo&, const std::vector<TrackBase*>&) const;

    void handleBeginPlay() const;

    void handleStreamPacket(const MediaPacket& packet) const;

    void handleStop() const;

    void handleTerminal() const;

    void handleError() const;

public:
    void AddObserver(observer_ptr observer) noexcept;

    HCSTRR GetMediaName() const noexcept { return m_strName; }

    bool IsStop() const noexcept { return m_bStop; }

private:
    HSTR m_strName;
    mutable observers_t m_observers;
    bool m_bStop;
};






#endif // __H_MEDIA_SLOT_BASE_H__

