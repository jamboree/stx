/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_SYNC_EVENT_HPP_INCLUDED
#define STX_SYNC_EVENT_HPP_INCLUDED

#include <mutex>
#include <condition_variable>

namespace stx
{
    class event
    {
        bool _not_ready;
        std::mutex _mtx;
        std::condition_variable _cond;

    public:

        event() : _not_ready(true) {}

        void set()
        {
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _not_ready = false;
            }
            _cond.notify_all();
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock(_mtx);
            while (_not_ready)
                _cond.wait(lock);
        }
    };
}

#endif