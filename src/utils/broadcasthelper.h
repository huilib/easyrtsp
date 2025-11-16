
#ifndef __H_BROADCASTHELPER_H__
#define __H_BROADCASTHELPER_H__

#include <hsocket.h>
#include <haddr.h>

using namespace HUICPP;


class BroadcastHelper {
private:
    class broad_cast_item {
    public:
        broad_cast_item(const HUdpSock& sock, HCSTRR strIp);

        ~broad_cast_item();

    private:
        const HUdpSock& m_sock;
        HSTR m_strIp;
    };

public:
    BroadcastHelper() noexcept = default;

    ~BroadcastHelper();

public:
    void AddBroadcast(const HUdpSock& sock, HCSTRR strIP);

private:
    std::vector<broad_cast_item*> m_items;

};


#endif //__H_BROADCASTHELPER_H__

