// Time.cpp : drangon time
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

#include <dgn/Time.h>

#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <memory.h>
#endif

#ifdef _WIN32
// extern long _timezone;
#define g_timezone _timezone
#define g_tzset _tzset
#else
// extern long timezone;
#define g_timezone timezone
#define g_tzset tzset
#endif

BEGIN_NS_DGN
////////////////

int64_t Time::Now()
{
#ifdef _WIN32
	static const int64_t s_FILETIME_1970 = 11644473600LL; // seconds between 1601-01-01 and 1970-01-01
	ULARGE_INTEGER fti;
	GetSystemTimeAsFileTime ((LPFILETIME) &fti); // 100-nanoseconds since 1601-01-01
	// The actual accuracy on XP seems to be 125,000 nanoseconds = 125 microseconds = 0.125 milliseconds
	return fti.QuadPart / 10 - ( s_FILETIME_1970 * 1000000LL );
#else
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (int64_t)tv.tv_sec * 1000000LL + tv.tv_usec;
#endif
}

uint32_t Time::Tick()
{
#ifdef _WIN32
	return timeGetTime();
#else
	struct timespec tms = { 0, 0 };
	clock_gettime( CLOCK_MONOTONIC, &tms );
	// PR_DEBUG( "tick : %d, %d", (int)tms.tv_sec, (int)tms.tv_nsec );
	return (uint32_t)( (uint32_t)tms.tv_sec * 1000 + (uint32_t)tms.tv_nsec / 1000000 );
#endif
}

void Time::SleepMs( int ms )
{
#ifdef _WIN32
	::Sleep( ms );

#else
	struct timeval tv;
	tv.tv_sec = ms / 1000;
	tv.tv_usec = ( ms % 1000 ) * 1000;
	// TODO : will EINTR affect ?
	select( 0, NULL, NULL, NULL, &tv );
#endif
	return;
}

void Time::SetNow()
{
#ifdef _WIN32
	// TODO : another way
	FromTUS( Time::Now() );
#else
	FromTUS( Time::Now() );
#endif
	return;
}

void Time::SetNow_GMT()
{
	FromTUS_GMT( Time::Now() );
	return;
}

void Time::FromTUS( int64_t tus )
{
	time_t tt = tus / 1000000LL;
	struct tm * mytm;
#ifdef _WIN32
	mytm = localtime( &tt );
#else
	struct tm tmp_tm;
	localtime_r( &tt, &tmp_tm );
	mytm = &tmp_tm;
#endif
	m_year = mytm->tm_year + 1900;
	m_month = mytm->tm_mon + 1;
	m_day = mytm->tm_mday;
	m_hour = mytm->tm_hour;
	m_minute = mytm->tm_min;
	m_sec = mytm->tm_sec;
	m_usec = tus % 1000000LL;
	m_is_gmt = 0;
	return;
}

void Time::FromTUS_GMT( int64_t tus )
{
	time_t tt = tus / 1000000LL;
	struct tm * mytm;
#ifdef _WIN32
	mytm = gmtime( &tt );
#else
	struct tm tmp_tm;
	gmtime_r( &tt, &tmp_tm );
	mytm = &tmp_tm;
#endif
	m_year = mytm->tm_year + 1900;
	m_month = mytm->tm_mon + 1;
	m_day = mytm->tm_mday;
	m_hour = mytm->tm_hour;
	m_minute = mytm->tm_min;
	m_sec = mytm->tm_sec;
	m_usec = tus % 1000000LL;
	m_is_gmt = 1;
	return;
}

int64_t Time::ToTUS() const
{
	struct tm mytm;
	memset( &mytm, 0, sizeof(mytm) );
	mytm.tm_year = m_year - 1900;
	mytm.tm_mon = m_month - 1;
	mytm.tm_mday = m_day;
	mytm.tm_hour = m_hour;
	mytm.tm_min = m_minute;
	mytm.tm_sec = m_sec;
	time_t tt = mktime( &mytm );
	int64_t tz_adjust = 0;
	if( m_is_gmt ) {
		g_tzset();
		tz_adjust = 0 - g_timezone;
	}
	return ((int64_t)tt + tz_adjust ) * 1000000LL + m_usec;
}

void Time::SetGMT( int gmt /*= 1*/ )
{
	if( gmt != 0 && m_is_gmt == 0 ) {
		FromTUS_GMT( ToTUS() );
	}
	else if( gmt == 0 && m_is_gmt != 0 ) {
		FromTUS( ToTUS() );
	}
	return;
}

