/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_BINARY_SEARCH_HPP_INCLUDED
#define STDEX_BINARY_SEARCH_HPP_INCLUDED

#include <functional>
#include <type_traits>
#include <stdex/is_iterator.hpp>

namespace stdex
{
    template<class It, class T, class Cmp>
    std::enable_if_t<is_random_access_iterator<It>::value, It>
    binary_search(It first, It last, T const& val, Cmp&& cmp)
    {
        It not_found(last);
        while (first != last)
        {
            It it(first + (last - first >> 1));
            if (cmp(val, *it))
                last = it;
            else if (cmp(*it, val))
                first = ++it;
            else
                return it;
        }
        return not_found;
    }

    template<class It, class T>
    inline std::enable_if_t<is_random_access_iterator<It>::value, It>
    binary_search(It first, It last, T const& val)
    {
        return stdex::binary_search(first, last, val, std::less<>{});
    }

    template<class Con, class T, class Cmp>
    inline auto binary_search(Con&& con, T const& val, Cmp&& cmp)
    {
        using std::begin;
        using std::end;
        return stdex::binary_search(begin(con), end(con), val, cmp);
    }

    template<class Con, class T>
    inline auto binary_search(Con&& con, T const& val)
    {
        return stdex::binary_search(con, val, std::less<>{});
    }
}

#endif