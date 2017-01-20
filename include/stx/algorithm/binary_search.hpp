/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_ALGORITHM_BINARY_SEARCH_HPP_INCLUDED
#define STX_ALGORITHM_BINARY_SEARCH_HPP_INCLUDED

#include <functional>

namespace stx
{
    template<class RandIt, class T, class Cmp>
    RandIt binary_search(RandIt first, RandIt last, T const& val, Cmp&& cmp)
    {
        RandIt not_found(last);
        while (first != last)
        {
            RandIt it(first + (last - first >> 1));
            if (cmp(val, *it))
                last = it;
            else if (cmp(*it, val))
                first = ++it;
            else
                return it;
        }
        return not_found;
    }

    template<class RandIt, class T>
    inline RandIt binary_search(RandIt first, RandIt last, T const& val)
    {
        return binary_search(first, last, val, std::less<>{});
    }
}

#endif