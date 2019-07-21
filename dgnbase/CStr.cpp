// CStr.cpp : String function
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

#include <dgn/CStr.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h> // vsnprintf
#include <stdlib.h>

BEGIN_NS_DGN
////////////////

static const CStr s_empty_cstr_obj;

const CStr & CStr::EmptyCStrObj()
{
	return s_empty_cstr_obj;
}

CStr::CStr() 
	: m_len(0)
	, m_cap(0)
	, m_flag( DGN_CSTR_FLAG_NORMAL )
{
	m_str = (char *)&m_len;
}

CStr::~CStr()
{
	if( m_flag == 0 && m_cap > 0 )
		delete [] m_str, m_str = NULL;
}	

CStr::CStr( const char * s )
	: m_len(0)
	, m_cap(0)
	, m_flag( DGN_CSTR_FLAG_NORMAL )
{
	m_str = (char *)&m_len;
	Assign( s );
}

CStr::CStr( const CStr & str )
	: m_len(0)
	, m_cap(0)
	, m_flag( DGN_CSTR_FLAG_NORMAL )
{
	m_str = (char *)&m_len;
	Assign( str.Str(), str.Len() );
}

CStr & CStr::operator = ( const char * s )
{
	if( m_str == s )
		return *this;

	Assign( s );
	return *this;
}

CStr & CStr::operator = ( const CStr & str )
{
	if( this == &str )
		return *this;

	Assign( str.Str(), str.Len() );
	return *this;
}

int CStr::Reserve( int len )
{
	if( m_flag != DGN_CSTR_FLAG_NORMAL )
		return -1;

	if( len <= 0 )
		return -1;

	if( len <= m_cap )
		return m_cap;

	// make a little more space, avoid some realloc, not waste too much
	int newlen = m_cap * 2 + m_cap / 8;
	if( newlen < len )
		newlen = len;
	char * str = new char[newlen];
	if( m_len > 0 )
		memcpy( str, m_str, m_len );
	str[m_len] = '\0';
	if( m_cap > 0 )
		delete [] m_str;
	m_str = str;
	m_cap = newlen;
	return m_cap;
}

int CStr::Assign( const char * str, int len )
{
	if( m_flag == DGN_CSTR_FLAG_EXTCONST )
		return -1;

	if( str == NULL ) {
		if( m_flag == DGN_CSTR_FLAG_EXTBUF ) {
			m_str[0] = '\0';
			return 0;
		}
		// NOTE : side effect to release buffer
		if( m_cap > 0 ) {
			delete [] m_str; 
			m_str = (char *)&m_len;
			m_len = 0;
			m_cap = 0;
		}
		return 0;
	}

	int blen, slen; // the require buf len and actual string len
	if( len < 0 ) {
		slen = (int)strlen( str );
		blen = slen + 1;
	}
	else {
		// NOTE : str may not end with '\0', or has '\0' less than len
		blen = len + 1;
		const char * p = str;
		while( p < str + len && *p != '\0' )
			p++;
		slen = (int)(p - str);
	}

	m_len = 0;
	Reserve( blen );
	if( slen >= m_cap )
		slen = m_cap - 1;
	if( slen > 0 )
		memcpy( m_str, str, slen );
	m_str[slen] = '\0';
	m_len = slen;

	return 0;
}
	
