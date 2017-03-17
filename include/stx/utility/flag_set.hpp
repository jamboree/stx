/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_UTILITY_FLAG_SET_HPP_INCLUDED
#define STX_UTILITY_FLAG_SET_HPP_INCLUDED

#include <type_traits>
#include <boost/config.hpp>
#include <stx/type_traits/enable_if_valid.hpp>

namespace stx
{
    namespace detail
    {
        template<class Enum, std::enable_if_t<std::is_enum<Enum>::value, bool> = true>
        auto get_flag_test(Enum val) -> enable_if_valid_t<decltype(get_flag(val)), std::true_type>;
        std::false_type get_flag_test(...);
    }

    template<class Enum>
    struct is_flag : decltype(detail::get_flag_test(std::declval<Enum>())) {};

    template<class Enum>
    struct flag_set
    {
        using value_type = std::underlying_type_t<Enum>;

        constexpr flag_set() : _flags() {}

        constexpr flag_set(Enum val) : _flags(get_flag(val)) {}

        explicit constexpr flag_set(value_type val) : _flags(val) {}

        BOOST_CXX14_CONSTEXPR flag_set& operator|=(flag_set other)
        {
            _flags |= other._flags;
            return *this;
        }

        BOOST_CXX14_CONSTEXPR flag_set& operator-=(flag_set other)
        {
            _flags &= ~other._flags;
            return *this;
        }

        BOOST_CXX14_CONSTEXPR flag_set& operator&=(flag_set other)
        {
            _flags &= other._flags;
            return *this;
        }

        BOOST_CXX14_CONSTEXPR flag_set& operator^=(flag_set other)
        {
            _flags ^= other._flags;
            return *this;
        }

        BOOST_CXX14_CONSTEXPR flag_set operator-(flag_set other) const
        {
            return flag_set(*this) -= other;
        }

        explicit constexpr operator bool() const
        {
            return !!_flags;
        }

        constexpr bool operator==(flag_set other) const
        {
            return _flags == other._flags;
        }

        constexpr bool operator!=(flag_set other) const
        {
            return _flags != other._flags;
        }

        friend constexpr flag_set operator|(flag_set lhs, flag_set rhs)
        {
            return flag_set(lhs._flags | rhs._flags);
        }

        friend constexpr flag_set operator&(flag_set lhs, flag_set rhs)
        {
            return flag_set(lhs._flags & rhs._flags);
        }

        friend constexpr flag_set operator^(flag_set lhs, flag_set rhs)
        {
            return flag_set(lhs._flags ^ rhs._flags);
        }

    private:
        value_type _flags;
    };
}

template<class Enum, std::enable_if_t<stx::is_flag<Enum>::value, bool> = true>
constexpr stx::flag_set<Enum> operator|(Enum lhs, Enum rhs)
{
    return stx::flag_set<Enum>(lhs) | rhs;
}

#endif