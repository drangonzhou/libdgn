// CStr.h : String function
// Copyright (C) 2011 ~ 2019 drangon <drangon.zhou (at) gmail.com>
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

#ifndef INCLUDED_DGN_CSTR_H
#define INCLUDED_DGN_CSTR_H

#include <dgn/dgn.h>

BEGIN_NS_DGN
////////////////

enum {
	DGN_CSTR_FLAG_NORMAL = 0,
	DGN_CSTR_FLAG_EXTBUF,	// point to exist buffer, can not resize or release
	DGN_CSTR_FLAG_EXTCONST,	// point const string, can not modify or release
};

class CStr
{
public:
	CStr();
	// explicit CStr( const char * s );
	// copy a new CStr, use CStr().AttachConst( s ) and const & to avoid copy
	CStr( const char * s );
	CStr( const CStr & str );
	~CStr();

	CStr & operator = ( const char * s );
	CStr & operator = ( const CStr & str );

	// change mode, buffer or const must end with '\0'
	CStr & AttachBuffer( char * buf, int bufsize ); 
	CStr & AttachConst( const char * str, int len = -1 );
	CStr & AttachNone();

	static const CStr & EmptyCStrObj();

public:
	operator const char * () const { return m_str; }
	const char * Str() const { return m_str; } // always not NULL
	int Len() const { return m_len; }
	int Reserve( int cap ); // enlarge if need, not shrink if too much, may allocate more

	// access raw pointer, use as risk, 
	char * GetRaw() { return m_str; }
	int ReleaseRaw( int len );

public:
	int Assign( const CStr & str ) { return Assign( str.Str(), str.Len() ); }
	int Assign( const char * s, int len = -1 );
	int AssignFmt( const char * fmt, ... ) DGN_ATTR_PRINTF(2,3);

	int Append( const CStr & str ) { return Append( str.Str(), str.Len() ); }
	int Append( const char * s, int len = -1 );
	int AppendFmt( const char * fmt, ... ) DGN_ATTR_PRINTF(2,3);

	int Cmp( const CStr & str, int len = -1 ) const { return Cmp( str.Str(), len ); }
	int Cmp( const char * s, int len = -1 ) const;
	int CaseCmp( const CStr & str, int len = -1 ) const { return CaseCmp( str.Str(), len ); }
	int CaseCmp( const char * str, int len = -1 ) const;

	int ToUpper(); // all char 'a'-'z' -> 'A'-'Z'
	int ToLower(); // all char 'A'-'Z' -> 'a'-'z'

	int ToInt() const;
	int64_t ToInt64() const;
	double ToDouble() const;

public:
	CStr & operator += ( const CStr & str ) {
		Append( str );
		return *this;
	}

	friend CStr operator + ( const CStr & left, const CStr & right );

	friend bool operator == ( const CStr & left, const CStr & right );
	friend bool operator != ( const CStr & left, const CStr & right );
	friend bool operator < ( const CStr & left, const CStr & right );
	friend bool operator > ( const CStr & left, const CStr & right );

public:
	static int Cmp( const char * s1, const char * s2, int n = -1 );
	static int CaseCmp( const char * s1, const char * s2, int n = -1 );

	static int ToInt( const char * s );
	static int64_t ToInt64( const char * s );
	static double ToDouble( const char * s );

protected:
	char * m_str; // always not NULL, 
	int m_len;  // string len, must < cap unless == 0
	int m_cap;  // capacity
	int m_flag; // flags
};

inline bool operator == ( const CStr & left, const CStr & right )
{
	return left.Cmp( right ) == 0;
}

inline bool operator != ( const CStr & left, const CStr & right )
{
	return ! ( left == right );
}

inline bool operator < ( const CStr & left, const CStr & right )
{
	return left.Cmp( right ) < 0;
}

inline bool operator > ( const CStr & left, const CStr & right )
{
	return right < left;
}

CStr operator + ( const CStr & left, const CStr & right );

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_CSTR_H

