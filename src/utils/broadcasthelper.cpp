
#include "broadcasthelper.h"


BroadcastHelper::broad_cast_item::broad_cast_item(const HUdpSock& sock, HCSTRR strIp)
    : m_sock(sock), m_strIp(strIp) {

    HIGNORE_RETURN(m_sock.JoinBroadcast(strIp));

}


BroadcastHelper::broad_cast_item::~broad_cast_item() {

    HIGNORE_RETURN(m_sock.LeaveBroadcast(m_strIp));

}



BroadcastHelper::~BroadcastHelper() {

    for (std::vector<broad_cast_item*>::size_type i = 0; i < m_items.size(); ++i) {

        broad_cast_item* item = m_items[i];

        HDELP(item);

    }

}



void BroadcastHelper::AddBroadcast(const HUdpSock& sock, HCSTRR strIP) {

    broad_cast_item* item = new broad_cast_item(sock, strIP);

    CHECK_NEWPOINT(item);

    m_items.push_back(item);

}


