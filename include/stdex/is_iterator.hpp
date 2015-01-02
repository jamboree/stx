/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_IS_ITERATOR_HPP_INCLUDED
#define STDEX_IS_ITERATOR_HPP_INCLUDED

#include <iterator>

namespace stdex
{
    template<class It, class Tag>
    using is_iterator =
        std::is_base_of<Tag, typename std::iterator_traits<It>::iterator_category>;

    template<class It>
    using is_input_iterator =
        is_iterator<It, std::input_iterator_tag>;
}

#endif