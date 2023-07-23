// JsonVal.h : json value
// Copyright (C) 2011 ~ 2023 drangon <drangon.zhou (at) gmail.com>
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

class DGN_LIB_API JsonVal
{
public:
	explicit JsonVal( enum jsonval_type_e type = JSONVAL_TYPE_NULL );
	explicit JsonVal( int val );
	explicit JsonVal( int64_t val );
	explicit JsonVal( double val );
	explicit JsonVal( const char * val, int n = -1 );
	explicit JsonVal( const CStr & val );
	explicit JsonVal( CStr && val );
	JsonVal( const JsonVal & jv );
	JsonVal( JsonVal && jv );
	~JsonVal();

	JsonVal & operator = ( const JsonVal & jv );
	JsonVal & operator = ( JsonVal && jv );

	static const JsonVal & NullJsonVal();

	static JsonVal Parse( const char * json );
	static JsonVal Parse( const CStr & json ) { return Parse( json.Str() ); }

	int FromBuf( const char * json );
	int FromBuf( const CStr & json ) { return FromBuf( json.Str() ); }
	//int ToBuf( char * buf, int maxlen ) const;
	// append to CStr, clear buffer before call this if needed
	int ToBuf( CStr * buf ) const;
	CStr ToBuf() const { CStr str; ToBuf( &str ); return str; }

public:
	enum jsonval_type_e GetType() const { return m_type; }
	void SetType( enum jsonval_type_e type );
	void SetNull() { SetType( JSONVAL_TYPE_NULL ); return; }
	void SetTrue() { SetType( JSONVAL_TYPE_TRUE ); return; }
	void SetFalse() { SetType( JSONVAL_TYPE_FALSE ); return; }

	// when get value, return *zero* value if not same type
	// when set value, change type if needed
	int GetInt() const;
	void SetInt( int val );
	JsonVal & operator = ( int val );

	int64_t GetInt64() const;
	void SetInt64( int64_t val );
	JsonVal & operator = ( int64_t val );

	double GetDouble() const;
	void SetDouble( double val );
	JsonVal & operator = ( double val );

	const CStr & GetString() const;
	void SetString( const char * str, int n = -1 ) { return SetString( CStr().AttachConst( str, n ) ); }
	void SetString( const CStr & str );
	void SetString( CStr && str );
	JsonVal & operator = ( const char * str ) { SetString( str ); return *this; }
	JsonVal & operator = ( const CStr & str ) { SetString( str ); return *this; }
	JsonVal & operator = ( CStr && str ) { SetString( str ); return *this; }

public:
	// array or object size, other type return 0
	int Size() const;

	// return empty val if not array type or index not valid
	const JsonVal & GetItem( int index ) const;
	const JsonVal & operator [] ( int index ) const { return GetItem( index ); }
	// ArrayCIter point to array item : const JsonVal &
	typedef std::vector< JsonVal >::const_iterator ArrayCIter;
	// return Empty array if not array type
	JsonVal::ArrayCIter ArrayBegin() const;
	JsonVal::ArrayCIter ArrayEnd() const;

	void SetArray( int size = 0 );
	// create item and change type if needed, if index < 0, set index = 0
	JsonVal & GetItem( int index );
	JsonVal & operator [] ( int index ) { return GetItem( index ); }
	// change to array type if needed, if index < 0, set index = 0
	void SetItem( int index, const JsonVal & obj );
	void SetItem( int index, JsonVal && obj );
	// ArrayIter point to array item : JsonVal &
	typedef std::vector< JsonVal >::iterator ArrayIter;
	// change to array type if needed
	JsonVal::ArrayIter ArrayBegin();
	JsonVal::ArrayIter ArrayEnd();

	bool HasKey( const char * key ) const { return HasKey( CStr().AttachConst( key ) ); }
	bool HasKey( const CStr & key ) const;
	// return empty object if not object type or key not found
	const JsonVal & GetItem( const char * key ) const { return GetItem( CStr().AttachConst(key) ); }
	const JsonVal & GetItem( const CStr & key ) const;
	const JsonVal & operator [] ( const char * key ) const { return GetItem( key ); }
	const JsonVal & operator [] ( const CStr & key ) const { return GetItem( key ); }
	// ObjectCIter->first is key : const CStr &, ObjectCIter->second is val : const JsonVal &
	typedef std::map< CStr, JsonVal >::const_iterator ObjectCIter;
	// return Empty object if not object type
	JsonVal::ObjectCIter ObjectBegin() const;
	JsonVal::ObjectCIter ObjectEnd() const;

	void SetObject() { SetType( JSONVAL_TYPE_OBJECT ); return; }
	// change to object type if needed, create item if key not found
	JsonVal & GetItem( const char * key ) { return GetItem( CStr().AttachConst( key ) ); }
	JsonVal & GetItem( const CStr & key );
	JsonVal & operator [] ( const char * key ) { return GetItem( key ); }
	JsonVal & operator [] ( const CStr & key ) { return GetItem( key ); }
	void SetItem( const char * key, const JsonVal & obj ) {	return SetItem( CStr().AttachConst( key ), obj ); }
	void SetItem( const CStr & key, const JsonVal & obj );
	void SetItem( const char * key, JsonVal && obj ) { return SetItem( CStr().AttachConst( key ), obj ); }
	void SetItem( const CStr & key, JsonVal && obj );

	// ObjectIter->first is key : const CStr &, ObjectIter->second is val : JsonVal &
	typedef std::map< CStr, JsonVal >::const_iterator ObjectIter;
	// change to object type if needed
	JsonVal::ObjectIter ObjectBegin();
	JsonVal::ObjectIter ObjectEnd();

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
		std::vector< JsonVal > * arr;
		std::map< CStr, JsonVal > * obj;
	} m_val;
};

//////////////////////////////////
END_NS_DGN

#endif // INCLUDED_DGN_JSONVAL_H

