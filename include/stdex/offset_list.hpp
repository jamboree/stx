/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_OFFSET_LIST_HPP_INCLUDED
#define STDEX_OFFSET_LIST_HPP_INCLUDED

#include <memory>
#include <type_traits>
#include <initializer_list>
#include <boost/iterator/iterator_facade.hpp>
#include <stdex/is_iterator.hpp>

namespace stdex
{
    template<class T, class Allocator = std::allocator<T>>
    class offset_list;
}

namespace stdex { namespace offset_list_detail
{
    template<std::size_t Len, std::size_t Align>
    struct aligned_node
    {
        std::ptrdiff_t diff;
        std::aligned_storage_t<Len, Align> data;
    };

    template<class T>
    using node = aligned_node<sizeof(T), alignof(T)>;

    template<class T>
    inline T* advance_in_bytes(T* p, std::ptrdiff_t n)
    {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(p)+n);
    }

    inline std::ptrdiff_t distance_in_bytes(void* from, void* to)
    {
        return static_cast<char*>(to)-static_cast<char*>(from);
    }

    template<class T>
    struct iterator
      : boost::iterator_facade<iterator<T>, T, std::bidirectional_iterator_tag>
    {
        iterator() : here() {}

        iterator(iterator<std::remove_const_t<T>> const& other)
        : prev(other.prev), here(other.here)
        {}

    private:

        template<class U, class A>
        friend class offset_list;
        friend class iterator<T const>;
        friend class boost::iterator_core_access;

        using node_t = node<T>;

        iterator(node_t* prev, node_t* here)
          : prev(prev), here(here)
        {}

        bool equal(iterator const& other) const
        {
            return here == other.here;
        }

        T& dereference() const
        {
            return *reinterpret_cast<T*>(&here->data);
        }

        void increment()
        {
            node_t* next = advance_in_bytes(prev, here->diff);
            prev = here;
            here = next;
        }

        void decrement()
        {
            node_t* prior = advance_in_bytes(here, -prev->diff);
            here = prev;
            prev = prior;
        }

        node_t* prev;
        node_t* here;
    };

    struct root
    {
        std::ptrdiff_t _head;
        std::ptrdiff_t _tail;
    };

    template<class Allocator, class T>
    using node_alloc = typename std::allocator_traits<Allocator>::
        template rebind_alloc<node<T>>;
}}

