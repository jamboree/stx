/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_FIND_HPP_INCLUDED
#define STDEX_FIND_HPP_INCLUDED

#include <algorithm>

namespace stdex { namespace detail
{
    template<class Con, class T>
    inline auto find_impl(priority<1>, Con& con, T const& val) ->
        decltype(con.find(val))
    {
        return con.find(val);
    }

    template<class Con, class T>
    inline auto find_impl(priority<0>, Con& con, T const& val)
    {
        using std::begin;
        using std::end;
        return std::find(begin(con), end(con), val);
    }
}}

namespace stdex
{
    template<class Con, class T>
    inline auto find(Con&& con, T const& val)
    {
        return detail::find_impl(priority<1>{}, con, val);
    }

    template<class Con, class T>
    inline bool contains(Con const& con, T const& val)
    {
        using std::end;
        return find(con, val) != end(con);
    }
}

#endif