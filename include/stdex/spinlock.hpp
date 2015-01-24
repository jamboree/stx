/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_PRIORITY_HPP_INCLUDED
#define STDEX_PRIORITY_HPP_INCLUDED

#include <atomic>

namespace stdex
{
	struct spinlock
	{
		explicit spinlock(std::atomic_flag& mutex) : _mutex(mutex)
		{
			while (!mutex.test_and_set(std::memory_order_acquire));
		}

		~spinlock()
		{
			_mutex.clear(std::memory_order_release);
		}

	private:

		std::atomic_flag& _mutex;
	};
}

#endif