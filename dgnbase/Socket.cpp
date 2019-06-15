// Socket.cpp : drangon socket
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

#ifdef _WIN32
#define FD_SETSIZE	1024
#include <winsock2.h>   // must include at first 
#endif

#include <dgn/Socket.h>
#include <dgn/Time.h>
#include <dgn/Logger.h>

#ifdef _WIN32
#include <windows.h>
#include <ws2tcpip.h>
#define socklen_t int
#define DGN_SHUT_RDWR	SD_BOTH
#define IS_ERR_EAGAIN()	( WSAGetLastError() == WSAEWOULDBLOCK )
#define GET_ERRNO() ( WSAGetLastError() )
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#define SOCKET int
#define closesocket close
#define INVALID_SOCKET (-1)
#define DGN_SHUT_RDWR   SHUT_RDWR
#define IS_ERR_EAGAIN() ( errno == EAGAIN || errno == EINTR )
#define GET_ERRNO() ( errno )
#endif


BEGIN_NS_DGN
////////////////

int Socket::GetHostAddr( const char * host, int port, struct sockaddr_storage * addr, int * addrlen )
{
	if( host == NULL || host[0] == '\0' )
		return -1;
		
#if 0 
	// mingw32 does not have getaddrinfo(), but vs2005 have
	struct hostent * hptr = gethostbyname( host );
	if( hptr == NULL || hptr->h_addr_list == NULL || hptr->h_addr_list[0] == NULL )
		return -1;
	memcpy( &(addr->sin_addr), hptr->h_addr_list[0], sizeof(addr->sin_addr) );
#endif // if 0

	struct addrinfo hint;
	struct addrinfo * res = NULL;
	memset( &hint, 0, sizeof(hint) );
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM; // enough, useable for DGRAM too
	hint.ai_protocol = IPPROTO_TCP;
	int ret = getaddrinfo( host, NULL, &hint, &res );
	if( ret != 0 || res == NULL )
		return -1;
	// only use the first resolve result
	*addrlen = res->ai_addrlen;
	memcpy( addr, res->ai_addr, res->ai_addrlen );
	if( res != NULL )
		freeaddrinfo( res ), res = NULL;
	if( addr->ss_family == AF_INET )
		((struct sockaddr_in *)addr)->sin_port = htons( port );
	else if( addr->ss_family == AF_INET6 )
		((struct sockaddr_in6 *)addr)->sin6_port = htons( port );
	return 0;
}

int Socket::GetAddrIp( const struct sockaddr_storage * addr, int addrlen, char ip[DGN_IP_LEN], int * port )
{
	if( addr == NULL )
		return -1;
	
#if 0
#ifdef _WIN32
	char * tmp = inet_ntoa( addr->sin_addr );
	if( tmp == NULL )
		return -1;
	strcpy( ip, tmp ); // NOTE : ip buf should big enough
#else
	inet_ntop( AF_INET, (void *)&(addr->sin_addr), ip, INET_ADDRSTRLEN );
#endif
#endif // if 0

	ip[0] = '\0';
	if( getnameinfo( (const struct sockaddr *)addr, addrlen, ip, DGN_IP_LEN, NULL, 0, NI_NUMERICHOST ) != 0 )
		return -1;
	if( port != NULL ) {
		if( addr->ss_family == AF_INET )
			*port = ntohs( ((const struct sockaddr_in *)addr)->sin_port );
		else if( addr->ss_family == AF_INET6 )
			*port = ntohs( ((const struct sockaddr_in6 *)addr)->sin6_port );
	}
	return 0;
}

char * Socket::Resolve( const char * host, char ip[DGN_IP_LEN] )
{
	int addrlen = 0;
	struct sockaddr_storage addr;
	if( GetHostAddr( host, 0, &addr, &addrlen ) < 0 ) {
		ip[0] = '\0';
		return ip;
	}
	if( GetAddrIp( &addr, addrlen, ip, NULL ) < 0 ) {
		ip[0] = '\0';
	}
	return ip;
}

CStr Socket::Resolve( const char * host )
{
	CStr str;
	str.Reserve( DGN_IP_LEN );
	char * p = str.GetRaw();
	p[DGN_IP_LEN - 1] = '\0';
	Resolve( host, p );
	str.ReleaseRaw( strlen(p) );
	return str;
}

