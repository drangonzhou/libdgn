// JsonVal.cpp : json value
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

#include "JsonVal.h"

#include <math.h> // pow()
#include <string.h>

BEGIN_NS_DGN
////////////////////////////////

static const JsonVal s_jsonval_empty;
static const std::vector< JsonVal > s_jsonval_array_empty;
static const std::map< CStr, JsonVal > s_jsonval_object_empty;

const JsonVal & JsonVal::NullJsonVal()
{
	return s_jsonval_empty;
}

JsonVal::JsonVal( enum jsonval_type_e type ) : m_type( type )
{
	switch( type )
	{
	case JSONVAL_TYPE_NULL :
	case JSONVAL_TYPE_FALSE :
	case JSONVAL_TYPE_TRUE :
	case JSONVAL_TYPE_INT :
		m_val.i = 0;
		break;
	case JSONVAL_TYPE_DOUBLE :
		m_val.d = 0.0;
		break;
	case JSONVAL_TYPE_STRING :
		m_val.s = new dgn::CStr();
		break;
	case JSONVAL_TYPE_ARRAY :
		m_val.arr = new std::vector< JsonVal >();
		break;
	case JSONVAL_TYPE_OBJECT :
		m_val.obj = new std::map< CStr, JsonVal >();
		break;
	default :
		break;
	};
}

JsonVal::JsonVal( int val )
{
	m_type = JSONVAL_TYPE_INT;
	m_val.i = (int64_t)val;
}

JsonVal::JsonVal( int64_t val )
{
	m_type = JSONVAL_TYPE_INT;
	m_val.i = val;
}

JsonVal::JsonVal( double val )
{
	m_type = JSONVAL_TYPE_DOUBLE;
	m_val.d = val;
}

JsonVal::JsonVal( const char * val, int n )
{
	m_type = JSONVAL_TYPE_STRING;
	m_val.s = new dgn::CStr();
	m_val.s->Assign( val, n );
}

JsonVal::JsonVal( const CStr & val )
{
	m_type = JSONVAL_TYPE_STRING;
	m_val.s = new dgn::CStr( val );
}

JsonVal::JsonVal( CStr && val )
{
	m_type = JSONVAL_TYPE_STRING;
	m_val.s = new dgn::CStr( val );
}

JsonVal::JsonVal( const JsonVal & jv )
{
	m_type = JSONVAL_TYPE_NULL;
	assign( jv );
}

JsonVal::JsonVal( JsonVal && jv )
{
	m_type = jv.m_type;
	switch( m_type )
	{
	case JSONVAL_TYPE_NULL:
	case JSONVAL_TYPE_FALSE:
	case JSONVAL_TYPE_TRUE:
	case JSONVAL_TYPE_INT:
		m_val.i = jv.m_val.i;
		break;
	case JSONVAL_TYPE_DOUBLE:
		m_val.d = jv.m_val.d;
		break;
	case JSONVAL_TYPE_STRING:
		m_val.s = jv.m_val.s;
		jv.m_val.s = NULL;
		break;
	case JSONVAL_TYPE_ARRAY:
		m_val.arr = jv.m_val.arr;
		jv.m_val.arr = NULL;
		break;
	case JSONVAL_TYPE_OBJECT:
		m_val.obj = jv.m_val.obj;
		jv.m_val.obj = NULL;
		break;
	default:
		break;
	};
	jv.m_type = JSONVAL_TYPE_NULL;
}


JsonVal::~JsonVal()
{
	clear();
}

void JsonVal::clear()
{
	switch( m_type )
	{
	case JSONVAL_TYPE_STRING :
		delete m_val.s;
		break;
	case JSONVAL_TYPE_ARRAY :
		delete m_val.arr;
		break;
	case JSONVAL_TYPE_OBJECT :
		delete m_val.obj;
		break;
	default :
		break;
	};
	m_type = JSONVAL_TYPE_NULL;
	m_val.i = 0;
	return;
}

JsonVal & JsonVal::operator = ( const JsonVal & jv )
{
	if( this == &jv )
		return *this;
	assign( jv );
	return *this;
}

