

#include "delayqueue.h"

int Timeval::operator>=(const Timeval& arg2) const noexcept {

    return (seconds() > arg2.seconds()) or 
        (seconds() == arg2.seconds() and useconds() >= arg2.useconds());

}


void Timeval::operator+=(const DelayInterval& arg2) noexcept {

    fTv.tv_sec += arg2.seconds(); 
    fTv.tv_usec += arg2.useconds();
    if (useconds() >= MILLION) {
        fTv.tv_usec -= MILLION;
        ++fTv.tv_sec;
    }

}


void Timeval::operator-=(const DelayInterval& arg2) noexcept {

    fTv.tv_sec -= arg2.seconds(); 
    fTv.tv_usec -= arg2.useconds();
    if ((int)useconds() < 0) {
        fTv.tv_usec += MILLION;
        --fTv.tv_sec;
    }
    if ((int)seconds() < 0) {
        fTv.tv_sec = fTv.tv_usec = 0;
    }

}


DelayInterval operator-(const Timeval& arg1, const Timeval& arg2) noexcept {
    second_t secs = arg1.seconds() - arg2.seconds();
    second_t usecs = arg1.useconds() - arg2.useconds();

    if ((int)usecs < 0) {
        usecs += Timeval::MILLION;
        --secs;
    }
    if ((int)secs < 0)
        return DELAY_ZERO;
    else
        return DelayInterval(secs, usecs);
}


///// DelayInterval /////
DelayInterval operator*(short arg1, const DelayInterval& arg2) noexcept {
    second_t result_seconds = arg1 * arg2.seconds();
    second_t result_useconds = arg1 * arg2.useconds();

    second_t carry = result_useconds/ Timeval::MILLION;
    result_useconds -= carry * Timeval::MILLION;
    result_seconds += carry;

    return DelayInterval(result_seconds, result_useconds);
}


#ifndef INT_MAX
#define INT_MAX	0x7FFFFFFF
#endif

const DelayInterval DELAY_ZERO(0, 0);
const DelayInterval DELAY_SECOND(1, 0);
const DelayInterval DELAY_MINUTE = 60*DELAY_SECOND;
const DelayInterval DELAY_HOUR = 60*DELAY_MINUTE;
const DelayInterval DELAY_DAY = 24*DELAY_HOUR;
const DelayInterval ETERNITY(INT_MAX, Timeval::MILLION - 1);


TaskHandler::TaskHandler(task_func& proc, DelayInterval timeToDelay, intptr_t token) noexcept
    : m_deltaTimeRemaining(timeToDelay), m_token(token),
    m_proc(proc) {

}


TaskHandler::~TaskHandler() noexcept {

}



///// DelayQueue /////
DelayQueue::DelayQueue() noexcept 
    : m_lastSyncTime({0,0}), m_tasks() {

    m_lastSyncTime = TimeNow();

}


DelayQueue::~DelayQueue() noexcept {

    for (task_list_t::iterator it = m_tasks.begin(); it != m_tasks.end(); ++it) {
        HDELP(*it);
    }

}


void DelayQueue::AddEntry(TaskHandler* newEntry) noexcept {

    synchronize();

    task_list_t::iterator curIt = m_tasks.begin();
    while (curIt != m_tasks.end() and  newEntry->GetDelayInterval() >= (*curIt)->GetDelayInterval()) {
        newEntry->SetDelayInterval(newEntry->GetDelayInterval() - (*curIt)->GetDelayInterval());
        curIt ++;
    }

    if (curIt != m_tasks.end()) {
        (*curIt)->SetDelayInterval((*curIt)->GetDelayInterval() - newEntry->GetDelayInterval());
    }

    m_tasks.insert(curIt, newEntry);

}

void DelayQueue::UpdateEntry(TaskHandler* entry, DelayInterval newDelay) noexcept {
    
    if (entry == nullptr) return;

    RemoveEntry(entry);

    entry->SetDelayInterval(newDelay);

    AddEntry(entry);

}

void DelayQueue::UpdateEntry(intptr_t tokenToFind, DelayInterval newDelay) noexcept {

    TaskHandler* entry = findEntryByToken(tokenToFind);
    UpdateEntry(entry, newDelay);

}

void DelayQueue::RemoveEntry(TaskHandler* entry) noexcept {

    m_tasks.remove(entry);

}

TaskHandler* DelayQueue::RemoveEntry(intptr_t tokenToFind) noexcept {

    TaskHandler* entry = findEntryByToken(tokenToFind);

    RemoveEntry(entry);

    return entry;

}

const DelayInterval&  DelayQueue::TimeToNextAlarm() noexcept {

    if (m_tasks.empty()) {
        return DELAY_ZERO;
    }

    if ((*m_tasks.cbegin())->GetDelayInterval() == DELAY_ZERO) {
        return DELAY_ZERO;
    }

    synchronize();

    return (*m_tasks.cbegin())->GetDelayInterval();

}


void DelayQueue::HandleAlarm() {
    
    if (m_tasks.empty()) {        
        return ;
    }

    if ((*m_tasks.cbegin())->GetDelayInterval() != DELAY_ZERO) {
        synchronize();
    }

    std::vector<TaskHandler*> timeup_tasks;

    for (task_list_t::iterator it = m_tasks.begin(); it != m_tasks.end(); ) {

        TaskHandler* th = *it;

        if (th->GetDelayInterval() == DELAY_ZERO) {

            it = m_tasks.erase(it);

            timeup_tasks.push_back(th);            

        }

        break;

    }

    for (std::vector<TaskHandler*>::size_type i = 0; i < timeup_tasks.size(); ++i) {
        TaskHandler* th = timeup_tasks[i];

        th->HandleTimeout();

        HDELP(th);
    }
    

    /*if ((*m_tasks.cbegin())->GetDelayInterval() == DELAY_ZERO) {
        // This event is due to be handled:
        TaskHandler* th = *m_tasks.begin();

        RemoveEntry(th); // do this first, in case handler accesses queue

        th->HandleTimeout();

        HDELP(th);
    }*/
}

TaskHandler* DelayQueue::findEntryByToken(intptr_t tokenToFind) noexcept {

    for(task_list_t::iterator it = m_tasks.begin(); it != m_tasks.end(); ++it) {
        if ((*it)->token() == tokenToFind) {
            return *it;
        }
    }

    return nullptr;

}

void DelayQueue::synchronize() noexcept {
    // First, figure out how much time has elapsed since the last sync:
    DelayEventTime timeNow = TimeNow();
    if (timeNow < m_lastSyncTime) {
        // The system clock has apparently gone back in time; reset our sync time and return:
        m_lastSyncTime  = timeNow;
        return;
    }
    DelayInterval timeSinceLastSync = timeNow - m_lastSyncTime;
    m_lastSyncTime = timeNow;

    // Then, adjust the delay queue for any entries whose time is up:
    task_list_t::iterator curIt = m_tasks.begin();
    while (curIt != m_tasks.end() and  timeSinceLastSync >= (*curIt)->GetDelayInterval()) {
        timeSinceLastSync -= (*curIt)->GetDelayInterval();
        (*curIt)->SetDelayInterval(DELAY_ZERO);
        curIt ++;
    }

    if (curIt != m_tasks.end()) {
        (*curIt)->SetDelayInterval((*curIt)->GetDelayInterval() - timeSinceLastSync);
    }

}

///// _EventTime /////
DelayEventTime TimeNow() {

    struct timeval tvNow;

    gettimeofday(&tvNow, NULL);

    return DelayEventTime(tvNow.tv_sec, tvNow.tv_usec);

}

const DelayEventTime THE_END_OF_TIME(INT_MAX);


