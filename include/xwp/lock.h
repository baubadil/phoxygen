/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef XWP_LOCK_H
#define XWP_LOCK_H

#include <mutex>

namespace XWP
{

/***************************************************************************
 *
 *  Lock
 *
 **************************************************************************/

class Lock
{
public:
    Lock(std::recursive_mutex &m)
        : _m(m)
    {
        _m.lock();
        _fLocked = true;
    }

    ~Lock()
    {
        release();
    }

    void release()
    {
        if (_fLocked)
            _m.unlock();
        _fLocked = false;
    }

protected:
    std::recursive_mutex    &_m;
    bool                    _fLocked = false;
};


}

#endif // XWP_LOCK_H
