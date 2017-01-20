/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_UTILITY_PRIORITY_HPP_INCLUDED
#define STX_UTILITY_PRIORITY_HPP_INCLUDED

namespace stx
{
    template<unsigned N>
    struct priority : priority<N - 1> {};

    template<>
    struct priority<0> {};
}

#endif