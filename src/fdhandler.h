

#ifndef __H_FD_HANDLER_H__
#define __H_FD_HANDLER_H__

#include <huicpp.h>
#include <functional>

#include <sys/select.h>

using namespace HUICPP;

using fd_handler_callback_t = std::function<void(HUN)>;

class FdHandler {
public:
    FdHandler(FD_T fd, HUN care_events, fd_handler_callback_t call_back) noexcept;

    ~FdHandler() noexcept;

public:
    void SetCareEvents(HUN care_events) noexcept { m_care_events = care_events; }    

    void SetCallback(fd_handler_callback_t callback) noexcept { m_callback = callback; }

    FD_T GetFd() const noexcept { return m_fd; }

    HUN GetCareEvent() const noexcept { return m_care_events; }

    void Call(HUN evs) const;

private:
    FD_T m_fd;
    HUN m_care_events;
    fd_handler_callback_t m_callback;
};

enum FD_IO_TYPE {
    FIT_NONE = 0,
    FIT_READ = 1 << 0,
    FIT_WRITE = 1 << 1,
    FIT_EXCEP = 1 << 2,
    FIT_TEST = FIT_READ | FIT_WRITE | FIT_EXCEP
};

class FdHandlerMap {
private:
    using fd_map_t = std::map<FD_T, FdHandler*>;

public:
    FdHandlerMap() ;

    ~ FdHandlerMap();

public:
    void AddHandler(FD_T fd, HUN care_events, fd_handler_callback_t call_back);
    void RemoveHandler(FD_T fd);

    FD_T SetupFdsets(fd_set& rfds, fd_set& wfds, fd_set& efds) const;

    void CallHandler(const fd_set& rfds, const fd_set& wfds, const fd_set& efds) const;

private:
    fd_map_t m_fd_map;
};


#endif // __H_FD_HANDLER_H__