int Time::IsLeapYear( int year )
{
	int is_leapyear = 0;
	if( year % 100 == 0 ) {
		if( year % 400 == 0 )
			is_leapyear = 1;
	}
	else if( year % 4 == 0 ) {
		is_leapyear = 1;
	}
	return is_leapyear;
}

static const int s_mday[12] = { 31, 29, 31, 30, 31, 30, 
		31, 31, 30, 31, 30, 31 };

int Time::ToYday( int year, int month, int day )
{
	int is_leapyear = Time::IsLeapYear( year );
	if( month < 1 || month > 12 )
		return -1;
	if( day < 1 || day > s_mday[month - 1] )
		return -1;
	if( month == 2 && day == 29 && is_leapyear == 0 )
		return -1;

	int i;
	int yday = 0;
	for( i = 0; i + 1 < month; i++ )
		yday += s_mday[i];
	if( month > 2 )
		yday += is_leapyear - 1;
	yday += day;
	return yday;
}

int Time::FromYday( int year, int yday, int * month, int * day )
{
	int is_leapyear = Time::IsLeapYear( year );
	if( yday < 1 || yday > 365 + is_leapyear )
		return -1;
	*month = 1;
	while( yday > s_mday[ *month - 1 ] ) {
		yday -= s_mday[*month - 1];
		if( *month == 2 )
			yday += 1 - is_leapyear;
		*month += 1;
	}
	*day = yday;
	if( *month == 2 && *day == 29 && is_leapyear == 0 ) {
		*month = 3;
		*day = 1;
	}
	return 0;
}

int Time::GetWeekDay( int year, int month, int day )
{
	// Zeller fomular : w=y+[y/4]+[c/4]-2c+[26(m+1)/10]+d-1
	int y = year;
	int m = month;
	if( m <= 2 ) {
		y = y - 1;
		m = m + 12;
	}
	int w = (y % 100) + (y % 100) / 4 + (y / 100) / 4 
			- 2 * (y / 100) + 26 * (m + 1) / 10 + day - 1;
	w = (w + 70 ) % 7;
	return w;
}

int Time::IsWeekend( int year, int month, int day )
{
	int w = Time::GetWeekDay( year, month, day );
	return (w == 0 || w == 6 ) ? 1 : 0;
}