namespace stdex
{
    template<class T, class Allocator>
    class alignas(offset_list_detail::node<T>) offset_list
      : offset_list_detail::root
      , offset_list_detail::node_alloc<Allocator, T>
    {
        using node_t = offset_list_detail::node<T>;
        using node_alloc = offset_list_detail::node_alloc<Allocator, T>;
        using node_alloc_traits = std::allocator_traits<node_alloc>;
        using alloc_traits = std::allocator_traits<Allocator>;

    public:

        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
        using iterator = offset_list_detail::iterator<T>;
        using const_iterator = offset_list_detail::iterator<T const>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        offset_list() noexcept(std::is_nothrow_default_constructible<Allocator>::value)
          : offset_list(Allocator())
        {}

        explicit offset_list(Allocator const& alloc) noexcept
          : root(), node_alloc(alloc)
        {}

        offset_list(size_type count, T const& val, Allocator const& alloc = Allocator())
          : offset_list(alloc)
        {
            while (count--)
                emplace_back(val);
        }

        offset_list(size_type count, Allocator const& alloc = Allocator())
          : offset_list(alloc)
        {
            while (count--)
                emplace_back();
        }

        template<class InputIt, std::enable_if_t<is_input_iterator<InputIt>::value, bool> = true>
        offset_list(InputIt first, InputIt last, Allocator const& alloc = Allocator())
          : offset_list(alloc)
        {
            for (; first != last; ++first)
                emplace_back(*first);
        }

        offset_list(offset_list const& other)
          : offset_list(other, std::allocator_traits<Allocator>::
                select_on_container_copy_construction(other.get_allocator()))
        {}

        offset_list(offset_list const& other, Allocator const& alloc)
          : offset_list(other.begin(), other.end(), alloc)
        {}

        offset_list(offset_list&& other) noexcept
          : offset_list(std::move(other), node_alloc(std::move(other.alloc_base())))
        {}

        offset_list(offset_list&& other, Allocator const& alloc) noexcept
          : node_alloc(alloc)
        {
            if (other._head)
                steal(other);
            else
            {
                _head = 0;
                _tail = 0;
            }
        }

        offset_list(std::initializer_list<T> init, Allocator const& alloc = Allocator())
          : offset_list(init.begin(), init.end(), alloc)
        {}

        ~offset_list()
        {
            destroy(begin(), end());
        }

        /// \exception-safety basic
        offset_list& operator=(offset_list const& other)
        {
            if (alloc_traits::propagate_on_container_copy_assignment::value)
            {
                if (alloc_base() != other.alloc_base())
                {
                    clear();
                    alloc_base() = other.alloc_base();
                }
            }
            assign(other.begin(), other.end());
            return *this;
        }

        offset_list& operator=(offset_list&& other) noexcept(
            alloc_traits::propagate_on_container_move_assignment::value)
        {
            if (!alloc_traits::propagate_on_container_move_assignment::value)
            {
                if (alloc_base() != other.alloc_base())
                {
                    assign
                    (
                        std::make_move_iterator(other.begin())
                      , std::make_move_iterator(other.end())
                    );
                    return *this;
                }
            }
            ~offset_list();
            return *new(this) offset_list(std::move(other), other.alloc_base());
        }

        /// \exception-safety basic
        offset_list& operator=(std::initializer_list<T> ilist)
        {
            assign(ilist);
            return *this;
        }

        /// \exception-safety basic
        void assign(size_type count, T const& val)
        {
            iterator i(begin()), e(end());
            for ( ; i != e; ++i, --count)
            {
                if (count)
                    *i = val;
                else
                {
                    erase(i, e);
                    return;
                }
            }
            if (count)
            {
                offset_list tmp(count, val, alloc_base());
                splice_impl(e, tmp, tmp.begin(), tmp.end());
            }
        }

        /// \exception-safety basic
        template<class InputIt, std::enable_if_t<is_input_iterator<InputIt>::value, bool> = true>
        void assign(InputIt first, InputIt last)
        {
            iterator i(begin()), e(end());
            for ( ; i != e; ++i, ++first)
            {
                if (first != last)
                    *i = *first;
                else
                {
                    erase(i, e);
                    return;
                }
            }
            if (first != last)
            {
                offset_list tmp(first, last, alloc_base());
                splice_impl(e, tmp, tmp.begin(), tmp.end());
            }
        }

        /// \exception-safety basic
        void assign(std::initializer_list<T> ilist)
        {
            assign(ilist.begin(), ilist.end());
        }

        allocator_type get_allocator() const noexcept
        {
            return static_cast<allocator_type const&>(*this);
        }

        reference front() noexcept
        {
            return *reinterpret_cast<T*>(&node(_head)->data);
        }

        const_reference front() const noexcept
        {
            return *reinterpret_cast<T const*>(&node(_head)->data);
        }

        reference back() noexcept
        {
            return *reinterpret_cast<T*>(&node(_tail)->data);
        }

        const_reference back() const noexcept
        {
            return *reinterpret_cast<T const*>(&node(_tail)->data);
        }

        iterator begin() noexcept
        {
            return iterator(reinterpret_cast<node_t*>(this), node(_head));
        }

        const_iterator begin() const noexcept
        {
            return const_cast<offset_list*>(this)->begin();
        }

        const_iterator cbegin() const noexcept
        {
            return const_cast<offset_list*>(this)->begin();
        }

        iterator end() noexcept
        {
            return iterator(node(_tail), reinterpret_cast<node_t*>(this));
        }

        const_iterator end() const noexcept
        {
            return const_cast<offset_list*>(this)->end();
        }

        const_iterator cend() const noexcept
        {
            return const_cast<offset_list*>(this)->end();
        }

        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }

