/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_ALGORITHM_APPLY_PERMUTATION_HPP_INCLUDED
#define STX_ALGORITHM_APPLY_PERMUTATION_HPP_INCLUDED

#include <utility>

namespace stx
{
    template<class RandValueIt, class RandIndexIt>
    void apply_permutation(RandValueIt val_it, RandValueIt val_end, RandIndexIt idx_it)
    {
        auto count = val_end - val_it;
        using D = decltype(count);
        D i = 0;
        for (; i != count; ++i)
        {
            if (idx_it[i] == i)
                continue;
            auto tmp(std::move(val_it[i]));
            D curr = i;
            do
            {
                D next = idx_it[curr];
                val_it[curr] = std::move(val_it[next]);
                idx_it[curr] = curr;
                curr = next;
            } while (idx_it[curr] != i);
            val_it[curr] = std::move(tmp);
            idx_it[curr] = curr;
        }
    }
}

#endif