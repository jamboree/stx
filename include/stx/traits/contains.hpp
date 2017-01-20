/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_TRAITS_CONTAINS_HPP_INCLUDED
#define STX_TRAITS_CONTAINS_HPP_INCLUDED

#include <stx/traits/find.hpp>

namespace stx { namespace traits_detail
{
    template<class Container, class T>
    inline auto contains(Container&& con, T const& val) ->
        decltype(find(std::forward<Container>(con), val) != end(std::forward<Container>(con)))
    {
        return find(std::forward<Container>(con), val) != end(std::forward<Container>(con));
    }

    struct contains_fn
    {
        template<class Container, class T>
        auto operator()(Container&& con, T const& val) const ->
            decltype(contains(std::forward<Container>(con), val))
        {
            return contains(std::forward<Container>(con), val);
        }
    };
}}

namespace stx { namespace traits
{
    constexpr traits_detail::contains_fn contains{};
}}

#endif