int CStr::AssignFmt( const char * fmt, ... )
{
	if( m_flag == DGN_CSTR_FLAG_EXTCONST )
		return -1;

	if( fmt == NULL ) {
		if( m_flag == DGN_CSTR_FLAG_EXTBUF ) {
			m_str[0] = '\0';
			return 0;
		}
		// NOTE : side effect to release buffer
		if( m_cap > 0 ) {
			delete [] m_str; 
			m_str = (char *)&m_len;
			m_len = 0;
			m_cap = 0;
		}
		return 0;
	}

	// NOTE : snprintf在linux下和windows下的实现的语义有差异，包括结尾的'\0'和返回值，需区分处理
	int count = -1;
	va_list ap;
	if( m_cap > 0 ) {
		va_start( ap, fmt );
		count = vsnprintf( m_str, m_cap, fmt, ap );
		va_end( ap );
	}
	if( count < 0 ) {
		if( m_flag == DGN_CSTR_FLAG_EXTBUF )
			return -1;
		va_start( ap, fmt );
#ifdef _WIN32
		count = _vscprintf( fmt, ap );
#else
		count = vsnprintf( NULL, 0, fmt, ap );
#endif
		va_end( ap );
	}

	if( count >= m_cap ) {
		if( m_flag == DGN_CSTR_FLAG_EXTBUF )
			return -1;
		// buffer is not large enough, enlarge buffer, and redo again
		Reserve( count + 1 ); // add last '\0'
		va_start( ap, fmt );
		count = vsnprintf( m_str, m_cap, fmt, ap );
		va_end( ap );
	}

	if( count >= 0 )
		m_len = count;

	return count >= 0 ? 0 : -1;
}

int CStr::Append( const char * str, int len )
{
	if( m_flag == DGN_CSTR_FLAG_EXTCONST )
		return -1;

	if( str == NULL )
		return 0;

	int blen, slen; // the require buf len and actual string len
	if( len < 0 ) {
		slen = (int)strlen( str );
		blen = slen + 1;
	}
	else {
		// NOTE : str may not end with '\0', or has '\0' less than len
		blen = len + 1;
		const char * p = str;
		while( *p != '\0' && p < str + len )
			p++;
		slen = (int)(p - str);
	}

	Reserve( m_len + blen );
	if( slen >= m_cap - m_len )
		slen = m_cap - m_len - 1;
	if( slen > 0 )
		memcpy( m_str + m_len, str, slen );
	m_str[m_len + slen] = '\0';
	m_len += slen;

	return 0;
}

int CStr::AppendFmt( const char * fmt, ... )
{
	if( m_flag == DGN_CSTR_FLAG_EXTCONST )
		return -1;

	if( fmt == NULL )
		return 0;

	// NOTE : snprintf在linux下和windows下的实现的语义有差异，包括结尾的'\0'和返回值，需区分处理
	int count = -1;
	va_list ap;
	if( m_cap > 0 ) {
		va_start( ap, fmt );
		count = vsnprintf( m_str + m_len, m_cap - m_len, fmt, ap );
		va_end( ap );
	}
	if( count < 0 ) {
		if( m_flag == DGN_CSTR_FLAG_EXTBUF )
			return -1;
		va_start( ap, fmt );
#ifdef _WIN32
		count = _vscprintf( fmt, ap );
#else
		count = vsnprintf( NULL, 0, fmt, ap );
#endif
		va_end( ap );
	}

	if( count >= m_cap - m_len ) {
		if( m_flag == DGN_CSTR_FLAG_EXTBUF )
			return -1;
		Reserve( m_len + count + 1 );
		va_start( ap, fmt );
		count = vsnprintf( m_str + m_len, m_cap - m_len, fmt, ap );
		va_end( ap );
	}
	if( count >= 0 )
		m_len += count;

	return count >= 0 ? 0 : -1;
}

int CStr::Cmp( const char * str, int len ) const
{
	if( str == NULL ) {
		if( m_str[0] == '\0' )
			return 0;
		else
			return 1;
	}

	if( len < 0 )
		return strcmp( m_str, str );

	return strncmp( m_str, str, len );
}

int CStr::CaseCmp( const char * str, int len ) const
{
	if( str == NULL ) {
		if( m_str[0] == '\0' )
			return 0;
		else
			return 1;
	}

	if( len < 0 ) {
#ifdef _WIN32
		return _stricmp( m_str, str );
#else
		return strcasecmp( m_str, str );
#endif
	}

#ifdef _WIN32
	return _strnicmp( m_str, str, len );
#else
	return strncasecmp( m_str, str, len );
#endif
}

int CStr::Cmp( const char * s1, const char * s2, int n /*= -1*/ )
{
	if( n < 0 )
		return strcmp( s1, s2 );
	return strncmp( s1, s2, n );
}