int Socket::SetNonBlock( sock_t sk, int nonblock )
{
#ifdef _WIN32
	int ret;
	unsigned long nb = nonblock ? 1 : 0;
	ret = ioctlsocket( sk, FIONBIO, &nb );
	return ret == SOCKET_ERROR ? -1 : 0;
#else
	int ret, flags;
	flags = fcntl( sk, F_GETFL );
	if( flags < 0 ) {
		return -1;
	}
	if( nonblock ) 
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	ret = fcntl( sk, F_SETFL, flags );
	return ret < 0 ? -1 : 0;
#endif
}

Socket::Socket() : m_sock( INVALID_SOCKET ), m_timeout_ms( 0 )
{

}

Socket::~Socket()
{
	Close();
}

int Socket::Close()
{
	if( m_sock != INVALID_SOCKET ) {
		shutdown( m_sock, DGN_SHUT_RDWR );
		closesocket( m_sock );
		m_sock = INVALID_SOCKET;
	}
	return 0;
}

enum socket_connect_result_e Socket::Connect( const char * host, int port )
{
	int addrlen = 0;
	struct sockaddr_storage addr;
	if( GetHostAddr( host, port, &addr, &addrlen ) < 0 ) {
		PR_DEBUG( "resolve [%s] failed", host );
		return DGN_SOCKET_CONNECT_FAILED;
	}

	Close();

	m_sock = socket( AF_INET, SOCK_STREAM, 0 );
	if( m_sock == INVALID_SOCKET ) {
		PR_DEBUG( "socket() failed" );
		return DGN_SOCKET_CONNECT_FAILED;
	}
	if( SetNonBlock( m_sock, 1 ) < 0 ) {
		PR_DEBUG( "SetNonBlock() failed" );
		Close();
		return DGN_SOCKET_CONNECT_FAILED;
	}

	int ret = connect( m_sock, (struct sockaddr *)&addr, addrlen );
	if( ret >= 0 ) {
		return DGN_SOCKET_CONNECT_OK;
	}

#ifdef _WIN32
	if( ret < 0 && WSAGetLastError() != WSAEWOULDBLOCK )
#else
	if( ret < 0 && errno != EINPROGRESS )
#endif
	{
		PR_DEBUG( "connect failed, err %d", GET_ERRNO() );
		Close();
		return DGN_SOCKET_CONNECT_FAILED;
	}

	if( m_timeout_ms <= 0 )
		return DGN_SOCKET_CONNECT_TRYING; // sock still in progress

	int ret_evt;
	ret = Poll( DGN_POLLOUT, &ret_evt );
	if( ret <= 0 ) {
		// PR_DEBUG( "connect poll timeout" );
		return DGN_SOCKET_CONNECT_TRYING;
	}

	return ConnectCheck();
}

enum socket_connect_result_e Socket::ConnectCheck()
{
	if( m_sock == INVALID_SOCKET ) {
		return DGN_SOCKET_CONNECT_FAILED;
	}

#ifdef _WIN32
	fd_set rset, wset, eset;
	struct timeval tv;

	FD_ZERO( &rset );
	FD_ZERO( &wset );
	FD_ZERO( &eset );
	FD_SET( m_sock, &wset );
	FD_SET( m_sock, &eset );
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int ret = select( 0, &rset, &wset, &eset, &tv );
	if( ret > 0 && FD_ISSET( m_sock, &wset ) )
		return DGN_SOCKET_CONNECT_OK; // connected
	if( ret > 0 && FD_ISSET( m_sock, &eset ) )
		return DGN_SOCKET_CONNECT_FAILED; // connect failed
	return DGN_SOCKET_CONNECT_TRYING; // still in progress

#else
	struct pollfd pfd;
	pfd.fd = m_sock;
	pfd.revents = 0;
	pfd.events = POLLOUT;

	int ret = poll( &pfd, 1, 0 );
	if( pfd.revents & POLLOUT ) {
		int err = 0;
		socklen_t len = sizeof(err);
		ret = getsockopt( m_sock, SOL_SOCKET, SO_ERROR, (void *)&err, &len );
		if( ret == 0 && err == 0 )
			return DGN_SOCKET_CONNECT_OK; // connected
		PR_DEBUG( "getsockopt ret %d, err %d", ret, err );
		return DGN_SOCKET_CONNECT_FAILED; // failed
	}