JsonVal & JsonVal::operator = ( JsonVal && jv )
{
	if( this == &jv )
		return *this;
	clear();
	m_type = jv.m_type;
	switch( m_type )
	{
	case JSONVAL_TYPE_NULL:
	case JSONVAL_TYPE_FALSE:
	case JSONVAL_TYPE_TRUE:
	case JSONVAL_TYPE_INT:
		m_val.i = jv.m_val.i;
		break;
	case JSONVAL_TYPE_DOUBLE:
		m_val.d = jv.m_val.d;
		break;
	case JSONVAL_TYPE_STRING:
		m_val.s = jv.m_val.s;
		jv.m_val.s = NULL;
		break;
	case JSONVAL_TYPE_ARRAY:
		m_val.arr = jv.m_val.arr;
		jv.m_val.arr = NULL;
		break;
	case JSONVAL_TYPE_OBJECT:
		m_val.obj = jv.m_val.obj;
		jv.m_val.obj = NULL;
		break;
	default:
		break;
	};
	jv.m_type = JSONVAL_TYPE_NULL;
	return *this;
}

void JsonVal::assign( const JsonVal & jv )
{
	if( m_type != jv.m_type ) {
		clear();
	}
	switch( jv.m_type )
	{
	case JSONVAL_TYPE_NULL :
	case JSONVAL_TYPE_FALSE :
	case JSONVAL_TYPE_TRUE :
		break;
	case JSONVAL_TYPE_INT :
		m_val.i = jv.m_val.i;
		break;
	case JSONVAL_TYPE_DOUBLE :
		m_val.d = jv.m_val.d;
		break;
	case JSONVAL_TYPE_STRING :
		if( m_type == JSONVAL_TYPE_NULL )
			m_val.s = new dgn::CStr( *jv.m_val.s );
		else
			m_val.s->Assign( *jv.m_val.s );
		break;
	case JSONVAL_TYPE_ARRAY :
		if( m_type == JSONVAL_TYPE_NULL )
			m_val.arr = new std::vector< JsonVal >( *jv.m_val.arr );
		else
			*m_val.arr = *jv.m_val.arr;
		break;
	case JSONVAL_TYPE_OBJECT :
		if( m_type == JSONVAL_TYPE_NULL )
			m_val.obj = new std::map< CStr, JsonVal >( *jv.m_val.obj );
		else
			*m_val.obj = *jv.m_val.obj;
		break;
	default :
		break;
	}
	m_type = jv.m_type;

	return;
}

void JsonVal::SetType( enum jsonval_type_e type )
{
	if( m_type == type )
		return;

	clear();
	m_type = type;
	switch( m_type )
	{
	case JSONVAL_TYPE_NULL :
	case JSONVAL_TYPE_FALSE :
	case JSONVAL_TYPE_TRUE :
	case JSONVAL_TYPE_INT :
		m_val.i = 0;
		break;
	case JSONVAL_TYPE_DOUBLE :
		m_val.d = 0.0;
		break;
	case JSONVAL_TYPE_STRING :
		m_val.s = new dgn::CStr();
		break;
	case JSONVAL_TYPE_ARRAY :
		m_val.arr = new std::vector< JsonVal >();
		break;
	case JSONVAL_TYPE_OBJECT :
		m_val.obj = new std::map< CStr, JsonVal >();
		break;
	default :
		break;
	};
	return;
}

int JsonVal::GetInt() const
{
	if( m_type == JSONVAL_TYPE_INT )
		return (int)m_val.i;
	return 0;
}

void JsonVal::SetInt( int val )
{
	SetType( JSONVAL_TYPE_INT );
	m_val.i = (int64_t)val;
	return;
}

JsonVal & JsonVal::operator = ( int val )
{
	SetInt( val );
	return *this;
}

int64_t JsonVal::GetInt64() const
{
	if( m_type == JSONVAL_TYPE_INT )
		return m_val.i;
	return 0;
}

void JsonVal::SetInt64( int64_t val )
{
	SetType( JSONVAL_TYPE_INT );
	m_val.i = val;
	return;
}

JsonVal & JsonVal::operator = ( int64_t val )
{
	SetInt64( val );
	return *this;
}

double JsonVal::GetDouble() const
{
	if( m_type == JSONVAL_TYPE_DOUBLE )
		return m_val.d;
	return 0.0;
}

void JsonVal::SetDouble( double val )
{
	SetType( JSONVAL_TYPE_DOUBLE );
	m_val.d = val;
	return;
}

JsonVal & JsonVal::operator = ( double val )
{
	SetDouble( val );
	return *this;
}

const CStr & JsonVal::GetString() const
{
	if( m_type != JSONVAL_TYPE_STRING )
		return CStr::EmptyCStrObj();
	return *m_val.s;
}