        reverse_iterator rend() noexcept
        {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        bool empty() const noexcept
        {
            return !_head;
        }

        size_type size() const noexcept
        {
            return std::distance(begin(), end());
        }

        size_type max_size() const noexcept
        {
            return alloc_traits::max_size(get_allocator());
        }

        void clear() noexcept
        {
            destroy(begin(), end());
            _head = 0;
            _tail = 0;
        }

        /// \exception-safety strong
        iterator insert(const_iterator pos, T const& val)
        {
            return emplace(pos, val);
        }

        /// \exception-safety strong
        iterator insert(const_iterator pos, T&& val)
        {
            return emplace(pos, std::forward<T>(val));
        }

        /// \exception-safety strong
        iterator insert(const_iterator pos, size_type count, T const& val)
        {
            return count? insert_multi(pos, count, val) : pos;
        }

        /// \exception-safety strong
        template<class InputIt, std::enable_if_t<is_input_iterator<InputIt>::value, bool> = true>
        iterator insert(const_iterator pos, InputIt first, InputIt last)
        {
            return first != last? insert_multi(pos, first, last) : pos;
        }

        /// \exception-safety strong
        iterator insert(const_iterator pos, std::initializer_list<T> ilist)
        {
            return insert(pos, ilist.begin(), ilist.end());
        }

        /// \exception-safety strong
        template<class... Args>
        iterator emplace(const_iterator pos, Args&&... args)
        {
            using namespace offset_list_detail;
            node_t* p = new_node(distance_in_bytes(pos.prev, pos.here), std::forward<Args>(args)...);
            pos.prev->diff += distance_in_bytes(pos.here, p);
            if (pos.here != reinterpret_cast<node_t*>(this))
                pos.here->diff += distance_in_bytes(p, pos.prev);
            else
                _tail = distance_in_bytes(this, p);
            return iterator(pos.prev, p);
        }

        iterator erase(const_iterator pos) noexcept
        {
            using namespace offset_list_detail;
            std::ptrdiff_t diff = pos.here->diff;
            delete_node(pos.here);
            node_t* next = advance_in_bytes(pos.prev, diff);
            pos.prev->diff += distance_in_bytes(pos.here, next);
            if (next != reinterpret_cast<node_t*>(this))
                next->diff += distance_in_bytes(pos.prev, pos.here);
            else
                _tail = -diff;
            return iterator(pos.prev, next);
        }

        iterator erase(const_iterator first, const_iterator last) noexcept
        {
            using namespace offset_list_detail;
            destroy(first, last);
            node_t* next = last.here;
            first.prev->diff += distance_in_bytes(first.here, next);
            if (next != reinterpret_cast<node_t*>(this))
                next->diff += distance_in_bytes(first.prev, last.prev);
            else
                _tail = distance_in_bytes(this, first.prev);
            return iterator(first.prev, next);
        }

        /// \exception-safety strong
        void push_back(T const& val)
        {
            emplace_back(val);
        }

        /// \exception-safety strong
        void push_back(T&& val)
        {
            emplace_back(std::move(val));
        }

        /// \exception-safety strong
        template<class... Args>
        void emplace_back(Args&&... args)
        {
            using namespace offset_list_detail;
            node_t* p = new_node(-_tail, std::forward<Args>(args)...);
            std::ptrdiff_t next_tail = distance_in_bytes(this, p);
            node(_tail)->diff += next_tail;
            _tail = next_tail;
        }

        void pop_back() noexcept
        {
            using namespace offset_list_detail;
            node_t* last = node(_tail);
            std::ptrdiff_t next_tail = -last->diff;
            delete_node(last);
            node(next_tail)->diff += distance_in_bytes(last, this);
            _tail = next_tail;
        }

        /// \exception-safety strong
        void push_front(T const& val)
        {
            emplace_front(val);
        }

        /// \exception-safety strong
        void push_front(T&& val)
        {
            emplace_front(std::move(val));
        }

        /// \exception-safety strong
        template<class... Args>
        void emplace_front(Args&&... args)
        {
            (void)emplace(begin(), std::forward<Args>(args)...);
        }

        void pop_front() noexcept
        {
            (void)erase(begin());
        }

        /// \exception-safety strong
        void resize(size_type count)
        {
            resize_impl(count);
        }

        /// \exception-safety strong
        void resize(size_type count, T const& val)
        {
            resize_impl(count, val);
        }

        void swap(offset_list& other) noexcept
        {
            using namespace offset_list_detail;
            if (alloc_traits::propagate_on_container_swap::value)
            {
                using std::swap;
                swap(alloc_base(), other.alloc_base());
            }
            if (other._head)
            {
                if (_head)
                {
                    std::ptrdiff_t offset = distance_in_bytes(&other, this);
                    node_t* here = node(_head);
                    node_t* there = other.node(other._head);
                    here->diff += offset;
                    there->diff -= offset;
                    _head = distance_in_bytes(this, there);
                    other._head = distance_in_bytes(&other, here);
                    here = node(_tail);
                    there = other.node(other._tail);
                    here->diff -= offset;
                    there->diff += offset;
                    _tail = distance_in_bytes(this, there);
                    other._tail = distance_in_bytes(&other, here);
                }
                else
                    steal(other);
            }
            else if (_head)
                other.steal(*this);
        }

        void splice(const_iterator pos, offset_list& other)
        {
            splice(pos, other, other.begin(), other.end());
        }

        void splice(const_iterator pos, offset_list&& other)
        {
            splice(pos, other, other.begin(), other.end());
        }

        void splice(const_iterator pos, offset_list& other, const_iterator it)
        {
            const_iterator next(it);
            splice_impl(pos, other, it, ++next);
        }

        void splice(const_iterator pos, offset_list& other, const_iterator first, const_iterator last)
        {
            if (first != last)
                splice_impl(pos, other, first, last);
        }

        void remove(T const& val)
        {
            remove_if([&val](T const& ref)
            {
                return ref == val;
            });
        }

        template<class UnaryPredicate>
        void remove_if(UnaryPredicate pred)
        {
            const_iterator i(begin()), e(end());
            while (i != e)
            {
                if (pred(*i))
                    i = erase(i);
                else
                    ++i;
            }
        }

        void reverse() noexcept
        {
            const_iterator i(begin()), e(end());
            if (i != e)
            {
                do
                {
                    std::ptrdiff_t& diff = i.here->diff;
                    ++i;
                    diff = -diff;
                } while (i != e);
                std::swap(_head, _tail);
            }
        }

        void unique()
        {
            unique(std::equal_to<>());
        }

        template<class BinaryPredicate>
        void unique(BinaryPredicate pred)
        {
            T const* prev;
            const_iterator i(begin()), e(end());
            if (i != e)
            {
                prev = &i.here->data;
                ++i;
                while (i != e)
                {
                    if (pred(*prev, *i))
                        i = erase(i);
                    else
                    {
                        prev = &i.here->data;
                        ++i;
                    }
                }
            }
        }

    private:

        node_t* node(std::ptrdiff_t offset)
        {
            using namespace offset_list_detail;
            return static_cast<node_t*>(advance_in_bytes<void>(this, offset));
        }

        node_t const* node(std::ptrdiff_t offset) const
        {
            using namespace offset_list_detail;
            return static_cast<node_t const*>(advance_in_bytes<void const>(this, offset));
        }

        node_alloc& alloc_base()
        {
            return static_cast<node_alloc&>(*this);
        }

        template<class... Ts>
        node_t* new_node(std::ptrdiff_t diff, Ts&&... ts)
        {
            node_t* p = node_alloc_traits::allocate(alloc_base(), 1);
            try
            {
                node_alloc_traits::construct
                  (
                    alloc_base()
                    , reinterpret_cast<T*>(&p->data)
                    , std::forward<Ts>(ts)...
                  );
            }
            catch(...)
            {
                node_alloc_traits::deallocate(alloc_base(), p, 1);
                throw;
            }
            p->diff = diff;
            return p;
        }

        void delete_node(node_t* p)
        {
            node_alloc_traits::destroy(alloc_base(), reinterpret_cast<T*>(&p->data));
            node_alloc_traits::deallocate(alloc_base(), p, 1);
        }

        void steal(offset_list& other)
        {
            using namespace offset_list_detail;
            std::ptrdiff_t offset = distance_in_bytes(&other, this);
            node_t* there = other.node(other._head);
            there->diff -= offset;
            _head = distance_in_bytes(this, there);
            there = other.node(other._tail);
            there->diff += offset;
            _tail = distance_in_bytes(this, there);
            other._head = 0;
            other._tail = 0;
        }

        void destroy(const_iterator first, const_iterator last)
        {
            const_iterator pos(first);
            while (pos != last)
            {
                node_t* here = pos.here;
                ++pos;
                delete_node(here);
            }
        }

        void splice_impl(const_iterator pos, offset_list& other, const_iterator first, const_iterator last)
        {
            using namespace offset_list_detail;
            pos.prev->diff += distance_in_bytes(pos.here, first.here);
            first.prev->diff += distance_in_bytes(first.here, last.here);
            first.here->diff += distance_in_bytes(pos.prev, first.prev);
            last.prev->diff += distance_in_bytes(last.here, pos.here);
            if (last.here != reinterpret_cast<node_t*>(&other))
                last.here->diff += distance_in_bytes(first.prev, last.prev);
            else
                other._tail = distance_in_bytes(&other, first.prev);
            if (pos.here != reinterpret_cast<node_t*>(this))
                pos.here->diff += distance_in_bytes(last.prev, pos.prev);
            else
                _tail = distance_in_bytes(this, last.prev);
        }

        template<class... Ts>
        iterator insert_multi(const_iterator& pos, Ts const&... ts)
        {
            offset_list tmp(ts..., alloc_base());
            iterator it(tmp.begin());
            splice_impl(pos, tmp, it, tmp.end());
            return iterator(pos.prev, it.here);
        }

        template<class... Ts>
        void resize_impl(size_type count, Ts const&... ts)
        {
            iterator i(begin()), e(end());
            for ( ; i != e; ++i, --count)
            {
                if (!count)
                {
                    erase(i, e);
                    return;
                }
            }
            if (count)
            {
                offset_list tmp(count, ts..., alloc_base());
                splice_impl(e, tmp, tmp.begin(), tmp.end());
            }
        }
    };

