

#include "fdhandler.h"


FdHandler::FdHandler(FD_T fd, HUN care_events, fd_handler_callback_t call_back) noexcept
    : m_fd(fd), m_care_events(care_events), m_callback(call_back) {

}


FdHandler::~FdHandler() {

}


void FdHandler::Call(HUN evs) const {
    if (m_callback) {
        m_callback(evs);
    }
}


FdHandlerMap::FdHandlerMap()
    : m_fd_map() {

}


FdHandlerMap::~ FdHandlerMap() {

}


void FdHandlerMap::AddHandler(FD_T fd, HUN care_events, fd_handler_callback_t call_back) {

    fd_map_t::iterator fit = m_fd_map.find(fd);
    if (fit == m_fd_map.end()) {
        // no-exists.
        FdHandler* handler = new FdHandler(fd, care_events, call_back);
        CHECK_NEWPOINT(handler);
        m_fd_map.insert(fd_map_t::value_type(fd, handler));
    } else {
        // exitst.
        FdHandler* handler = fit->second;
        handler->SetCareEvents(care_events);
        handler->SetCallback(call_back);
    }

}


void FdHandlerMap::RemoveHandler(FD_T fd) {

    fd_map_t::const_iterator fit = m_fd_map.find(fd);
    if (fit != m_fd_map.cend()) {
        const FdHandler* ptr = fit->second;
        HDELP(ptr);
        m_fd_map.erase(fit);
    }

}


FD_T FdHandlerMap::SetupFdsets(fd_set& rfds, fd_set& wfds, fd_set& efds) const {

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    FD_T res = 0;
    for (fd_map_t::const_iterator cit = m_fd_map.cbegin(); cit != m_fd_map.cend(); ++cit) {

        const FdHandler* fdh = cit->second;
        if ((fdh->GetCareEvent() & FIT_TEST) == 0) {
            continue;
        }

        if (res < fdh->GetFd()) {
            res = fdh->GetFd();
        }

        if (fdh->GetCareEvent() & FIT_READ) {
            FD_SET(fdh->GetFd(), &rfds);
        }

        if (fdh->GetCareEvent() & FIT_WRITE) {
            FD_SET(fdh->GetFd(), &wfds);
        }

        if (fdh->GetCareEvent() & FIT_EXCEP) {
            FD_SET(fdh->GetFd(), &efds);
        }
                        
    }

    return res;

}


void FdHandlerMap::CallHandler(const fd_set& rfds, const fd_set& wfds, const fd_set& efds) const {

    for (fd_map_t::const_iterator cit = m_fd_map.cbegin(); cit != m_fd_map.cend(); ++cit) {

        const FdHandler* fdh = cit->second;

        HUN result_events = 0;
        if (FD_ISSET(fdh->GetFd(), &rfds)) {
            result_events |= FIT_READ;
        }

        if (FD_ISSET(fdh->GetFd(), &wfds)) {
            result_events |= FIT_WRITE;
        }

        if (FD_ISSET(fdh->GetFd(), &efds)) {
            result_events |= FIT_EXCEP;
        }

        if ((result_events & fdh->GetCareEvent()) != 0) {
            fdh->Call(result_events);
        }
    }    

}