	// PR_DEBUG( "poll ret %d, revent %d", ret, pfd.revents );
	return DGN_SOCKET_CONNECT_TRYING; // still in progress
#endif
}

int Socket::TcpSvr( const char * host, int port )
{
	int addrlen = 0;
	struct sockaddr_storage addr;
	if( GetHostAddr( host, port, &addr, &addrlen ) < 0 ) {
		PR_DEBUG( "resolve [%s] failed", host );
		return -1;
	}

	Close();

	m_sock = socket( AF_INET, SOCK_STREAM, 0 );
	if( m_sock == INVALID_SOCKET ) {
		PR_DEBUG( "socket() failed" );
		return -1;
	}
	if( SetNonBlock( m_sock, 1 ) < 0 ) {
		PR_DEBUG( "SetNonBlock() failed" );
		Close();
		return -1;
	}

#ifndef _WIN32
	// WIN32 need not set reuseaddr, has other means
	int option = 1;
	if( setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option) ) < 0 ) {
		PR_DEBUG( "setsockopt() SO_REUSEADDR failed" );
		Close();
		return -1;
	}
#endif

	if( bind( m_sock, (struct sockaddr *)&addr, addrlen ) < 0 ) {
		PR_DEBUG( "bind [%s:%d] failed", host, port );
		Close();
		return -1;
	}
	if( listen( m_sock, 7 ) < 0 ) {
		PR_DEBUG( "listen() [%s:%d] failed", host, port );
		Close();
		return -1;
	}

	return 0;
}

Socket * Socket::Accept()
{
	if( m_sock == INVALID_SOCKET ) {
		PR_DEBUG( "accept but not listen()" );
		return NULL;
	}

	if( m_timeout_ms > 0 ) {
		int ret_evt;
		int ret = Poll( DGN_POLLIN, &ret_evt );
		if( ret <= 0 ) {
			return NULL;
		}
	}

	SOCKET tmpsk = accept( m_sock, NULL, 0 );
	if( tmpsk == INVALID_SOCKET )
		return NULL;

	if( SetNonBlock( tmpsk, 1 ) < 0 ) {
		PR_DEBUG( "client set nonblock failed" );
		closesocket( tmpsk ), tmpsk = INVALID_SOCKET;
		return NULL;
	}

	Socket * sk = new Socket();
	sk->m_sock = tmpsk;
	
	return sk;
}

int Socket::UdpSvr( const char * host, int port )
{
	int addrlen = 0;
	struct sockaddr_storage addr;
	if( GetHostAddr( host, port, &addr, &addrlen ) < 0 ) {
		PR_DEBUG( "resolve [%s] failed", host );
		return -1;
	}

	Close();

	m_sock = socket( AF_INET, SOCK_DGRAM, 0 );
	if( m_sock == INVALID_SOCKET ) {
		PR_DEBUG( "socket() failed" );
		Close();
		return -1;
	}

	if( SetNonBlock( m_sock, 1 ) < 0 ) {
		PR_DEBUG( "SetNonBlock() failed" );
		Close();
		return -1;
	}

#ifndef _WIN32
	// WIN32 need not set reuseaddr, has other means
	int option = 1;
	if( setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option) ) < 0 ) {
		PR_DEBUG( "setsockopt() SO_REUSEADDR failed" );
		Close();
		return -1;
	}
#endif

	if( bind( m_sock, (struct sockaddr *)&addr, addrlen ) < 0 ) {
		PR_DEBUG( "bind [%s:%d] failed", host, port );
		Close();
		return -1;
	}

	return 0;
}

int Socket::AttachSock( sock_t sk )
{
	Close();
	m_sock = sk;
	return 0;
}

sock_t Socket::DetachSock()
{
	uint32_t tmp_sock = (uint32_t)m_sock;
	m_sock = INVALID_SOCKET;
	return tmp_sock;
}

int Socket::LocalAddr( char ip[DGN_IP_LEN], int * port )
{
	if( m_sock == INVALID_SOCKET ) {
		return -1;
	}

	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	int ret = getsockname( m_sock, (struct sockaddr *)&addr, &addrlen );
	if( ret < 0 ) {
		PR_DEBUG( "getsockname() failed" );
		return -1;
	}
	return GetAddrIp( &addr, addrlen, ip, port );
}