void JsonVal::SetString( const CStr & str )
{
	SetType( JSONVAL_TYPE_STRING );
	( *m_val.s ) = str;
	return;
}

void JsonVal::SetString( CStr && str )
{
	SetType( JSONVAL_TYPE_STRING );
	( *m_val.s ) = str;
	return;
}

int JsonVal::Size() const
{
	if( m_type == JSONVAL_TYPE_ARRAY )
		return (int)m_val.arr->size();
	else if( m_type == JSONVAL_TYPE_OBJECT )
		return (int)m_val.obj->size();
	return 0;
}

const JsonVal & JsonVal::GetItem( int index ) const
{
	if( m_type != JSONVAL_TYPE_ARRAY || index < 0 || index >= (int)m_val.arr->size() )
		return NullJsonVal();
	return (*m_val.arr)[index];
}

JsonVal::ArrayCIter JsonVal::ArrayBegin() const
{
	if( m_type == JSONVAL_TYPE_ARRAY )
		return m_val.arr->begin();
	return s_jsonval_array_empty.begin();
}

JsonVal::ArrayCIter JsonVal::ArrayEnd() const
{
	if( m_type == JSONVAL_TYPE_ARRAY )
		return m_val.arr->end();
	return s_jsonval_array_empty.end();
}

void JsonVal::SetArray( int size )
{
	SetType( JSONVAL_TYPE_ARRAY );
	if( size < 0 )
		size = 0;
	m_val.arr->resize( size );
	return;
}

JsonVal & JsonVal::GetItem( int index )
{
	SetType( JSONVAL_TYPE_ARRAY );
	if( index < 0 ) {
		index = 0;
	}
	int sz = (int)m_val.arr->size();
	if( index >= sz ) {
		m_val.arr->resize( index + 1 );
	}
	return (*m_val.arr)[index];
}

void JsonVal::SetItem( int index, const JsonVal & obj )
{
	JsonVal & val = GetItem( index );
	val = obj;
	return;
}

void JsonVal::SetItem( int index, JsonVal && obj )
{
	JsonVal & val = GetItem( index );
	val = obj;
	return;
}

JsonVal::ArrayIter JsonVal::ArrayBegin()
{
	SetType( JSONVAL_TYPE_ARRAY );
	return m_val.arr->begin();
}

JsonVal::ArrayIter JsonVal::ArrayEnd()
{
	SetType( JSONVAL_TYPE_ARRAY );
	return m_val.arr->end();
}

const JsonVal & JsonVal::GetItem( const CStr & name ) const
{
	if( m_type != JSONVAL_TYPE_OBJECT )
		return NullJsonVal();
	std::map< CStr, JsonVal >::const_iterator it = m_val.obj->find( name );
	if( it == m_val.obj->end() )
		return NullJsonVal();
	return it->second;
}

JsonVal::ObjectCIter JsonVal::ObjectBegin() const
{
	if( m_type == JSONVAL_TYPE_OBJECT )
		return m_val.obj->begin();
	return s_jsonval_object_empty.begin();
}

JsonVal::ObjectCIter JsonVal::ObjectEnd() const
{
	if( m_type == JSONVAL_TYPE_OBJECT )
		return m_val.obj->end();
	return s_jsonval_object_empty.end();
}

JsonVal & JsonVal::GetItem( const CStr & name ) // create item and change type if needed
{
	SetType( JSONVAL_TYPE_OBJECT );
	return ( *m_val.obj )[name];
}

void JsonVal::SetItem( const CStr & name, const JsonVal & obj )
{
	JsonVal & val = GetItem( name );
	val = obj;
	return;
}

void JsonVal::SetItem( const CStr & name, JsonVal && obj )
{
	JsonVal & val = GetItem( name );
	val = obj;
	return;
}

JsonVal::ObjectIter JsonVal::ObjectBegin()
{
	SetType( JSONVAL_TYPE_OBJECT );
	return m_val.obj->begin();
}

JsonVal::ObjectIter JsonVal::ObjectEnd()
{
	SetType( JSONVAL_TYPE_OBJECT );
	return m_val.obj->end();
}

JsonVal JsonVal::Parse( const char * json )
{
	JsonVal jv;
	jv.FromBuf( json );
	return jv;
}

