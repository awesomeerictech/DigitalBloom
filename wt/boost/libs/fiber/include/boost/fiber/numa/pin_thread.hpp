
//          Copyright Oliver Kowalke 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_NUMA_PIN_THREAD_H
#define BOOST_FIBERS_NUMA_PIN_THREAD_H

#include <cstdint>
#include <thread>

#include <boost/config.hpp>

#include <boost/fiber/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

/*

extern "C" {
    // Ensure Windows API level is at least Windows 7 / Server 2008 R2
    // without permanently changing a macro that may be set by other files.
#   if defined(_WIN32_WINNT)
#     if _WIN32_WINNT < 0x0601
#       pragma push_macro("_WIN32_WINNT")
#       undef _WIN32_WINNT
#       define _WIN32_WINNT 0x0601
#       define BOOST_FIBERS_TEMP_WIN32_WINNT_PUSHED
#     endif
#   else
#     define _WIN32_WINNT 0x0601
#     define BOOST_FIBERS_TEMP_WIN32_WINNT_DEFINED
#   endif

#   include <windows.h>

#   if defined(BOOST_FIBERS_TEMP_WIN32_WINNT_PUSHED)
#     pragma pop_macro("_WIN32_WINNT")
#     undef BOOST_FIBERS_TEMP_WIN32_WINNT_PUSHED
#   elif defined(BOOST_FIBERS_TEMP_WIN32_WINNT_DEFINED)
#     undef _WIN32_WINNT
#     undef BOOST_FIBERS_TEMP_WIN32_WINNT_DEFINED
#   endif
}

*/

namespace boost {
namespace fibers {
namespace numa {

BOOST_FIBERS_DECL
void pin_thread( std::uint32_t, std::thread::native_handle_type);

BOOST_FIBERS_DECL
void pin_thread( std::uint32_t cpuid);

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_NUMA_PIN_THREAD_H
