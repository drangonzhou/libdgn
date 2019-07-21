// IniDoc.h : INI document parser
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

class IniSection
{
private:
	friend class IniDoc;
	IniSection() { }
	~IniSection() { Reset(); }

public:
	int Reset() { m_kvs.clear(); return 0; }
	int Set( const char * key, const char * val );
	int Del( const char * key );

public:
	// CIter->first is key : CStr, CIter->second is val : CStr
	typedef std::map< CStr, CStr >::const_iterator CIter;
	CIter Begin() const { return m_kvs.begin(); }
	CIter End() const { return m_kvs.end(); }
	int Size() const { return (int)m_kvs.size(); }
	
	const char * Get( const char * key ) const;

private:
	std::map< CStr, CStr > m_kvs;
};

class IniDoc
{
public:
	IniDoc() {}
	~IniDoc() { Reset(); }

	int Load( const char * fname );
	int LoadBuf( const CStr & buf );
	int Save( const char * fname );
	int SaveBuf( CStr * buf );

	int Reset();

public:
	int AddSection( const char * name );
	int DelSection( const char * name );
	IniSection * GetSection( const char * name );

	typedef std::map< CStr, IniSection * >::iterator Iter;
	Iter Begin() { return m_sects.begin(); }
	Iter End() { return m_sects.end(); }
	int Size() { return (int)m_sects.size(); }

	int Set( const char * name, const char * key, const char * val );
	int Del( const char * name, const char * key );

public:
	typedef std::map< CStr, IniSection * >::const_iterator CIter;
	CIter begin() const { return m_sects.begin(); }
	CIter end() const { return m_sects.end(); }

	const char * Get( const char * name, const char * key ) const;

protected:
	std::map< CStr, IniSection * > m_sects;
};

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_INIDOC_H

