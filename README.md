stdex
=====

lib for c++14 and forward

## Dependencies

- [Boost](http://www.boost.org/)

## Components

- __offset_list__: STL-like relocatable [subtraction linked list](http://en.wikipedia.org/wiki/XOR_linked_list#Subtraction_linked_list)
- __function_ref__: non-allocating synchronous function callback
- __enable_if_valid__: SFINAE utility
- __is_iterator__: iterator traits
- __priority__: priority-based tag-dispatching utility
- __reconstruct__: object reconstruction utility
- __find__: `find` and `contains` algorithms
- __binary_search__: binary_search that returns an iterator
- __overload__: overload callable objects
- __task__: awaitable task based on [N4286](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4286.pdf)
- __act__: [ASIO](http://www.boost.org/doc/libs/release/doc/html/boost_asio.html) Cooperative Task

## License

    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
