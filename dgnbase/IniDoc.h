// IniDoc.h : INI document parser
// Copyright (C) 2012 ~ 2023 drangon zhou <drangon.zhou (at) gmail.com>
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

#ifndef INCLUDED_DGN_INIDOC_H
#define INCLUDED_DGN_INIDOC_H

#include <dgn/CStr.h>
#include <map>

BEGIN_NS_DGN
////////////////

// Note :
// comment must in single line, begin with '#' or ';' , and ignore by parser
// no section means empty section, with section name ""
// key and value can contain space, but begin and end space is stripped.
// optimise for reading

class DGN_LIB_API IniSection
{
public:
	IniSection() = default;
	~IniSection() = default;

	int Reset() { m_kvs.clear(); return 0; }

	// auto create new key if key not exist
	CStr & Get( const CStr & key );
	// Iter->first is key : const CStr &, Iter->second is val : CStr &
	typedef std::map< CStr, CStr >::iterator Iter;
	IniSection::Iter Begin() { return m_kvs.begin(); }
	IniSection::Iter End() { return m_kvs.end(); }

	int Set( const char * key, const char * val ) { return Set( CStr().AttachConst( key ), CStr().AttachConst( val ) ); }
	int Set( const CStr & key, const CStr & val );
	int Del( const char * key ) { return Del( CStr().AttachConst( key ) ); }
	int Del( const CStr & key );

public:
	// CIter->first is key : const CStr &, CIter->second is val : const CStr &
	typedef std::map< CStr, CStr >::const_iterator CIter;
	IniSection::CIter Begin() const { return m_kvs.begin(); }
	IniSection::CIter End() const { return m_kvs.end(); }
	int Size() const { return (int)m_kvs.size(); }
	
	// return NULL if key not exist
	const char * Get( const char * key ) const;
	// return Empty CStr if key not exist, same as key exist and value is Empty CStr
	const CStr & Get( const CStr & key ) const; 
	
	bool HasKey( const char * key ) const { return HasKey( CStr().AttachConst( key ) ); }
	bool HasKey( const CStr & key ) const { return m_kvs.find( key ) != m_kvs.end(); }

private:
	std::map< CStr, CStr > m_kvs;
};

class DGN_LIB_API IniDoc
{
public:
	IniDoc() {}
	~IniDoc() {}

	int LoadFile( const char * fname );
	int LoadFile( const CStr & fname ) { return LoadFile( fname.Str() ); }
	int LoadBuf( const char * buf, int len = -1 ); 
	int LoadBuf( const CStr & buf ) { return LoadBuf( buf.Str(), buf.Len() ); }
	int SaveFile( const char * fname );
	int SaveFile( const CStr & fname ) { return SaveFile( fname.Str() ); }
	int SaveBuf( CStr * buf );

	int Reset() { m_sects.clear(); return 0; }

public:
	int AddSection( const char * name ) { return AddSection( CStr().AttachConst( name ) ); }
	int AddSection( const CStr & name );
	int DelSection( const char * name ) { return DelSection( CStr().AttachConst( name ) ); }
	int DelSection( const CStr & name );

	// return new section if section not exist
	IniSection & GetSection( const CStr & name );

	// auto create new key if section or key not exist
	CStr & Get( const CStr & name, const CStr & key );

	// Iter->first is key : const CStr &, Iter->second is val : IniSection &
	typedef std::map< CStr, IniSection >::iterator Iter;
	IniDoc::Iter Begin() { return m_sects.begin(); }
	IniDoc::Iter End() { return m_sects.end(); }

	int Set( const char * name, const char * key, const char * val ) { return Set( CStr().AttachConst( name ), CStr().AttachConst( key ), CStr().AttachConst( val ) ); }
	int Set( const CStr & name, const CStr & key, const CStr & val );
	int Del( const char * name, const char * key ) { return Del( CStr().AttachConst( name ), CStr().AttachConst( key ) ); }
	int Del( const CStr & name, const CStr & key );

public:
	// Iter->first is key : const CStr &, Iter->second is val : const IniSection &
	typedef std::map< CStr, IniSection >::const_iterator CIter;
	IniDoc::CIter Begin() const { return m_sects.begin(); }
	IniDoc::CIter End() const { return m_sects.end(); }
	int Size() const { return (int)m_sects.size(); }

	// return NULL if section not exist
	const IniSection * GetSection( const char * name ) const;
	// return empty IniSection if section not exist
	const IniSection & GetSection( const CStr & name ) const;

	bool HasSection( const char * name ) const { return HasSection( CStr().AttachConst( name ) ); }
	bool HasSection( const CStr & name ) const;

	// return NULL if section or key not exist
	const char * Get( const char * name, const char * key ) const;
	// return Empty CStr if section or key not exist, same as key exist and value is Empty CStr
	const CStr & Get( const CStr & name, const CStr & key ) const;

	bool HasKey( const char * name, const char * key ) const { return HasKey( CStr().AttachConst( name ), CStr().AttachConst( key ) ); }
	bool HasKey( const CStr & name, const CStr & key ) const;

protected:
	std::map< CStr, IniSection > m_sects;
};

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_INIDOC_H

