/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_ALGORITHM_UNSTABLE_REMOVE_HPP_INCLUDED
#define STX_ALGORITHM_UNSTABLE_REMOVE_HPP_INCLUDED

#include <utility>

namespace stx
{
    template<class BidIt, class UnaryPred>
    BidIt unstable_remove_if(BidIt it, BidIt end, UnaryPred&& pred)
    {
        for (; it != end; ++it)
        {
            if (pred(*it))
            {
                do
                {
                    if (--end == it)
                        return it;
                } while (pred(*end));
                *it = std::move(*end);
            }
        }
        return it;
    }

    template<class BidIt, class T>
    inline BidIt unstable_remove(BidIt it, BidIt end, T const& val)
    {
        return unstable_remove_if(it, end, [&val](auto const& i) {return i == val; });
    }
}

#endif