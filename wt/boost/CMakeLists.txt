# Copyright 2019, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.8...3.16)

#[[ The default build type must be set before project()
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()]]

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force my bool variable to OFF" FORCE)
set(BOOST_ENABLE_MPI OFF CACHE BOOL "Force my bool variable to OFF" FORCE)
set(BOOST_ENABLE_PYTHON OFF CACHE BOOL "Force my bool variable to OFF" FORCE)
#[[set(BOOST_INCLUDE_LIBRARIES "asio;chrono;date_time;system;program_options;thread;filesystem;test;any;regex;serialization" CACHE STRING "Internal variable to control optimized builds (Include these only)" FORCE)]]

set(BOOST_INCLUDE_LIBRARIES
"accumulators;algorithm;align;any;array;asio;assert;assign;atomic;beast;bimap;bind;bloom;callable_traits;charconv;chrono;circular_buffer;cobalt;compat;compute;concept_check;config;container;container_hash;context;contract;conversion;convert;core;coroutine;coroutine2;crc;date_time;describe;detail;dll;dynamic_bitset;endian;exception;fiber;filesystem;flyweight;foreach;format;function;function_types;functional;fusion;geometry;gil;graph;hana;hash2;headers;heap;histogram;hof;icl;integer;interprocess;intrusive;io;iostreams;iterator;json;lambda;lambda2;leaf;lexical_cast;locale;lockfree;log;logic;math;metaparse;move;mp11;mpl;mqtt5;msm;multi_array;multi_index;multiprecision;nowide;numeric;openmethod;optional;outcome;parameter;parser;pfr;phoenix;poly_collection;polygon;pool;predef;preprocessor;process;program_options;property_map;property_tree;proto;ptr_container;qvm;random;range;ratio;rational;redis;regex;safe_numerics;scope;scope_exit;serialization;signals2;smart_ptr;sort;spirit;stacktrace;statechart;static_assert;static_string;stl_interfaces;system;test;thread;throw_exception;timer;tokenizer;tti;tuple;type_erasure;type_index;type_traits;typeof;units;unordered;url;utility;uuid;variant;variant2;vmd;wave;winapi;xpressive;yap" CACHE STRING "Internal variable to control optimized builds (Include these only)" FORCE)


#[[set(BOOST_EXCLUDE_LIBRARIES "accumulators;algorithm;align;any;array;asio;assert;assign;atomic;beast;bimap;bind;bloom;callable_traits;charconv;chrono;circular_buffer;cobalt;compat;compute;concept_check;config;container;container_hash;context;contract;conversion;convert;core;coroutine;coroutine2;crc;date_time;describe;detail;dll;dynamic_bitset;endian;exception;fiber;filesystem;flyweight;foreach;format;function;function_types;functional;fusion;geometry;gil;graph;graph_parallel;hana;hash2;headers;heap;histogram;hof;icl;integer;interprocess;intrusive;io;iostreams;iterator;json;lambda;lambda2;leaf;lexical_cast;lexical_cast;locale;lockfree;log;logic;math;metaparse;move;mp11;mpi;mpl;mqtt5;msm;multi_array;multi_index;multiprecision;mysql;nowide;numeric;openmethod;optional;outcome;parameter;parameter;parser;pfr;phoenix;poly_collection;polygon;pool;predef;preprocessor;process;program_options;property_map;property_map_parallel;property_tree;proto;ptr_container;python;qvm;random;range;ratio;rational;redis;regex;safe_numerics;scope;scope_exit;serialization;signals2;smart_ptr;sort;spirit;stacktrace;statechart;static_assert;static_string;stl_interfaces;system;test;thread;throw_exception;timer;tokenizer;tti;tuple;type_erasure;type_index;type_traits;typeof;units;unordered;url;utility;uuid;variant;variant2;vmd;wave;winapi;xpressive;yap" CACHE STRING "Internal variable to control optimized builds (Exclude these only)" FORCE)]]

#[[set(BOOST_EXCLUDE_LIBRARIES
"atomic;chrono;context;contract;coroutine;coroutine2;date_time;exception;fiber;filesystem;graph_parallel;iostreams;locale;log;math;mpi;program_options;python;random;regex;serialization;stacktrace;system;test;thread;timer;type_erasure;wave" CACHE STRING "Internal variable to control optimized builds (Exclude these only)" FORCE)]]
# b2 toolset=clang target-os=android architecture=arm address-model=64

set(BOOST_EXCLUDE_LIBRARIES "python;mpi;graph_parallel;mysql" CACHE STRING "Internal variable to control optimized builds (Exclude these only)" FORCE)


set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Internal variable to control optimized builds (Exclude these only)" FORCE)

#set(MaterialDesignIcons "${CMAKE_BINARY_DIR}/eric/materialdesignicons-src" CACHE INTERNAL "MaterialDesignIcons" )
#set(EXTERNAL_LIB_ROOT_DIR "${CMAKE_SOURCE_DIR}/external/libs" CACHE PATH "Root directory where external libraries can be found" FORCE)
# set(INTERNAL_BUILD_MODE "Release" CACHE STRING "Internal variable to control optimized builds (Release/Debug)" FORCE)

project(Boost VERSION 1.90.0 LANGUAGES CXX)

set(BOOST_SUPERPROJECT_VERSION ${PROJECT_VERSION})
set(BOOST_SUPERPROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tools/cmake/include)

IF(UNIX OR ANDROID OR APPLE)
	add_subdirectory(${CMAKE_SOURCE_DIR}/wt/boost/libarchive)
ELSE()
   add_subdirectory(${CMAKE_SOURCE_DIR}/wt/boost/libarchive)
ENDIF()

include(BoostRoot)

