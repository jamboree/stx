/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_UTILITY_FLAG_SET_HPP_INCLUDED
#define STX_UTILITY_FLAG_SET_HPP_INCLUDED

#include <type_traits>
#include <boost/config.hpp>

namespace stx
{
    template<class Enum>
    struct flag_set
    {
        using value_type = std::underlying_type_t<Enum>;

        constexpr flag_set() : _flags() {}

        constexpr flag_set(Enum val) : _flags(static_cast<value_type>(val)) {}

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

#endif