int JsonVal::FromBuf( const char * str )
{
	SetType( JSONVAL_TYPE_NULL );
	if( str == NULL )
		return -1;
	int tmp = do_from_json( str );
	if( tmp > 0 ) {
		int tail = (int)strspn( str + tmp, " \t\r\n" );
		tmp += tail;
		if( str[tmp] != '\0' )
			tmp = -(tmp + 1);
	}
	if( tmp < 0 )
		SetType( JSONVAL_TYPE_NULL );
	return tmp;
}

//int JsonVal::ToBuf( char * buf, int maxlen ) const
//{
//	if( buf == NULL || maxlen <= 0 )
//		return -1;
//	buf[0] = '\0';
//	CStr str;
//	str.AttachBuffer( buf, maxlen );
//	return ToBuf( &str );
//}

int JsonVal::ToBuf( CStr * buf ) const
{
	if( buf == NULL )
		return -1;
	int sz = get_json_size();
	int olen = buf->Len();
	buf->Reserve( olen + sz + 1 );
	do_to_json( buf );
	return buf->Len() - olen;
}

int JsonVal::get_json_size() const
{
	int sz = 0;
	switch( GetType() )
	{
	case JSONVAL_TYPE_NULL :
		sz = 4;
		break;
	case JSONVAL_TYPE_FALSE :
		sz = 5;
		break;
	case JSONVAL_TYPE_TRUE :
		sz = 4;
		break;
	case JSONVAL_TYPE_INT :
		sz = 10; // max 10 number;
		break;
	case JSONVAL_TYPE_DOUBLE:
		sz = 25; // double 16 valid num
		break;
	case JSONVAL_TYPE_STRING :
		// FIXME : escape value
		sz = m_val.s->Len() + 2;
		break;
	case JSONVAL_TYPE_ARRAY :
	{
		int i, num;
		num = Size();
		for( i = 0; i < num; ++i ) {
			const JsonVal & item = GetItem( i );
			sz += item.get_json_size() + 2;
		}
		sz += 4;
	}
	break;
	case JSONVAL_TYPE_OBJECT :
	{
		for( ObjectCIter it = ObjectBegin(); it != ObjectEnd(); ++it ) {
			sz += it->first.Len() + 2 + 3 + it->second.get_json_size() + 2;
		}
		sz += 4;
	}
		break;
	default :
		break;
	}
	return sz;
}

int JsonVal::do_to_json( CStr * buf ) const
{
	switch( GetType() )
	{
	case JSONVAL_TYPE_NULL :
		buf->Append( "null" );
		break;
	case JSONVAL_TYPE_FALSE :
		buf->Append( "false" );
		break;
	case JSONVAL_TYPE_TRUE :
		buf->Append( "true" );
		break;
	case JSONVAL_TYPE_INT :
#ifdef _WIN32
		buf->AppendFmt( "%I64d", m_val.i );
#else
		buf->AppendFmt( "%lld", (long long)m_val.i );
#endif
		break;
	case JSONVAL_TYPE_DOUBLE:
		buf->AppendFmt( "%.20g", m_val.d );
		break;
	case JSONVAL_TYPE_STRING :
		buf->Append( "\"" );
		// FIXME : escape char
		buf->Append( *m_val.s );
		buf->Append( "\"" );
		break;
	case JSONVAL_TYPE_ARRAY :
	{
		int i, num;
		num = Size();
		buf->Append( "[ " );
		for( i = 0; i < num; ++i ) {
			const JsonVal & item = GetItem( i );
			if( i != 0 )
				buf->Append( ", " );
			item.do_to_json( buf );
		}
		buf->Append( " ]" );
	}
		break;
	case JSONVAL_TYPE_OBJECT :
	{
		buf->Append( "{ " );
		for( ObjectCIter it = ObjectBegin(); it != ObjectEnd(); ++it ) {
			if( it != ObjectBegin() )
				buf->Append( ", " );
			buf->Append( "\"" );
			buf->Append( it->first );
			buf->Append( "\" : " );
			it->second.do_to_json( buf );
		}
		buf->Append( " }" );
	}
		break;
	default :
		break;
	}
	return 0;
}