int Socket::LocalAddr( CStr * ip, int * port )
{
	ip->Reserve( DGN_IP_LEN );
	char * p = ip->GetRaw();
	p[DGN_IP_LEN - 1] = '\0';
	int ret = LocalAddr( p, port );
	ip->ReleaseRaw( strlen(p) );
	return ret;
}

int Socket::RemoteAddr( char ip[DGN_IP_LEN], int * port )
{
	if( m_sock == INVALID_SOCKET ) {
		return -1;
	}

	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	int ret = getpeername( m_sock, (struct sockaddr *)&addr, &addrlen );
	if( ret < 0 ) {
		PR_DEBUG( "getpeername() failed" );
		return -1;
	}
	return GetAddrIp( &addr, addrlen, ip, port );
}

int Socket::RemoteAddr( CStr * ip, int * port )
{
	ip->Reserve( DGN_IP_LEN );
	char * p = ip->GetRaw();
	p[DGN_IP_LEN - 1] = '\0';
	int ret = RemoteAddr( p, port );
	ip->ReleaseRaw( strlen(p) );
	return ret;
}

int Socket::Send( const char * buf, int len )
{
	if( m_sock == INVALID_SOCKET ) {
		return -1;
	}

	int currlen = 0;
	int ret = send( m_sock, buf, len, 0 );
	if( ret == len )
		return ret;
	if( ret < 0 && ! IS_ERR_EAGAIN() )
		return ret;
	if( m_timeout_ms <= 0 ) {
		return ret < 0 ? 0 : ret;
	}

	if( ret > 0 )
		currlen = ret;

	while( currlen < len ) {
		int ret_evt = 0;
		ret = Poll( DGN_POLLOUT, &ret_evt );
		if( ret <= 0 ) {
			return (ret < 0 && currlen == 0) ? -1 : currlen;
		}

		ret = send( m_sock, buf + currlen, len - currlen, 0 );
		if( ret < 0 && ! IS_ERR_EAGAIN() ) {
			return currlen == 0 ? -1 : currlen;
		}
		if( ret > 0 )
			currlen += ret;
	}
	return currlen;
}

int Socket::Recv( char * buf, int minlen, int maxlen )
{
	if( m_sock == INVALID_SOCKET ) {
		return -1;
	}

	int currlen = 0;
	int ret = recv( m_sock, buf, maxlen, 0 );
	if( ret >= minlen )
		return ret;
	if( ret == 0 )
		return -1; // peer reset, peer closed, treat as error
	if( ret < 0 && ! IS_ERR_EAGAIN() )
		return ret;
	if( m_timeout_ms <= 0 ) {
		return ret < 0 ? 0 : ret;
	}

	if( ret > 0 )
		currlen = ret;

	while( currlen < minlen ) {
		int ret_evt = 0;
		ret = Poll( DGN_POLLIN, &ret_evt );
		if( ret <= 0 ) {
			return ret < 0 && currlen == 0 ? -1 : currlen;
		}

		ret = recv( m_sock, buf + currlen, maxlen - currlen, 0 );
		if( ret < 0 && ! IS_ERR_EAGAIN() ) {
			return currlen == 0 ? -1 : currlen;
		}
		if( ret == 0 ) {
			return currlen == 0 ? -1 : currlen;
		}
		if( ret > 0 )
			currlen += ret;
	}

	return currlen;
}

int Socket::SendTo( const char * buf, int len, const char * remote_host, int port )
{
	if( m_sock == INVALID_SOCKET ) {
		return -1;
	}

	int addrlen = 0;
	struct sockaddr_storage addr;
	if( GetHostAddr( remote_host, port, &addr, &addrlen ) < 0 )
		return -2;
	return sendto( m_sock, buf, len, 0, (struct sockaddr *)&addr, addrlen );
}

int Socket::RecvFrom( char * buf, int len, char remote_ip[DGN_IP_LEN], int * port )
{
	if( m_sock == INVALID_SOCKET ) {
		return -1;
	}

	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	int ret = recvfrom( m_sock, buf, len, 0, (struct sockaddr *)&addr, &addrlen );
	if( ret > 0 ) {
		GetAddrIp( &addr, addrlen, remote_ip, port );
		return ret;
	}
	if( ret < 0 && ! IS_ERR_EAGAIN() ) {
		return -1;
	}

	if( m_timeout_ms <= 0 ) {
		return 0;
	}

	int ret_evt = 0;
	ret = Poll( DGN_POLLIN, &ret_evt );
	if( ret <= 0 ) {
		return 0;
	}

	addrlen = sizeof(addr);
	ret = recvfrom( m_sock, buf, len, 0, (struct sockaddr *)&addr, &addrlen );
	if( ret > 0 ) {
		GetAddrIp( &addr, addrlen, remote_ip, port );
		return ret;
	}
	if( ret < 0 && ! IS_ERR_EAGAIN() ) {
		return -1;
	}

	return 0;
}


