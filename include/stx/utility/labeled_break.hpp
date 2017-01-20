/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_UTILITY_LABELED_BREAK_HPP_INCLUDED
#define STX_UTILITY_LABELED_BREAK_HPP_INCLUDED

#define STX_LABEL(name)                                                         \
if (constexpr bool stx_label_##name = false){STX_BREAK(name); stx_break_##name:;} else

#define STX_BREAK(name)                                                         \
do {goto stx_break_##name;} while (stx_label_##name)

#endif