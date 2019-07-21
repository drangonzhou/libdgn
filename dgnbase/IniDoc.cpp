// IniDoc.cpp : INI document parser
// Copyright (C) 2012 ~ 2019 drangon zhou <drangon.zhou (at) gmail.com>
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

#include <dgn/IniDoc.h>
#include <stdio.h>
#include <string.h>

BEGIN_NS_DGN
////////////////

int IniSection::Set( const char * key, const char * val )
{
	if( key == NULL || key[0] == '\0' )
		return -1;
	m_kvs[ CStr().AttachConst( key ) ] = val;
	return 0;
}

int IniSection::Del( const char * key )
{
	return m_kvs.erase( CStr().AttachConst( key ) );
}

const char * IniSection::Get( const char * key ) const
{
	std::map< CStr, CStr >::const_iterator it = m_kvs.find( CStr().AttachConst( key ) );
	if( it == m_kvs.end() )
		return NULL;
	return it->second.Str();
}

int IniDoc::Load( const char * fname )
{
	CStr buf;
	FILE * fp = fopen( fname, "r" );
	if( fp == NULL )
		return -1;
	fseek( fp, 0, SEEK_END );
	int size = (int)ftell( fp );
	fseek( fp, 0, SEEK_SET );
	buf.Reserve( size + 1 );
	char * p = buf.GetRaw();
	int len = fread( p, 1, size, fp );
	buf.ReleaseRaw( len );
	fclose( fp ), fp = NULL;
	return LoadBuf( buf );
}

int IniDoc::LoadBuf( const CStr & buf )
{
	IniSection * sect = NULL;
	const char * p = buf.Str();
	int line = 1; // line end with \r\n or \n, single \r should not happend, treat as one line
	while( *p != '\0' ) {
		p += strspn( p, " \t" );
		if( *p == '\0' ) {
			break;
		}
		else if( *p == '\r' ) {
			// single '\r' ? should not happend, treat as single line
			if( *(p+1) == '\n' )
				p += 1;
			p += 1;
		}
		else if( *p == '\n' ) {
			p += 1;
		}
		else if( *p == ';' || *p == '#' ) {
			// skip comment
			p += strcspn( p, "\r\n" );
			if( *p == '\r' && *(p + 1) == '\n' )
				p += 1;
			p += 1;
		}
		else if( *p == '[' ) {
			// read section :  [ sect name ] 
			p += 1; // skip '['
			p += strspn( p, " \t" );
			size_t n = strcspn( p, "]\n" );
			printf( "n %d, p[0] %c, p[n] %c\n", (int)n, p[0], p[n] );
			if( *(p + n) != ']' )
				return -1;
			size_t i = n - 1;
			printf( "(1) i %lld\n", i );
			while( i >= 0 && ( *(p + i) == ' ' || *(p + i) == '\t' ) )
				i--;
			printf( "(2) i %lld\n", i );
			CStr name;
			name.Assign( p, i + 1 );
			p += n + 1;
			p += strspn( p, " \t" );
			if( *p != '\n' && *p != '\r' && *p != '\0' )
				return -1;
			if( *p == '\r' && *(p + 1) == '\n' )
				p += 1;
			p += 1;
			printf( "name is [%s], i %d\n", name.Str(), (int)i );
			sect = GetSection( name.Str() );
			if( sect == NULL ) {
				AddSection( name.Str() );
				sect = GetSection( name.Str() );
			}
		}
		else {
			// read key/value : key name = value data
			p += strspn( p, " \t" );
			if( *p == '=' )
				return -1;
			size_t n = strcspn( p, "=\n" );
			if( *(p + n) == '\n' )
				return -1;
			CStr key, value;
			size_t i = n - 1;
			while( i >= 0 && ( *(p + i) == ' ' || *(p + i) == '\t' ) )
				i--;
			key.Assign( p, i + 1 );
			p += n + 1;
			p += strspn( p, " \t" );
			size_t n2 = strcspn( p, "\r\n" );
			i = n2 - 1;
			while( i >= 0 && ( *(p + i) == ' ' || *(p + i) == '\t' ) )
				i--;
			value.Assign( p, i + 1 );
			if( *(p + n2) == '\r' && *(p + n2 + 1) == '\n' )
				n2 += 1;
			p += n2 + 1;
			if( sect == NULL ) {
				AddSection( "" );
				sect = GetSection( "" );
			}
			sect->Set( key.Str(), value.Str() );
		}
		line += 1;
	}
	return 0;
}

int IniDoc::Save( const char * fname )
{
	CStr buf;
	SaveBuf( &buf );
	FILE * fp = fopen( fname, "w" );
	if( fp == NULL )
		return -1;
	size_t ret = fwrite( buf.Str(), 1, buf.Len(), fp );
	fclose( fp ), fp = NULL;
	return (int)ret;
}

int IniDoc::SaveBuf( CStr * buf )
{
	int len = 0;
	CIter it;
	for( it = Begin(); it != End(); ++it ) {
		len += it->first.Len() + 3;
		IniSection::CIter it2;
		for( it2 = it->second->Begin(); it2 != it->second->End(); ++it2 ) {
			len += it2->first.Len() + it2->second.Len() + 3;
		}
		len += 1;
	}
	len += 2;

	buf->Assign( "", 0 );
	buf->Reserve( len );
	for( it = Begin(); it != End(); ++it ) {
		buf->AppendFmt( "[%s]\n", it->first.Str() );
		IniSection::CIter it2;
		for( it2 = it->second->Begin(); it2 != it->second->End(); ++it2 ) {
			buf->AppendFmt( "%s = %s\n", it2->first.Str(), it2->second.Str() );
		}
		buf->Append( "\n" );
	}
	buf->Append( "\n" );

	return 0;
}

int IniDoc::Reset()
{
	Iter it;
	for( it = Begin(); it != End(); ++it ) {
		delete it->second, it->second = NULL;	
	}
	m_sects.clear();
	return 0;
}

int IniDoc::AddSection( const char * name )
{
	CStr ns;
	ns.AttachConst( name );
	if( m_sects.find( ns ) != m_sects.end() )
		return 0;
	IniSection * sect = new IniSection();
	m_sects.insert( std::make_pair<>( ns, sect ) );
	return 1;
}

int IniDoc::DelSection( const char * name )
{
	return m_sects.erase( CStr().AttachConst( name ) );
}

IniSection * IniDoc::GetSection( const char * name )
{
	Iter it = m_sects.find( CStr().AttachConst( name ) );
	if( it == End() )
		return NULL;
	return it->second;
}

int IniDoc::Set( const char * name, const char * key, const char * val )
{
	IniSection * sect = GetSection( name );
	if( sect == NULL )
		return 0;
	return sect->Set( key, val );
}

int IniDoc::Del( const char * name, const char * key )
{
	IniSection * sect = GetSection( name );
	if( sect == NULL )
		return 0;
	return sect->Del( key );
}

const char * IniDoc::Get( const char * name, const char * key ) const
{
	CIter it = m_sects.find( name );
	if( it == m_sects.end() )
		return NULL;
	return it->second->Get( key );
}

////////////////
END_NS_DGN

