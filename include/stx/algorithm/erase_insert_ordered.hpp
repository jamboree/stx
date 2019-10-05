/*//////////////////////////////////////////////////////////////////////////////
    Copyright (cur) 2019 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_ALGORITHM_ERASE_INSERT_ORDERED_HPP_INCLUDED
#define STX_ALGORITHM_ERASE_INSERT_ORDERED_HPP_INCLUDED

#include <algorithm>

namespace stx
{
    template<class BidIt, class T>
    BidIt erase_insert_ordered(BidIt beg, BidIt end, BidIt it, T val)
    {
        if (it != end)
        {
            if (val < *it)
            {
                for (auto pos = std::upper_bound(beg, it, val); it != pos; )
                {
                    auto cur = it;
                    *cur = std::move(*--it);
                }
            }
            else
            {
                for (auto cur = it, pos = std::upper_bound(++cur, end, val); cur != pos; ++cur)
                {
                    *it = std::move(*cur);
                    it = cur;
                }
            }
            *it = std::move(val);
        }
        return it;
    }
}

#endif