// File.cpp : File Operation
// Copyright (C) 2012 ~ 2020 drangon zhou <drangon.zhou (at) gmail.com>
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

#define _FILE_OFFSET_BITS 64
#include <dgn/File.h>
#include <dgn/Logger.h>

#ifdef _WIN32
#include <windows.h>
#define INVALID_FD	INVALID_HANDLE_VALUE
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/file.h>  // flock
#define INVALID_FD	(-1)
#endif

BEGIN_NS_DGN
////////////////
File::File() : m_fd( INVALID_FD )
{
}

File::~File()
{
	Close();
}

int File::Unlink( const char * name )
{
#ifdef _WIN32
	if( DeleteFile( name ) )
		return 0;
	return -1;
#else
	return unlink( name );
#endif
}

int File::Truncate( const char * name, int64_t off )
{
	File f;
	if( f.Open( name, DGN_OPEN_WRITE ) < 0 )
		return -1;
	return f.Truncate( off );
}

int File::Open( const char * name, enum dgn_file_open_flag_e create_flag )
{
	if( name == NULL || name[0] == '\0' )
		return -1;

	Close();

#ifdef _WIN32
	DWORD faccess, fcreate;
	if( create_flag == DGN_OPEN_CREATE_EXCL ) {
		faccess = GENERIC_READ | GENERIC_WRITE;
		fcreate = CREATE_NEW;
	}
	else if( create_flag == DGN_OPEN_CREATE ) {
		faccess = GENERIC_READ | GENERIC_WRITE;
		fcreate = OPEN_ALWAYS;
	}
	else if( create_flag == DGN_OPEN_WRITE ) {
		faccess = GENERIC_READ | GENERIC_WRITE;
		fcreate = OPEN_EXISTING;
	}
	else {
		faccess = GENERIC_READ;
		fcreate = OPEN_EXISTING;
	}

	HANDLE fd = CreateFile( name, faccess, 0, NULL, fcreate, FILE_ATTRIBUTE_NORMAL, NULL );
	if( fd == INVALID_HANDLE_VALUE )
		return -1;
#else
	int flag;
	if( create_flag == DGN_OPEN_CREATE_EXCL )
		flag = O_RDWR | O_CREAT | O_EXCL;
	else if( create_flag == DGN_OPEN_CREATE )
		flag = O_RDWR | O_CREAT;
	else if( create_flag == DGN_OPEN_WRITE )
		flag = O_RDWR;
	else
		flag = O_RDONLY;

	int fd = open( name, flag, 0660 );
	if( fd < 0 )
		return -1;
#endif
	m_fd = fd;
	return 0;
}

bool File::IsOpened() const
{
	return m_fd != INVALID_FD;
}

int64_t File::Seek( int64_t off, enum dgn_file_seek_e where )
{
	if( m_fd == INVALID_FD ) {
		PR_DEBUG( "m_fd is invalid" );
		return -1;
	}
#ifdef _WIN32
	int mywhere = FILE_BEGIN;
	if( where == DGN_SEEK_SET )
		mywhere = FILE_BEGIN;
	else if( where == DGN_SEEK_CUR )
		mywhere = FILE_CURRENT;
	else if( where == DGN_SEEK_END )
		mywhere = FILE_END;
	LARGE_INTEGER liin, liout;
	liin.QuadPart = off;
	liout.QuadPart = -1;
	if( SetFilePointerEx( m_fd, liin, &liout, mywhere ) == 0 )
		return -1;
	return liout.QuadPart;
#else
	int mywhere = SEEK_SET;
	if( where == DGN_SEEK_SET )
		mywhere = SEEK_SET;
	else if( where == DGN_SEEK_CUR )
		mywhere = SEEK_CUR;
	else if( where == DGN_SEEK_END )
		mywhere = SEEK_END;
	return lseek( m_fd, off, mywhere );
#endif
}

int64_t File::Size()
{
	if( m_fd == INVALID_FD ) {
		PR_DEBUG( "m_fd is invalid" );
		return -1;
	}
#ifdef _WIN32
	LARGE_INTEGER liout;
	liout.QuadPart = -1;
	if( GetFileSizeEx( m_fd, &liout ) == 0 )
		return -1;
	return liout.QuadPart;
#else
	int64_t curr = lseek( m_fd, 0, SEEK_CUR );
	int64_t sz = lseek( m_fd, 0, SEEK_END );
	lseek( m_fd, curr, SEEK_SET );
	return sz;
#endif
}

int File::Read( char * buf, int len )
{
	if( m_fd == INVALID_FD ) {
		PR_DEBUG( "m_fd is invalid" );
		return -1;
	}
#ifdef _WIN32
	DWORD retlen = -1;
	ReadFile( m_fd, buf, len, &retlen, NULL );
	return retlen;
#else
	return read( m_fd, buf, len );
#endif
}

int File::Write( const char * buf, int len )
{
	if( m_fd == INVALID_FD ) {
		PR_DEBUG( "m_fd is invalid" );
		return -1;
	}
#ifdef _WIN32
	DWORD retlen = -1;
	if( WriteFile( m_fd, buf, len, &retlen, NULL ) == 0 )
		return -1;
	return retlen;
#else
	return write( m_fd, buf, len );
#endif
}

int File::Truncate( int64_t off )
{
	if( m_fd == INVALID_FD ) {
		PR_DEBUG( "m_fd is invalid" );
		return -1;
	}
#ifdef _WIN32
	Seek( off, DGN_SEEK_SET );
	SetEndOfFile( m_fd );
#else
	if( ftruncate( m_fd, off ) < 0 )
		return -1;
	Seek( off, DGN_SEEK_SET );
#endif
	return 0;
}

int File::Close()
{
	if( m_fd == INVALID_FD )
		return 0;
#ifdef _WIN32
	CloseHandle( m_fd );
#else
	close( m_fd );
#endif
	m_fd = INVALID_FD;
	return 0;
}

int File::Lock()
{
	if( m_fd == INVALID_FD ) {
		PR_DEBUG( "m_fd is invalid" );
		return -1;
	}
#ifdef _WIN32
	OVERLAPPED ol;
	memset( &ol, 0, sizeof(ol) );
	BOOL ret = LockFileEx( m_fd, LOCKFILE_EXCLUSIVE_LOCK, 0, 0x7FFFFFFF, 0, &ol );
	return ( ret ) ? 0 : -1;
#else
	return flock( m_fd, LOCK_EX );
#endif
}

int File::Unlock()
{
	if( m_fd == INVALID_FD ) {
		PR_DEBUG( "m_fd is invalid" );
		return -1;
	}
#ifdef _WIN32
	OVERLAPPED ol;
	memset( &ol, 0, sizeof(ol) );
	BOOL ret = UnlockFileEx( m_fd, 0, 0x7FFFFFFF, 0, &ol );
	return ( ret ) ? 0 : -1;
#else
	return flock( m_fd, LOCK_UN );
#endif

}

////////////////
END_NS_DGN

