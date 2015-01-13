/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_OVERLOAD_HPP_INCLUDED
#define STDEX_OVERLOAD_HPP_INCLUDED

#include <type_traits>

namespace stdex { namespace overload_detail
{
    template<class... F>
    struct composite
    {
        template<class NeverMatch>
        void operator()();
    };

    template<class F, class... Fs>
    struct composite<F, Fs...> : private F, private composite<Fs...>
    {
        using F::operator();
        using composite<Fs...>::operator();

        composite(F&& f, Fs&&... fs)
          : F(std::move(f))
          , composite<Fs...>(std::move(fs)...)
        {}
    };

    template<class F, class... Fs>
    struct composite<F*, Fs...> : private composite<Fs...>
    {
        operator F*() const
        {
            return _f;
        }

        composite(F* f, Fs&&... fs)
          : _f(f)
          , composite<Fs...>(std::move(fs)...)
        {}

    private:

        F* _f;
    };
}}

namespace stdex
{
    template<class... F>
    inline overload_detail::composite<std::remove_reference_t<F>...>
    overload(F&&... f)
    {
        return {std::forward<F>(f)...};
    }
}

#endif