int JsonVal::do_from_json( const char * str )
{
	int len = 0;
	len += (int)strspn( str, " \t\r\n" );
	if( str[len] == 'n' ) {
		if( strncmp( str + len, "null", 4 ) != 0 )
			return -(len + 1);
		SetNull();
		len += 4;
	}
	else if( str[len] == 'f' ) {
		if( strncmp( str + len, "false", 5 ) != 0 )
			return -(len + 1);
		SetFalse();
		len += 5;
	}
	else if( str[len] == 't' ) {
		if( strncmp( str + len, "true", 4 ) != 0 )
			return -(len + 1);
		SetTrue();
		len += 4;
	}
	else if( str[len] == '-' || (str[len] >= '0' && str[len] <= '9') ) {
		int sign = 1;
		int isf = 0;
		double dt = 0.0;
		if( str[len] == '-' ) {
			sign = -1;
			len += 1;
		}
		if( str[len] == '0' ) {
			// 0 must not follow digit before '.'
			if( str[len + 1] >= '0' && str[len + 1] <= '9' )
				return -(len + 1);
		}
		while( str[len] >= '0' && str[len] <= '9' ) {
			dt = dt * 10.0 + (str[len] - '0');
			len += 1;
		}
		if( str[len] == '.' ) {
			isf = 1;
			len += 1;
			if( str[len] < '0' || str[len] > '9' )
				return -(len + 1);
			double tmp = 1.0;
			while( str[len] >= '0' && str[len] <= '9' ) {
				tmp = tmp / 10.0;
				dt = dt + (str[len] - '0') * tmp;
				len += 1;
			}
		}
		if( str[len] == 'e' || str[len] == 'E' ) {
			isf = 1;
			len += 1;
			int p_sign = 1;
			double p_exp = 0;
			if( str[len] == '+' || str[len] == '-' ) {
				len += 1;
				if( str[len] == '-' )
					p_sign = -1;
			}
			if( str[len] < '0' || str[len] > '9' )
				return -(len + 1);
			while( str[len] >= '0' && str[len] <= '9' ) {
				p_exp = p_exp * 10.0 + (str[len] - '0');
				len += 1;
			}
			p_exp *= (double)p_sign;
			dt *= pow( 10, p_exp );
		}
		// dt *= (double)sign;
		if( isf == 0 && dt <= (double)(0x7fffffff) )
			SetInt( (int)dt * sign );
		else
			SetDouble( dt * (double)sign );
	}
	else if( str[len] == '\"' ) {
		len += 1;
		// FIXME : escape char
		int n = (int)strcspn( str + len, "\"" );
		if( str[len + n] != '"' )
			return -(len + 1);
		SetString( str + len, n );
		len += n + 1;
	}
	else if( str[len] == '[' ) {
		len += 1;
		SetArray(0);
		int idx = 0;
		while( 1 ) {
			len += (int)strspn( str + len, " \t\r\n" );
			if( str[len] == '\0' )
				return -(len + 1);
			if( str[len] == ']' ) {
				len += 1;
				break;
			}
			if( idx != 0 ) {
				if( str[len] != ',' )
					return -(len + 1);
				len += 1;
				len += (int)strspn( str + len, " \t\r\n" );
			}
			JsonVal & item = GetItem( idx );
			int tmp = item.do_from_json( str + len );
			if( tmp < 0 )
				return -len + tmp; // tmp has add 1, so not need len + 1
			len += tmp;
			idx += 1;
		}
	}
	else if( str[len] == '{' ) {
		len += 1;
		
		SetNull(); // clear all object
		SetObject();
		JsonVal nm; // name
		int idx = 0;
		while( 1 ) {
			len += (int)strspn( str + len, " \t\r\n" );
			if( str[len] == '\0' )
				return -(len + 1);
			if( str[len] == '}' ) {
				len += 1;
				break;
			}
			if( idx != 0 ) {
				if( str[len] != ',' )
					return -(len + 1);
				len += 1;
				len += (int)strspn( str + len, " \t\r\n" );
			}
			if( str[len] != '"' )
				return -(len + 1);
			int tmp = nm.do_from_json( str + len );
			if( tmp < 0 )
				return -len + tmp;
			len += tmp;
			len += (int)strspn( str + len, " \t\r\n" );
			if( str[len] != ':' )
				return -(len + 1);
			len += 1;
			len += (int)strspn( str + len, " \t\r\n" );
			JsonVal & item = GetItem( nm.GetString() );
			tmp = item.do_from_json( str + len );
			if( tmp < 0 )
				return -len + tmp;
			len += tmp;
			idx += 1;
		}
	}
	else if( str[len] != '\0' ) {
		len = -(len + 1);
	}
	return len;
}

//////////////////////////////////
END_NS_DGN

