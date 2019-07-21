// JsonVal.h : json value
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

#ifndef INCLUDED_DGN_JSONVAL_H
#define INCLUDED_DGN_JSONVAL_H

#include <dgn/dgn.h>
#include <dgn/CStr.h>

#include <vector>
#include <map>

BEGIN_NS_DGN
////////////////////////////////

enum jsonval_type_e
{
	JSONVAL_TYPE_NULL = 0,
	JSONVAL_TYPE_FALSE,
	JSONVAL_TYPE_TRUE,
	JSONVAL_TYPE_INT,
	JSONVAL_TYPE_DOUBLE,
	JSONVAL_TYPE_STRING,
	JSONVAL_TYPE_ARRAY,
	JSONVAL_TYPE_OBJECT,
};

class JsonVal
{
public:
	explicit JsonVal( enum jsonval_type_e type = JSONVAL_TYPE_NULL );
	explicit JsonVal( int val );
	explicit JsonVal( int64_t val );
	explicit JsonVal( double val );
	explicit JsonVal( const char * val, int n = -1 );
	explicit JsonVal( const CStr & str );
	JsonVal( const JsonVal & jv );
	~JsonVal();

	JsonVal & operator = ( const JsonVal & jv );

	static const JsonVal & NullJsonVal();

	static JsonVal Parse( const char * json );
	static JsonVal Parse( const CStr & json );

	int FromBuf( const char * json );
	int FromBuf( const CStr & json );
	int ToBuf( char * buf, int maxlen ) const;
	int ToBuf( CStr * buf ) const;
	CStr ToBuf() const;

public:
	enum jsonval_type_e GetType() const;
	void SetType( enum jsonval_type_e type );
	void SetNull() { SetType( JSONVAL_TYPE_NULL ); return; }
	void SetTrue() { SetType( JSONVAL_TYPE_TRUE ); return; }
	void SetFalse() { SetType( JSONVAL_TYPE_FALSE ); return; }

	int GetInt() const;
	void SetInt( int val );
	JsonVal & operator = ( int val );

	int64_t GetInt64() const;
	void SetInt64( int64_t val );
	JsonVal & operator = ( int64_t val );

	double GetDouble() const;
	void SetDouble( double val );
	JsonVal & operator = ( double val );

	const char * GetString() const;
	void SetString( const char * str, int n = -1 );
	void SetString( const CStr & str );
	JsonVal & operator = ( const char * str );
	JsonVal & operator = ( const CStr & str );

	int GetArrayLen() const;
	const JsonVal & GetItem( int index ) const;
	JsonVal & GotItem( int index );  // create item and change type if needed
	const JsonVal & operator [] ( int index ) const;
	JsonVal & operator [] ( int index );
	void SetArray( int size = 0 );
	void SetItem( int index, const JsonVal & obj );

	// TODO : add object item iterator
	int GetAllItemName( std::vector< CStr > * ret ) const;
	const JsonVal & GetItem( const char * name ) const;
	const JsonVal & GetItem( const CStr & name ) const;
	JsonVal & GotItem( const char * name ); // create item and change type if needed
	JsonVal & GotItem( const CStr & name );
	const JsonVal & operator [] ( const char * name ) const;
	const JsonVal & operator [] ( const CStr & name ) const;
	JsonVal & operator [] ( const char * name );
	JsonVal & operator [] ( const CStr & name );
	void SetObject();
	void SetItem( const char * name, const JsonVal & obj );
	void SetItem( const CStr & name, const JsonVal & obj );
	
protected:
	void clear();
	void assign( const JsonVal & jv );
	int get_json_size() const;
	int do_to_json( CStr * buf ) const;
	int do_from_json( const char * str );

protected:
	enum jsonval_type_e m_type;
	union {
		int64_t i;
		double d;
		CStr * s;
		std::vector< JsonVal * > * arr;
		std::map< CStr, JsonVal * > * obj;
	} m_val;
};

//////////////////////////////////
END_NS_DGN

#endif // INCLUDED_DGN_JSONVAL_H

