// Atomic.h : drangon library, Atomic Operation
// Copyright (C) 2011 ~ 2019 drangon zhou <drangon.zhou (at) gmail.com>
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

#ifndef INCLUDED_DGN_ATOMIC_H
#define INCLUDED_DGN_ATOMIC_H

#include <dgn/dgn.h>

#ifdef _WIN32
#include <windows.h>
#endif

BEGIN_NS_DGN
////////////////

class Atomic
{
public:
	Atomic( int val = 0 ) : m_val( val ) {}
	~Atomic() {}

	// return new value
	inline int Get() const { return m_val; }
	inline int Set( int val );
	inline int Add( int val );
	inline int Sub( int val );
	inline int Inc();
	inline int Dec();

protected:
	volatile int m_val;
};

inline int Atomic::Set( int val )
{
#ifdef _WIN32
	InterlockedExchange( (LONG *)&m_val, val );
#else
	m_val = val;
#endif
	return val;
}

inline int Atomic::Add( int val )
{
#ifdef _WIN32
	return InterlockedExchangeAdd( (LONG *)&m_val, val ) + val;
#else
	// TODO : x86 only, other arch need imp
	int tmp = val;
	asm volatile ("lock; xaddl %0,%1"
		: "=r" (tmp), "=m" (m_val)
		: "0" (tmp), "m" (m_val)
		: "memory", "cc");
	return tmp + val;
#endif
}

inline int Atomic::Sub( int val )
{
	return Add( - val );
}

inline int Atomic::Inc()
{
#ifdef _WIN32
	return InterlockedIncrement( (LONG *)&m_val );
#else
	return Add( 1 );
#endif
}

inline int Atomic::Dec()
{
#ifdef _WIN32
	return InterlockedDecrement( (LONG *)&m_val );
#else
	return Sub( 1 );
#endif
}

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_ATOMIC_H

