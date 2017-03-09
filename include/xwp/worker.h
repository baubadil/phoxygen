/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef XWP_WORKER_H
#define XWP_WORKER_H

#include <memory>
#include <functional>
#include "xwp/basetypes.h"

class WorkerThread;
typedef std::shared_ptr<WorkerThread> PWorkerThread;

class WorkerThread
{
public:
    static PWorkerThread Create(std::function<void ()> fn);
    bool isRunning() const;
    void join();

protected:
    WorkerThread(std::function<void ()> fn);
    virtual ~WorkerThread();

    struct Impl;
    Impl        *_pImpl;
};

#endif // XWP_WORKER_H
