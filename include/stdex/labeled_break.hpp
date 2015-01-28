/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_LABELED_BREAK_HPP_INCLUDED
#define STDEX_LABELED_BREAK_HPP_INCLUDED


#define $(name)                                                                 \
if (constexpr bool $##name = false){break$(name); break$##name:;} else

#define break$(name)                                                            \
do {goto break$##name;} while ($##name)


#endif
