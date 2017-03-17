/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_TYPE_TRAITS_IS_ITERATOR_HPP_INCLUDED
#define STX_TYPE_TRAITS_IS_ITERATOR_HPP_INCLUDED

#include <iterator>
#include <type_traits>
#include <stx/type_traits/enable_if_valid.hpp>

namespace stx { namespace detail
{
    template<class It>
    using iterator_category_t =
        typename std::iterator_traits<It>::iterator_category;

    template<class It, class Tag, class Enable = void>
    struct is_iterator_impl : std::false_type {};

    template<class It>
    struct is_iterator_impl<It, void, enable_if_valid_t<iterator_category_t<It>>>
      : std::true_type
    {};

    template<class It, class Tag>
    struct is_iterator_impl<It, Tag, enable_if_valid_t<iterator_category_t<It>>>
      : std::is_base_of<Tag, iterator_category_t<It>>
    {};
}}

namespace stx
{
    template<class It, class Tag = void>
    using is_iterator = detail::is_iterator_impl<It, Tag>;

    template<class It>
    using is_input_iterator = is_iterator<It, std::input_iterator_tag>;

    template<class It>
    using is_output_iterator = is_iterator<It, std::output_iterator_tag>;

    template<class It>
    using is_forward_iterator = is_iterator<It, std::forward_iterator_tag>;

    template<class It>
    using is_bidirectional_iterator = is_iterator<It, std::bidirectional_iterator_tag>;

    template<class It>
    using is_random_access_iterator = is_iterator<It, std::random_access_iterator_tag>;
}

#endif