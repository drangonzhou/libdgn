// Time.h : drangon time
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

#ifndef INCLUDED_DGN_TIME_H
#define INCLUDED_DGN_TIME_H

#include <dgn/dgn.h>
#include <dgn/CStr.h>

#ifdef _MSC_VER
#pragma comment( lib, "winmm" )
#endif

BEGIN_NS_DGN
////////////////

class CStr;

class DGN_LIB_API Time
{
public:
	static int64_t Now();  // us
	static uint32_t Tick(); // ms
	static long NowSec();  // sec
	static long NowSecEx() {	long now = NowSec(); s_NowSecEx = now; return now; } // sec, update cache time
	static long NowSecFast() {	return s_NowSecEx; }  // sec, fast, use cache time
	static void SleepMs( int ms );

private:
	static long s_NowSecEx;  // cached time value

public:
	int16_t m_year;
	int8_t m_month;
	int8_t m_day;

	int8_t m_hour;
	int8_t m_minute;
	int8_t m_sec;
	int8_t m_is_gmt; // 0 : localtime (default), 1 : gmt

	int32_t m_usec;

public:
	Time( int year = 0, int month = 0, int day = 0, int hour = 0, 
			int minute = 0, int sec = 0, int usec = 0, int is_gmt = 0 ) 
		: m_year(year), m_month(month), m_day(day), m_hour(hour), 
			m_minute(minute), m_sec(sec), m_is_gmt(is_gmt), m_usec(usec) {}
	Time( const Time & ts ) : m_year(ts.m_year), m_month(ts.m_month), 
		m_day(ts.m_day), m_hour(ts.m_hour), m_minute(ts.m_minute), 
		m_sec(ts.m_sec), m_is_gmt(ts.m_is_gmt), m_usec(ts.m_usec) {}

	Time & operator = ( const Time & ts ) { m_year = ts.m_year; 
		m_month = ts.m_month; m_day = ts.m_day; m_hour = ts.m_hour; 
		m_minute = ts.m_minute; m_sec = ts.m_sec; m_is_gmt = ts.m_is_gmt; 
		m_usec = ts.m_usec; return *this; }

	void SetNow();
	void SetNow_GMT();
	void FromTUS( int64_t tus );
	void FromTUS_GMT( int64_t tus );
	int64_t ToTUS() const;
	void SetGMT( int gmt = 1 ); // 1 : change to gmt, 0 : change to localtime

	// parse RFC1123 : Tue, 15 Jan 2013 21:47:38 GMT
	// RFC822 : Tue, 15 Jan 13 21:47:38 GMT
	// RFC850 : Tuesday, 15-Jan-13 21:47:38 GMT
	// ASCTIME : Tue Jan 15 21:47:38 2013
	int FromRfc( const char * str );
	// generate RFC1123 : Tue, 15 Jan 2013 21:47:38 GMT, append to str
	int ToRfc( CStr * ret ) const;
	CStr ToRfc() const;

	friend DGN_LIB_API bool operator < ( const Time & a, const Time & b );
	friend DGN_LIB_API bool operator <= ( const Time & a, const Time & b );
	friend DGN_LIB_API bool operator > ( const Time & a, const Time & b );
	friend DGN_LIB_API bool operator >= ( const Time & a, const Time & b );
	friend DGN_LIB_API bool operator == ( const Time & a, const Time & b );
	friend DGN_LIB_API int64_t operator - ( const Time & a, const Time & b );

	static int IsLeapYear( int year );
	static int ToYday( int year, int month, int day );
	static int FromYday( int year, int yday, int * month, int * day );
	static int GetWeekDay( int year, int month, int day );
	static int IsWeekend( int year, int month, int day );

	static int ParseWeekday( const char * weekday, int n = -1 );
	static int ParseMonth( const char * month, int n = -1 );
	static const char * GetWeekdayName( int weekday, int is_full = 0 ); // 0~6
	static const char * GetMonthName( int month ); // 1~12
};

DGN_LIB_API inline bool operator < ( const Time & a, const Time & b )
{
	if( a.m_year < b.m_year ) return true;
	else if( a.m_year > b.m_year ) return false;
	else if( a.m_month < b.m_month ) return true;
	else if( a.m_month > b.m_month ) return false;
	else if( a.m_day < b.m_day ) return true;
	else if( a.m_day > b.m_day ) return false;
	else if( a.m_hour < b.m_hour ) return true;
	else if( a.m_hour > b.m_hour ) return false;
	else if( a.m_minute < b.m_minute ) return true;
	else if( a.m_minute > b.m_minute ) return false;
	else if( a.m_sec < b.m_sec ) return true;
	else if( a.m_sec > b.m_sec ) return false;
	else if( a.m_usec < b.m_usec ) return true;
	else return false;
}

DGN_LIB_API inline bool operator <= ( const Time & a, const Time & b )
{
	if( a.m_year < b.m_year ) return true;
	else if( a.m_year > b.m_year ) return false;
	else if( a.m_month < b.m_month ) return true;
	else if( a.m_month > b.m_month ) return false;
	else if( a.m_day < b.m_day ) return true;
	else if( a.m_day > b.m_day ) return false;
	else if( a.m_hour < b.m_hour ) return true;
	else if( a.m_hour > b.m_hour ) return false;
	else if( a.m_minute < b.m_minute ) return true;
	else if( a.m_minute > b.m_minute ) return false;
	else if( a.m_sec < b.m_sec ) return true;
	else if( a.m_sec > b.m_sec ) return false;
	else if( a.m_usec <= b.m_usec ) return true;
	else return false;
}

DGN_LIB_API inline bool operator > ( const Time & a, const Time & b )
{
	return b < a;
}

DGN_LIB_API inline bool operator >= ( const Time & a, const Time & b )
{
	return b <= a;
}

DGN_LIB_API inline bool operator == ( const Time & a, const Time & b )
{
	return ( a.m_year == b.m_year && a.m_month == b.m_month && a.m_day == b.m_day && a.m_hour == b.m_hour && a.m_minute == b.m_minute && a.m_sec == b.m_sec && a.m_usec == b.m_usec );
}

DGN_LIB_API inline int64_t operator - ( const Time & a, const Time & b )
{
	return a.ToTUS() - b.ToTUS();
}

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_TIME_H