int CStr::CaseCmp( const char * s1, const char * s2, int n /*= -1*/ )
{
	if( n < 0 ) {
#ifdef _WIN32
		return _stricmp( s1, s2 );
#else
		return strcasecmp( s1, s2 );
#endif
	}

#ifdef _WIN32
	return _strnicmp( s1, s2, n );
#else
	return strncasecmp( s1, s2, n );
#endif
}

CStr operator + ( const CStr & left, const CStr & right )
{
	int len = left.Len() + right.Len();
	if( len == 0 )
		return CStr();
	CStr str;
	str.m_len = len;
	str.m_cap = len + 1;
	str.m_str = new char[len + 1];
	if( left.Len() > 0 )
		memcpy( str.m_str, left.m_str, left.Len() );
	if( right.Len() > 0 )
		memcpy( str.m_str + left.Len(), right.m_str, right.Len() );
	str.m_str[len] = '\0';
	return str;
}

int CStr::ReleaseRaw( int len )
{
	if( m_cap == 0 ) {
		m_len = 0;
		return 0;
	}
	if( len < 0 || len >= m_cap ) {
		m_len = 0;
		m_str[len] = '\0';
		return 0;
	}
	m_len = len;
	m_str[len] = '\0';
	return len;
}

CStr & CStr::AttachBuffer( char * buf, int bufsize )
{
	if( m_flag == DGN_CSTR_FLAG_NORMAL ) {
		if( m_cap != 0 )
			delete [] m_str, m_str = NULL;
	}
	if( buf == NULL || bufsize <= 0 ) {
		m_str = (char *)&m_len;
		m_len = 0;
		m_cap = 0;
		m_flag = DGN_CSTR_FLAG_EXTBUF;
		return *this;
	}
	buf[bufsize - 1] = '\0';
	m_str = buf;
	m_cap = bufsize;
	m_len = strlen(m_str);
	m_flag = DGN_CSTR_FLAG_EXTBUF;
	return *this;
}

CStr & CStr::AttachConst( const char * str, int len )
{
	if( m_flag == DGN_CSTR_FLAG_NORMAL ) {
		if( m_cap != 0 )
			delete [] m_str, m_str = NULL;
	}
	if( str == NULL ) {
		m_str = (char *)&m_len;
		m_len = 0;
		m_cap = 0;
		m_flag = DGN_CSTR_FLAG_EXTCONST;
		return *this;
	}
	m_str = (char *)str;
	m_len = (len < 0) ? strlen(str) : len;
	m_cap = m_len + 1;
	m_flag = DGN_CSTR_FLAG_EXTCONST;
	return *this;
}

int CStr::ToInt() const
{
	return atoi( m_str );
}

int64_t CStr::ToInt64() const
{
#ifdef _WIN32
	return _atoi64( m_str );
#else
	return (int64_t)atoll( m_str );
#endif
}

double CStr::ToDouble() const
{
	return atof( m_str );
}

int CStr::ToInt( const char * s )
{
	return atoi( s );
}

int64_t CStr::ToInt64( const char * s )
{
#ifdef _WIN32
	return _atoi64( s );
#else
	return (int64_t)atoll( s );
#endif
}

double CStr::ToDouble( const char * s )
{
	return atof( s );
}

int CStr::ToUpper()
{
	if( m_flag == DGN_CSTR_FLAG_EXTCONST )
		return -1;
	int i;
	for( i = 0; i < m_len; ++i )
		if( m_str[i] >= 'a' && m_str[i] <= 'z' )
			m_str[i] = m_str[i] - 'a' + 'A';
	return 0;
}

int CStr::ToLower()
{
	if( m_flag == DGN_CSTR_FLAG_EXTCONST )
		return -1;
	int i;
	for( i = 0; i < m_len; ++i )
		if( m_str[i] >= 'A' && m_str[i] <= 'Z' )
			m_str[i] = m_str[i] - 'A' + 'a';
	return 0;
}

////////////////
END_NS_DGN

