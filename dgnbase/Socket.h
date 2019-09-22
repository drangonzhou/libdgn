// Socket.h : drangon socket
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

#ifndef INCLUDED_DGN_SOCKET_H
#define INCLUDED_DGN_SOCKET_H

#include <dgn/CStr.h>

#ifdef _MSC_VER
#pragma comment( lib, "ws2_32" )
#endif

struct sockaddr_storage;

BEGIN_NS_DGN
////////////////

enum socket_connect_result_e {
	DGN_SOCKET_CONNECT_UNKNOWN = 0,  // should not exist
	DGN_SOCKET_CONNECT_TRYING,
	DGN_SOCKET_CONNECT_FAILED,
	DGN_SOCKET_CONNECT_OK,
};

enum {
	DGN_POLLIN = 1,
	DGN_POLLOUT = 2,
};

#define DGN_IP_LEN	48    // ipv6 46 / ipv4 16 (with '\0'), use 48 for align

#ifdef _WIN64
typedef unsigned __int64 sock_t;
#elif defined( _WIN32 )
typedef unsigned int sock_t;
#else
typedef int sock_t;
#endif
#define DGN_INVALID_SOCK ((sock_t)-1)

class Socket
{
public:
	static int GetHostAddr( const char * host, int port, struct sockaddr_storage * addr, int * addrlen );
	static int GetAddrIp( const struct sockaddr_storage * addr, int addrlen, char ip[DGN_IP_LEN], int * port );
	static char * GetAddrIp( uint32_t ip_ne, char ip[DGN_IP_LEN] );
	static char * Resolve( const char * host, char ip[DGN_IP_LEN] );
	static CStr Resolve( const char * host );
	static int SetNonBlock( sock_t sock, int nonblock );

public:
	Socket();
	virtual ~Socket();

public:
	int SetTimeout( int timeout_ms ) { m_timeout_ms = timeout_ms; return 0; }
	int GetTimeout() const { return m_timeout_ms; }

	enum socket_connect_result_e Connect( const char * host, int port );
	enum socket_connect_result_e ConnectCheck(); // return at once, no timeout

	int TcpSvr( const char * host, int port );
	Socket * Accept(); // ret new client Socket that need delete

	int UdpSvr( const char * host, int port );

	sock_t GetRawSock() const { return m_sock; } // still own by me
	int AttachSock( sock_t sk ); // old sock closed, new sock own by me
	sock_t DetachSock(); // detached sock need close by outside

	bool IsValid() const { return m_sock != DGN_INVALID_SOCK; }
	int Close();

public:
	int LocalAddr( char ip[DGN_IP_LEN], int * port );
	int LocalAddr( CStr * ip, int * port );
	int RemoteAddr( char ip[DGN_IP_LEN], int * port );
	int RemoteAddr( CStr * ip, int * port );

	// for send/recv, return >0 for data processed, = 0 when no data and timeout
	// return < 0 if no data and error happend, include peer reset
	int Send( const char * buf, int len );
	int Recv( char * buf, int minlen, int maxlen );
	int SendTo( const char * buf, int len, const char * remote_host, int port );
	int RecvFrom( char * buf, int len, char remote_ip[DGN_IP_LEN], int * port );

	int Poll( int want_evt, int * ret_evt, int check_timeout_interval_ms = 250 );
	static int Pollex( int num, Socket ** skarr, const int * want_evt, int * ret_evt, volatile int * timeout_ms, int check_timeout_interval_ms = 250 );

protected:
	sock_t m_sock;
	volatile int m_timeout_ms; // default 0
};

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_SOCKET_H