int Socket::Poll( int want_evt, int * ret_evt, int check_timeout_interval_ms )
{
	if( m_sock == INVALID_SOCKET ) {
		*ret_evt = 0;
		return -1;
	}

	*ret_evt = 0;

	uint32_t now, tm_end;
	int old_timeout = m_timeout_ms;
	now = Time::Tick();
	tm_end = now + old_timeout;

	if( check_timeout_interval_ms <= 0 )
		check_timeout_interval_ms = m_timeout_ms;

#ifdef _WIN32
	fd_set rset, wset, eset;
	struct timeval tv;

	// PR_DEBUG( "tm_end %d, now %d", tm_end, now );
	while( 1 ) {
		int new_timeout = m_timeout_ms;
		if( new_timeout != old_timeout ) {
			// PR_DEBUG( "tm_end %d, old %d, new %d", tm_end, old_timeout, new_timeout );
			tm_end = tm_end - old_timeout + new_timeout;
		}
		now = Time::Tick();
		int diff = (int)( tm_end - now );
		if( diff < 0 ) {
			// PR_DEBUG( "tm_end %d, now %d", tm_end, now );
			return 0;
		}

		FD_ZERO( &rset );
		FD_ZERO( &wset );
		FD_ZERO( &eset );
		if( want_evt & DGN_POLLIN )
			FD_SET( m_sock, &rset );
		if( want_evt & DGN_POLLOUT ) {
			FD_SET( m_sock, &wset );
			FD_SET( m_sock, &eset );
		}
		if( diff > check_timeout_interval_ms )
			diff = check_timeout_interval_ms;
		tv.tv_sec = diff / 1000;
		tv.tv_usec = (diff % 1000) * 1000;

		int ret = select( 0, &rset, &wset, &eset, &tv );
		if( ret < 0 ) {
			// PR_DEBUG( "select ret %d, errno %d", ret, GET_ERRNO() );
			return 0;
		}
		if( ret == 0 )
			continue;
		if( FD_ISSET( m_sock, &rset ) )
			*ret_evt |= DGN_POLLIN;
		if( FD_ISSET( m_sock, &wset ) || FD_ISSET( m_sock, &eset ) )
			*ret_evt |= DGN_POLLOUT;
		return *ret_evt;
	}

#else
	struct pollfd pfd;

	// PR_DEBUG( "tm_end %d, now %d", tm_end, now );
	while( 1 ) {
		int new_timeout = m_timeout_ms;
		if( new_timeout != old_timeout ) {
			// PR_DEBUG( "tm_end %d, old %d, new %d", tm_end, old_timeout, new_timeout );
			tm_end = tm_end - old_timeout + new_timeout;
		}
		now = Time::Tick();
		int diff = (int)( tm_end - now );
		if( diff < 0 ) {
			// PR_DEBUG( "tm_end %d, now %d", tm_end, now );
			return 0;
		}

		pfd.fd = m_sock;
		pfd.events = pfd.revents = 0;
		if( want_evt & DGN_POLLIN )
			pfd.events |= POLLIN;
		if( want_evt & DGN_POLLOUT )
			pfd.events |= POLLOUT;

		if( diff > check_timeout_interval_ms )
			diff = check_timeout_interval_ms;

		int ret = poll( &pfd, 1, diff );
		// PR_DEBUG( "poll ret %d, err no %d", ret, GET_ERRNO() );
		if( ret < 0 && ! IS_ERR_EAGAIN() ) {
			return 0;
		}
		if( ret <= 0 )
			continue;
		if( pfd.revents & POLLIN )
			*ret_evt |= DGN_POLLIN;
		if( pfd.revents & POLLOUT )
			*ret_evt |= DGN_POLLOUT;

		return *ret_evt;
	}
#endif
	return 0; // never come here
}

