// File.h : File Operation
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

#ifndef INCLUDED_DGN_FILE_H
#define INCLUDED_DGN_FILE_H

#include <dgn/dgn.h>

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

BEGIN_NS_DGN
////////////////

enum dgn_file_open_flag_e {
	DGN_OPEN_DEFAULT = 0,  // default : open exist, read only
	DGN_OPEN_READ = 0,
	DGN_OPEN_WRITE = 1, // open exist, read/write
	DGN_OPEN_CREATE = 2,     // open exist or create new, read/write
	DGN_OPEN_CREATE_EXCL = 3, // like DGN_O_CREATE, but failed if already exist
};

enum dgn_file_seek_e {
	DGN_SEEK_SET,   // set offset
	DGN_SEEK_CUR,   // offset relate to current position
	DGN_SEEK_END,   // offset relate to file end ( usually neg )
};

class DGN_LIB_API File
{
public:
	File();
	~File();

	static int Unlink( const char * name );
	static int Truncate( const char * name, int64_t off );

	int Open( const char * name, enum dgn_file_open_flag_e create_flag = DGN_OPEN_CREATE );
	bool IsOpened() const;
	// return -1 if failed
	int64_t Seek( int64_t off, enum dgn_file_seek_e where = DGN_SEEK_SET );
	int64_t Tell() { return Seek( 0, DGN_SEEK_CUR ); }
	int64_t Size();
	int Read( char * buf, int len );
	int Write( const char * buf, int len );
	int Truncate( int64_t off );
	int Close();

	int Lock();
	int Unlock();

private:
#ifdef _WIN32
	HANDLE m_fd;
#else
	int m_fd;
#endif
};

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_FILE_H

