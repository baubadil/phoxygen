/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#include "xwp/worker.h"

#include <thread>

struct WorkerThread::Impl
{
    Impl(std::function<void ()> &fn)
        : thr(fn)
    { }

    std::thread     thr;
};

/* static */
PWorkerThread WorkerThread::Create(std::function<void ()> fn)
{
    class Derived : public WorkerThread
    {
    public:
        Derived(std::function<void ()> fn) : WorkerThread(fn) { };
    };

    auto p = make_shared<Derived>(fn);
    return p;
}

WorkerThread::WorkerThread(std::function<void ()> fn)
    : _pImpl(new Impl(fn))
{

}

WorkerThread::~WorkerThread()
{
    delete _pImpl;
}

bool WorkerThread::isRunning() const
{
    return _pImpl->thr.get_id() != std::thread::id();
}

void WorkerThread::join()
{
    _pImpl->thr.join();
}
