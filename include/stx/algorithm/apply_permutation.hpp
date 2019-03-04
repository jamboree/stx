/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017-2019 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_ALGORITHM_APPLY_PERMUTATION_HPP_INCLUDED
#define STX_ALGORITHM_APPLY_PERMUTATION_HPP_INCLUDED

#include <utility>
#include <iterator>

namespace stx
{
    template<class RandValueIt, class RandIndexIt>
    void apply_permutation(RandValueIt val_it, RandValueIt val_end, RandIndexIt idx_it)
    {
        using D = typename std::iterator_traits<RandIndexIt>::value_type;
        D count = D(val_end - val_it);
        for (D i = 0; i != count; ++i)
        {
            D next = idx_it[i];
            if (next == i)
                continue;
            auto tmp(std::move(val_it[i]));
            D curr = i;
            do
            {
                val_it[curr] = std::move(val_it[next]);
                idx_it[curr] = curr;
                curr = next;
                next = idx_it[curr];
            } while (next != i);
            val_it[curr] = std::move(tmp);
            idx_it[curr] = curr;
        }
    }
}

#endif