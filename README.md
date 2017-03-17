stx
=====

Arbitrary collections of STL-like components.

## Dependencies

- [Boost](http://www.boost.org/)

## Components

### algorithm
- `binary_search` - binary_search that returns an iterator.
- `apply_permutation` - reoder the elements by the specified indices.
- `unstable_remove` - faster `remove` that does not regard the order.

### container
- `offset_list` - relocatable [subtraction linked list](http://en.wikipedia.org/wiki/XOR_linked_list#Subtraction_linked_list).

### functional
- `function_ref` - non-allocating synchronous function callback.
- `overload` - overload callable objects.

### sync
- `event` -  a synchronization primitive that can be used to block the thread until the event is set.
- `spinlock` -  a busy waiting mutex.

### traits
- `find` - find an element in a container.
- `contains` - test whether an element is in the container.

### type_traits
- `enable_if_valid` - SFINAE on expression.
- `is_iterator` - iterator traits.
- `is_callable` - check if callable (with an optional return type).

### utility
- `priority` - priority-based tag-dispatching.
- `reconstruct` - object reconstruction.
- `flag_set` - a type-safe flag-set

## License

    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
