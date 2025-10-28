

#ifndef __H_DELAYQUEUE_H__
#define __H_DELAYQUEUE_H__

#include <huicpp.h>
#include <functional>
#include <sys/time.h>
#include <list>

using namespace HUICPP;

using second_t = HLN;

using task_func = std::function<void()>;

///// A "Timeval" can be either an absolute time, or a time interval /////
class Timeval {
public:
    static const constexpr HN MILLION = 1000000;

public:
    second_t seconds() const noexcept {
      return fTv.tv_sec;
    }
    second_t useconds() const noexcept{
      return fTv.tv_usec;
    }

    int operator>=(const Timeval& arg2) const noexcept;
    int operator<=(const Timeval& arg2) const noexcept{
      return arg2 >= *this;
    }
    int operator<(Timeval const& arg2) const noexcept{
      return !(*this >= arg2);
    }
    int operator>(Timeval const& arg2) const noexcept{
      return arg2 < *this;
    }
    int operator==(Timeval const& arg2) const noexcept{
      return *this >= arg2 && arg2 >= *this;
    }
    int operator!=(Timeval const& arg2) const noexcept {
      return !(*this == arg2);
    }

    void operator+=(class DelayInterval const& arg2) noexcept;
    void operator-=(class DelayInterval const& arg2) noexcept;

protected:
    Timeval(second_t seconds, second_t useconds) noexcept {
      fTv.tv_sec = seconds; fTv.tv_usec = useconds;
    }

private:   
    struct timeval fTv;
};


///// DelayInterval /////
class DelayInterval: public Timeval {
public:
  DelayInterval(second_t seconds, second_t useconds) noexcept
    : Timeval(seconds, useconds) {}
};

DelayInterval operator-(Timeval const& arg1, Timeval const& arg2) noexcept;

DelayInterval operator*(short arg1, DelayInterval const& arg2) noexcept;


extern DelayInterval const DELAY_ZERO;
extern DelayInterval const DELAY_SECOND;
extern DelayInterval const DELAY_MINUTE;
extern DelayInterval const DELAY_HOUR;
extern DelayInterval const DELAY_DAY;


class DelayEventTime: public Timeval {
public:
  DelayEventTime(unsigned secondsSinceEpoch = 0,
	    unsigned usecondsSinceEpoch = 0) noexcept
    // We use the Unix standard epoch: January 1, 1970
    : Timeval(secondsSinceEpoch, usecondsSinceEpoch) {}
};


DelayEventTime TimeNow();

extern DelayEventTime const THE_END_OF_TIME;


class TaskHandler {
public:
    TaskHandler(task_func& proc, DelayInterval timeToDelay, intptr_t token) noexcept;

    ~TaskHandler() noexcept;

public:
    intptr_t token() const noexcept {
      return m_token;
    }

    void HandleTimeout() {
      if (m_proc) {
        m_proc();
      }
    }

    const DelayInterval& GetDelayInterval() const noexcept { return m_deltaTimeRemaining; }

    void SetDelayInterval(const DelayInterval& newval) noexcept { m_deltaTimeRemaining = newval; }

private:
    DelayInterval m_deltaTimeRemaining;
    intptr_t m_token;
    task_func m_proc;
};

///// DelayQueue /////
class DelayQueue {
public:
    DelayQueue() noexcept;
    virtual ~DelayQueue() noexcept;

public:
    void AddEntry(TaskHandler* newEntry) noexcept; // returns a token for the entry
    void UpdateEntry(TaskHandler* entry, DelayInterval newDelay) noexcept;
    void UpdateEntry(intptr_t tokenToFind, DelayInterval newDelay) noexcept;
    void RemoveEntry(TaskHandler* entry) noexcept; // but doesn't delete it
    TaskHandler* RemoveEntry(intptr_t tokenToFind) noexcept; // but doesn't delete it

    const DelayInterval& TimeToNextAlarm() noexcept;
    void HandleAlarm();

private:

    TaskHandler* findEntryByToken(intptr_t token) noexcept;
    void synchronize() noexcept; // bring the 'time remaining' fields up-to-date

private:
    using task_list_t = std::list<TaskHandler*>;
    DelayEventTime m_lastSyncTime;
    task_list_t m_tasks;
};


#endif // __H_DELAYQUEUE_H__

