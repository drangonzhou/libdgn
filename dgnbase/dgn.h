// dgn.h : drangon library header
// Copyright (C) 2011 ~ 2023 drangon zhou <drangon.zhou (at) gmail.com>
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef INCLUDED_DGN_DGN_H
#define INCLUDED_DGN_DGN_H

// <1> common define, compatible with glibc/mingw/vs2019
// NULL
#ifndef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

// stdint
typedef signed char		int8_t;
typedef unsigned char	uint8_t;
typedef short			int16_t;
typedef unsigned short	uint16_t;
typedef int				int32_t;
typedef unsigned int	uint32_t;

#ifdef _WIN32

#ifdef __GNUC__
#define __int64 long long
#endif
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#ifdef _WIN64
typedef __int64 intptr_t;
typedef unsigned __int64 uintptr_t;
#else
typedef int intptr_t;
typedef unsigned int uintptr_t;
#endif

#else

// #include <stdint.h>
#if defined(__x86_64__) || defined(__powerpc64__)
typedef long int int64_t;
typedef unsigned long int uint64_t;
typedef long int intptr_t;
typedef unsigned long int uintptr_t;
#else
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
typedef int intptr_t;
typedef unsigned int uintptr_t;
#endif

#endif

// attribute : DEPRECATED ...
#ifdef _WIN32
#define DGN_DEPRECATED	__declspec(deprecated)
#define DGN_HIDDEN
#else
#define DGN_DEPRECATED	__attribute__ ((deprecated))
#define DGN_HIDDEN	__attribute__((visibility("hidden")))
#endif

// attribute : printf format check
#ifdef __GNUC__
#define DGN_ATTR_PRINTF(x,y) __attribute__((format (printf,x,y)))
#else
#define DGN_ATTR_PRINTF(x,y) 
#endif

// <2> dgn namespace and lib info

#define BEGIN_NS_DGN namespace dgn {
#define END_NS_DGN }

#define DGN_LIB_NAME	"libdgn"
#define DGN_LIB_DESC	"[libdgn] Drangon's simple C++ library"
#define DGN_LIB_VER_MAJOR	1
#define DGN_LIB_VER_MINOR	0
#define DGN_LIB_VER_PATCH	2
#define DGN_LIB_VER_FULL	( (DGN_LIB_VER_MAJOR << 16) + \
		(DGN_LIB_VER_MINOR << 8) + DGN_LIB_VER_PATCH )

#ifdef _WIN32
#ifdef DGN_LIB_EXPORTS
#define DGN_LIB_API __declspec(dllexport)
#else
#define DGN_LIB_API __declspec(dllimport)
#endif
#else
#define DGN_LIB_API
#endif

BEGIN_NS_DGN
////////////////

class Logger;

class DGN_LIB_API DgnLib
{
public:
	// default enable Logger output syslog
	static void Init( int argc, char * argv[] );
	static void Fini();
	
	static Logger * GetLogger();

	static const char * GetName();
	static const char * GetDesc();
	static int GetVersion( int * major = NULL, int * minor = NULL, int * patch = NULL );
};

////////////////
END_NS_DGN

namespace DrangonLibrary = dgn;

// NOTE : mininal depend
// #include <dgn/dgn.h>
// #include <dgn/Logger.h> // Logger
// #include <dgn/Thread.h> // Mutex
// #include <dgn/CStr.h>
// #include <dgn/Time.h>

#endif // INCLUDED_DGN_DGN_H

