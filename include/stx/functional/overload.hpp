/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_FUNCTIONAL_OVERLOAD_HPP_INCLUDED
#define STX_FUNCTIONAL_OVERLOAD_HPP_INCLUDED

#include <type_traits>

namespace stx { namespace overload_detail
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

        template<class... F2>
        composite(F const& f, F2&&... fs)
          : F(f)
          , composite<Fs...>(std::forward<F2>(fs)...)
        {}

        template<class... F2>
        composite(F&& f, F2&&... fs)
          : F(std::move(f))
          , composite<Fs...>(std::forward<F2>(fs)...)
        {}
    };

    template<class F, class... Fs>
    struct composite<F*, Fs...> : private composite<Fs...>
    {
        operator F*() const
        {
            return _f;
        }

        template<class... F2>
        composite(F* f, F2&&... fs)
          : _f(f)
          , composite<Fs...>(std::forward<F2>(fs)...)
        {}

    private:

        F* _f;
    };
}}

namespace stx
{
    template<class... F>
    inline overload_detail::composite<std::remove_reference_t<F>...>
    overload(F&&... f)
    {
        return {std::forward<F>(f)...};
    }
}

#endif