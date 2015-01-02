/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_FUNCTION_REF_HPP_INCLUDED
#define STDEX_FUNCTION_REF_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

namespace stdex
{
    template<class Sig>
    class function_ref;

    template<class R, class... Ts>
    class function_ref<R(Ts...)>
    {
        template<class F>
        using is_callable = std::is_convertible<std::result_of_t<F(Ts...)>, R>;

        template<class T>
        using enable_other_callable =
            std::enable_if_t<!std::is_same<std::decay_t<T>, function_ref>::value
              && is_callable<T>::value, bool>;

        template<class F>
        static R invoke(std::uintptr_t p, Ts&&... ts)
        {
            return (*reinterpret_cast<F*>(p))(std::forward<Ts>(ts)...);
        }

        std::uintptr_t _p;
        R(*_f)(std::uintptr_t, Ts&&...);

    public:

        template<class F, enable_other_callable<F> = true>
        function_ref(F&& f) noexcept
          : _p(reinterpret_cast<std::uintptr_t>(&f))
          , _f(invoke<std::remove_reference_t<F>>)
        {}

        template<class F, std::enable_if_t<is_callable<F>::value, bool> = true>
        function_ref(F* f) noexcept
          : _p(reinterpret_cast<std::uintptr_t>(f)), _f(invoke<F>)
        {}

        R operator()(Ts... ts) const
        {
            return _f(_p, std::forward<Ts>(ts)...);
        }
    };
}

#endif