    template<class T, class Alloc>
    inline bool operator==(offset_list<T, Alloc> const& lhs, offset_list<T, Alloc> const& rhs)
    {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<class T, class Alloc>
    inline bool operator!=(offset_list<T, Alloc> const& lhs, offset_list<T, Alloc> const& rhs)
    {
        return !(rhs == lhs);
    }

    template<class T, class Alloc>
    inline bool operator<(offset_list<T, Alloc> const& lhs, offset_list<T, Alloc> const& rhs)
    {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<class T, class Alloc>
    inline bool operator<=(offset_list<T, Alloc> const& lhs, offset_list<T, Alloc> const& rhs)
    {
        return !(rhs < lhs);
    }

    template<class T, class Alloc>
    inline bool operator>(offset_list<T, Alloc> const& lhs, offset_list<T, Alloc> const& rhs)
    {
        return rhs < lhs;
    }

    template<class T, class Alloc>
    inline bool operator>=(offset_list<T, Alloc> const& lhs, offset_list<T, Alloc> const& rhs)
    {
        return !(lhs < rhs);
    }

    template<class T, class Alloc>
    inline void swap(offset_list<T, Alloc> const& lhs, offset_list<T, Alloc> const& rhs)
    {
        lhs.swap(rhs);
    }
}

#endif