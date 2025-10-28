

#include "mediaslotmng.h"


HRET MediaSlotMng::CreateMp4Slot(HCSTRR strFileName, StreamObserver* ob) {

    media_slot_map_t::const_iterator cfit = m_media_slot_map.find(strFileName);
    HASSERT_RETURN(cfit ==  m_media_slot_map.cend(), SRC_ERR);

    Mp4MediaSlot* mp4slot = new Mp4MediaSlot(strFileName);
    NULLPOINTER_CHECK(mp4slot);

    mp4slot->AddObserver(ob);

    m_media_slot_map[strFileName] = mp4slot;

    return mp4slot->Init();

}