const static char * s_weekday_name[7] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const static char * s_weekday_fullname[7] = { "Sunday", "Monday", 
	"Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
const static char * s_month_name[13] = { "", "Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

int Time::ParseWeekday( const char * weekday, int n )
{
	int i;
	for( i = 0; i < 7; ++i ) {
		if( CStr::CaseCmp( weekday, s_weekday_name[i], n ) == 0 
				|| CStr::CaseCmp( weekday, s_weekday_fullname[i], n ) == 0 )
		{
			return i;
		}
	}
	return -1;
}

int Time::ParseMonth( const char * month, int n )
{
	int i;
	for( i = 1; i <= 12; ++i ) {
		if( CStr::CaseCmp( month, s_month_name[i], n ) == 0 ) {
			return i;
		}
	}
	return -1;
}

const char * Time::GetWeekdayName( int weekday, int is_full /*= 0*/ ) // 0~6
{
	if( weekday < 0 || weekday > 6 )
		return "(unknown)";
	if( is_full )
		return s_weekday_fullname[weekday];
	return s_weekday_name[weekday];
}

const char * Time::GetMonthName( int month ) // 1~12
{
	if( month < 1 || month > 12 )
		return "(unknown)";
	return s_month_name[month];
}

int Time::FromRfc( const char * str )
{
	const char * p = str;
	int n = 0;
	n = strcspn( p, " ," );
	if( p[n] == '\0' )
		return -1;

	// only test valid, not test correct
	int weekday = ParseWeekday( p, n );
	if( weekday < 0 )
		return -1;

	// ASCTIME : Tue Jan 15 21:47:38 2013
	if( p[n] == ' ' ) {
		p += n + 1;
		p += strspn( p, " " );

		int tmp = ParseMonth( p, 3 );
		if( tmp < 0 )
			return -1;
		m_month = tmp;
		if( p[3] != ' ' )
			return -1;
		p += 3;
		p += strspn( p, " " );

		tmp = CStr::ToInt( p );
		if( tmp <= 0 || tmp > 31 )
			return -1;
		m_day = tmp;
		p += strcspn( p, " " );
		p += strspn( p, " " );

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 23 )
			return -1;
		m_hour = tmp;
		p += strcspn( p, ":" );
		if( *p != ':' )
			return -1;
		p += 1;

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 59 )
			return -1;
		m_minute = tmp;
		p += strcspn( p, ":" );
		if( *p != ':' )
			return -1;
		p += 1;

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 59 )
			return -1;
		m_sec = tmp;
		p += strcspn( p, " " );
		if( *p != ' ' )
			return -1;
		p += strspn( p, " " );

		tmp = CStr::ToInt( p );
		if( tmp <= 0 )
			return -1;
		m_year = tmp;

		m_is_gmt = 0;
		m_usec = 0;
	}

	// RFC850 : Tuesday, 15-Jan-13 21:47:38 GMT
	else if( n > 3 ) {
		p += n + 1;
		p += strspn( p, " " );

		int tmp = CStr::ToInt( p );
		if( tmp <= 0 || tmp > 31 )
			return -1;
		m_day = tmp;
		p += strcspn( p, "-" );
		if( *p != '-' )
			return -1;
		p += 1;

		tmp = ParseMonth( p, 3 );
		if( tmp < 0 )
			return -1;
		m_month = tmp;
		if( p[3] != '-' )
			return -1;
		p += 4;

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 3000 )
			return -1;
		if( tmp < 70 ) {
			m_year = tmp + 2000;
		}
		else if( tmp >= 70 && tmp <= 99 ) {
			m_year = tmp + 1900;
		}
		else {
			// 4 digit, should not use
			m_year = tmp;
		}
		p += strcspn( p, " " );
		p += strspn( p, " " );

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 23 )
			return -1;
		m_hour = tmp;
		p += strcspn( p, ":" );
		if( *p != ':' )
			return -1;
		p += 1;

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 59 )
			return -1;
		m_minute = tmp;
		p += strcspn( p, ":" );
		if( *p != ':' )
			return -1;
		p += 1;

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 59 )
			return -1;
		m_sec = tmp;
		p += strcspn( p, " " );
		if( *p != ' ' )
			return -1;
		p += strspn( p, " " );

		// TODO : assume "GMT", other timezone need support

		m_is_gmt = 1;
		m_usec = 0;
	}

	// RFC1123 : Tue, 15 Jan 2013 21:47:38 GMT
	// RFC822 : Tue, 15 Jan 13 21:47:38 GMT
	else {
		p += n + 1;
		p += strspn( p, " " );

		int tmp = CStr::ToInt( p );
		if( tmp <= 0 || tmp > 31 )
			return -1;
		m_day = tmp;
		p += strcspn( p, " " );
		if( *p != ' ' )
			return -1;
		p += strspn( p, " " );

		tmp = ParseMonth( p, 3 );
		if( tmp < 0 )
			return -1;
		m_month = tmp;
		if( p[3] != ' ' )
			return -1;
		p += 3;
		p += strspn( p, " " );

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 3000 )
			return -1;
		if( tmp < 70 ) {
			m_year = tmp + 2000;
		}
		else if( tmp >= 70 && tmp <= 99 ) {
			m_year = tmp + 1900;
		}
		else {
			// 4 digit, should not use
			m_year = tmp;
		}
		p += strcspn( p, " " );
		p += strspn( p, " " );

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 23 )
			return -1;
		m_hour = tmp;
		p += strcspn( p, ":" );
		if( *p != ':' )
			return -1;
		p += 1;

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 59 )
			return -1;
		m_minute = tmp;
		p += strcspn( p, ":" );
		if( *p != ':' )
			return -1;
		p += 1;

		tmp = CStr::ToInt( p );
		if( tmp < 0 || tmp > 59 )
			return -1;
		m_sec = tmp;
		p += strcspn( p, " " );
		if( *p != ' ' )
			return -1;
		p += strspn( p, " " );

		// TODO : assume "GMT", other timezone need support

		m_is_gmt = 1;
		m_usec = 0;
	}

	return 0;
}

int Time::ToRfc(  CStr * ret ) const
{
	Time tt;
	const Time * pt = this;
	if( m_is_gmt == 0 ) {
		tt.FromTUS_GMT( ToTUS() );
		pt = &tt;
	}
	ret->Reserve( ret->Len() + 32 );
	ret->AppendFmt( "%s, %2d %s %4d %02d:%02d:%02d GMT", 
			GetWeekdayName( GetWeekDay( pt->m_year, pt->m_month, pt->m_day ) ),
			pt->m_day, GetMonthName( pt->m_month ), pt->m_year, 
			pt->m_hour, pt->m_minute, pt->m_sec );
	return 0;
}

CStr Time::ToRfc() const
{
	CStr str;
	ToRfc( &str );
	return str;
}

////////////////
END_NS_DGN