int Socket::Pollex( int num, Socket ** skarr, const int * want_evt, int * ret_evt, volatile int * timeout_ms, int check_timeout_interval_ms )
{
	int i;
	int ret = 0;
	uint32_t now, tm_end;
	int old_timeout = *timeout_ms;
	if( old_timeout < 0 )
		old_timeout = 0;
	now = Time::Tick();
	tm_end = now + old_timeout;

	if( check_timeout_interval_ms <= 0 )
		check_timeout_interval_ms = *timeout_ms;

#ifdef _WIN32
	fd_set rset, wset, eset;
	struct timeval tv;

	while( 1 ) {
		int new_timeout = *timeout_ms;
		if( new_timeout != old_timeout ) {
			tm_end = tm_end - old_timeout + new_timeout;
		}
		now = Time::Tick();
		int diff = (int)( tm_end - now );
		if( diff < 0 ) {
			ret = 0;
			break;
		}

		FD_ZERO( &rset );
		FD_ZERO( &wset );
		FD_ZERO( &eset );
		for( i = 0; i < num; ++i ) {
			if( skarr[i]->m_sock == INVALID_SOCKET )
				continue;
			if( want_evt[i] & DGN_POLLIN )
				FD_SET( skarr[i]->m_sock, &rset );
			if( want_evt[i] & DGN_POLLOUT ) {
				FD_SET( skarr[i]->m_sock, &wset );
				FD_SET( skarr[i]->m_sock, &eset );
			}
		}
		if( diff > check_timeout_interval_ms )
			diff = check_timeout_interval_ms;
		tv.tv_sec = 0;
		tv.tv_usec = diff * 1000;

		int ret = select( 0, &rset, &wset, &eset, &tv );
		if( ret == 0 || (ret < 0 && IS_ERR_EAGAIN()) )
			continue;
		if( ret < 0 )
			break;

		for( i = 0; i < num; ++i ) {
			ret_evt[i] = 0;
		}
		for( i = 0; i < num; ++i ) {
			if( skarr[i]->m_sock == INVALID_SOCKET )
				continue;
			if( FD_ISSET( skarr[i]->m_sock, &rset ) )
				ret_evt[i] |= DGN_POLLIN;
			if( FD_ISSET( skarr[i]->m_sock, &wset ) || FD_ISSET( skarr[i]->m_sock, &eset ) )
				ret_evt[i] |= DGN_POLLOUT;
		}
		return ret;
	}

#else
	struct pollfd pfd[1024];
	int pnum = 0;
	while( 1 ) {
		int new_timeout = *timeout_ms;
		if( new_timeout != old_timeout ) {
			tm_end = tm_end - old_timeout + new_timeout;
		}
		now = Time::Tick();
		int diff = (int)( tm_end - now );
		if( diff < 0 ) {
			ret = 0;
			return 0;
		}

		pnum = 0;
		for( i = 0; i < num; ++i ) {
			if( skarr[i]->m_sock == INVALID_SOCKET )
				continue;
			pfd[pnum].fd = skarr[i]->m_sock;
			pfd[pnum].events = pfd[pnum].revents = 0;
			if( want_evt[i] & DGN_POLLIN )
				pfd[pnum].events |= POLLIN;
			if( want_evt[i] & DGN_POLLOUT )
				pfd[pnum].events |= POLLOUT;
			pnum++;
		}

		if( diff > check_timeout_interval_ms )
			diff = check_timeout_interval_ms;

		int ret = poll( pfd, pnum, diff );
		if( ret == 0 || (ret < 0 && IS_ERR_EAGAIN() ) )
			continue;
		if( ret < 0 )
			break;

		for( i = 0; i < num; ++i ) {
			ret_evt[i] = 0;
		}
		pnum = 0;
		for( i = 0; i < num; ++i ) {
			if( skarr[i]->m_sock == INVALID_SOCKET )
				continue;
			if( pfd[pnum].revents & POLLIN )
				ret_evt[i] |= DGN_POLLIN;
			if( pfd[pnum].revents & POLLOUT )
				ret_evt[i] |= DGN_POLLOUT;
			pnum++;
		}
		return ret;
	}
#endif

	// timeout or select/poll error, clear evt
	for( i = 0; i < num; ++i ) {
		ret_evt[i] = 0;
	}

	return ret;
}

////////////////
END_NS_DGN

