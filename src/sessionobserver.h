

#ifndef __H_SESSION_OBSERVER_H__
#define __H_SESSION_OBSERVER_H__

#include <huicpp.h>
#include "mediasource/streaminfo.h"

using namespace HUICPP;


class SessionObserver {
public:
    SessionObserver() noexcept = default;

    ~SessionObserver() noexcept = default;

public:
    virtual void OnPacketComing(const MediaPacket& packet) = 0;

    virtual void OnRtcpActive() { }

    virtual void OnStreamStop() { }

    virtual void OnStreamTerminal() {  }

    virtual void OnStreamError() {  }

};


#endif //__H_SESSION_OBSERVER_H__