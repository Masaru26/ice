// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEUTIL_TIMER_H
#define ICEUTIL_TIMER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Time.h>

#include <set>
#include <map>

namespace IceUtil
{

class Timer;
typedef IceUtil::Handle<Timer> TimerPtr;

//
// Extend the TimerTask class and override the run() method to execute
// code at a specific time or repeatedly.
//
class ICE_UTIL_API TimerTask : virtual public IceUtil::Shared
{
public:

    virtual ~TimerTask() { }

    virtual bool operator<(const TimerTask& r) const;

    virtual void run() = 0;
};
typedef IceUtil::Handle<TimerTask> TimerTaskPtr;

//
// The timer class is used to schedule tasks for one-time execution or
// repeated execution. Tasks are executed by the dedicated timer thread 
// sequentially.
//
class ICE_UTIL_API Timer : public virtual IceUtil::Shared, private virtual IceUtil::Thread
{
public:

    //
    // Construct a timer and starts its execution thread.
    //
    Timer();

    //
    // Destroy the timer and join with its execution thread.
    //
    void destroy();

    //
    // Schedule a task for execution after a given delay.
    //
    void schedule(const TimerTaskPtr&, const IceUtil::Time&);

    //
    // Schedule a task for repeated execution with the given delay
    // between each execution.
    //
    void scheduleRepeated(const TimerTaskPtr&, const IceUtil::Time&);

    //
    // Cancel a task. Returns true if the task has not yet run or if
    // it's a task scheduled for repeated execution. Returns false if
    // the task has already run, was already cancelled or was never
    // schedulded.
    //
    bool cancel(const TimerTaskPtr&);

private:

    struct Token
    {
        IceUtil::Time scheduledTime;
        IceUtil::Time delay;
        TimerTaskPtr task;

        inline Token(const IceUtil::Time&, const IceUtil::Time&, const TimerTaskPtr&);
        inline bool operator<(const Token& r) const;
    };

    virtual void run();

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    bool _destroyed;
    std::set<Token> _tokens;
    std::map<TimerTaskPtr, IceUtil::Time> _tasks;
    IceUtil::Time _wakeUpTime;
};
typedef IceUtil::Handle<Timer> TimerPtr;

inline 
Timer::Token::Token(const IceUtil::Time& st, const IceUtil::Time& d, const TimerTaskPtr& t) :
    scheduledTime(st), delay(d), task(t)
{
}

inline bool
Timer::Token::operator<(const Timer::Token& r) const
{
    if(scheduledTime < r.scheduledTime)
    {
        return true;
    }
    else if(scheduledTime > r.scheduledTime)
    {
        return false;
    }
    
    return task < r.task;
}

}

#endif

