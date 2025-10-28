

#include "mediaslotbase.h"

MediaSlotBase::MediaSlotBase(HCSTRR strName) noexcept
    : m_strName(strName), m_observers(), m_bStop(false) {

}


MediaSlotBase::~MediaSlotBase() noexcept {

}



void MediaSlotBase::handleInited(const StreamInfo& info, 
    const std::vector<TrackBase*>& tracks) const {

    for(observers_t::size_type i = 0; i < m_observers.size(); ++i) {

        observer_ptr ptr = m_observers[i];

        ptr->OnStreamConnected(info, tracks);

    }

}


void MediaSlotBase::handleBeginPlay() const {

    for(observers_t::size_type i = 0; i < m_observers.size(); ++i) {

        observer_ptr ptr = m_observers[i];

        ptr->OnStreamBeginPlay();

    }

}


void MediaSlotBase::handleStreamPacket(const MediaPacket& packet) const {

    for(observers_t::size_type i = 0; i < m_observers.size(); ++i) {

        observer_ptr ptr = m_observers[i];

        ptr->OnStreamPlayingContinue(packet);

    }

}


void MediaSlotBase::handleStop() const {

    for(observers_t::size_type i = 0; i < m_observers.size(); ++i) {

        observer_ptr ptr = m_observers[i];

        ptr->OnStreamStop();

    }

}


void MediaSlotBase::handleTerminal() const {

    for(observers_t::size_type i = 0; i < m_observers.size(); ++i) {

        observer_ptr ptr = m_observers[i];

        ptr->OnStreamTerminal();

    }

}


void MediaSlotBase::handleError() const {

    for(observers_t::size_type i = 0; i < m_observers.size(); ++i) {

        observer_ptr ptr = m_observers[i];

        ptr->OnStreamError();

    }

}


void MediaSlotBase::AddObserver(observer_ptr observer) noexcept {

    m_observers.push_back(observer);

}


