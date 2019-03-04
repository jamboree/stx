/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_UTILITY_ARRAY_VIEW_HPP_INCLUDED
#define STX_UTILITY_ARRAY_VIEW_HPP_INCLUDED

#include <cstddef>
#include <type_traits>

namespace stx
{
    namespace detail
    {
        template<class V, class T>
        auto is_array_of_test(T&& t) -> std::enable_if_t<
            std::is_convertible_v<decltype(t.data()), V*> &&
            std::is_convertible_v<decltype(t.size()), std::size_t>,
            std::true_type>;

        template<class V>
        std::false_type is_array_of_test(...);

        template<class V, class T>
        struct is_array_of : decltype(is_array_of_test<V>(std::declval<T>())) {};
    }

    template<class T>
    struct array_view
    {
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using iterator = pointer;
        using const_iterator = const_pointer;

        constexpr array_view() : _data(), _n() {}

        constexpr array_view(T* data, std::size_t size) : _data(data), _n(size) {}

        template<std::size_t N>
        constexpr array_view(T(&array)[N]) : _data(array), _n(N) {}

        template<class Array, std::enable_if_t<detail::is_array_of<T, Array>::value, bool> = true>
        constexpr array_view(Array&& array) : _data(array.data()), _n(array.size()) {}

        constexpr reference operator[](size_type pos)
        {
            return _data[pos];
        }

        constexpr const_reference operator[](size_type pos) const
        {
            return _data[pos];
        }

        constexpr reference front()
        {
            return *_data;
        }

        constexpr const_reference front() const
        {
            return *_data;
        }

        constexpr reference back()
        {
            return _data[_n - 1];
        }

        constexpr const_reference back() const
        {
            return _data[_n - 1];
        }

        constexpr iterator begin()
        {
            return _data;
        }

        constexpr const_iterator begin() const
        {
            return _data;
        }

        constexpr iterator end()
        {
            return _data + _n;
        }

        constexpr const_iterator end() const
        {
            return _data + _n;
        }

        constexpr bool empty() const
        {
            return !_n;
        }

        constexpr size_type size() const
        {
            return _n;
        }

        constexpr void pop_front()
        {
            ++_data;
            --_n;
        }

        constexpr void pop_back()
        {
            --_n;
        }

    private:
        T* _data;
        std::size_t _n;
    };

    template<class T>
    constexpr array_view<T> make_array_view(T* data, std::size_t size)
    {
        return {data, size};
    }
}

#endif