

#ifndef __H_MEDIA_SLOT_MNG_H__
#define __H_MEDIA_SLOT_MNG_H__


#include "mp4mediaslot.h"
#include <hsingleton.h>


class MediaSlotMng {
private:
    using media_slot_map_t = std::map<HSTR, MediaSlotBase*>;

public:
    MediaSlotMng() = default;
    ~MediaSlotMng() = default;

public:
    HRET CreateMp4Slot(HCSTRR strFileName, StreamObserver*);

private:
    media_slot_map_t m_media_slot_map;
};


using media_slot_mng = HSingleton<MediaSlotMng>;


#endif // __H_MEDIA_SLOT_MNG_H__
