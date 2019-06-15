// Thread.cpp : thread
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

#include <dgn/Thread.h>
#include <dgn/Logger.h>

#ifdef _WIN32
#include <process.h>
#define INVALID_THD NULL
#else
// TODO : implementation depend, work with glibc
#define INVALID_THD ((pthread_t)-1)
#endif

BEGIN_NS_DGN
////////////////
////	CondVal
void CondVal::Signal( int val )
{
	if( val < 0 )
		return;
	m_val = val;
#ifdef _WIN32
	if( val == 0 )
		ResetEvent( m_event );
	else
		SetEvent( m_event );
#else
	if( val > 0 )
		pthread_cond_signal( &m_cond );
#endif
	return;
}

int CondVal::Wait( Mutex * lock, int timeout_ms )
{

	if( m_val == 0 ) {
#ifdef _WIN32
		lock->UnLock();
		DWORD tm = timeout_ms < 0 ? INFINITE : timeout_ms;
		/*DWORD ret =*/ WaitForSingleObject( m_event, tm );
		lock->Lock();
#else
		/*int ret;*/
		if( timeout_ms < 0 ) {
			/*ret =*/ pthread_cond_wait( &m_cond, &lock->m_lock );
		}
		else {
			struct timespec ts;
			clock_gettime( CLOCK_REALTIME, &ts );
			ts.tv_sec += timeout_ms / 1000;
			ts.tv_nsec += (timeout_ms % 1000) * ( 1000 * 100 );
			if( ts.tv_nsec >= 1000 * 1000 * 100 ) {
				ts.tv_sec += 1;
				ts.tv_nsec -= 1000 * 1000 * 100;
			}
			/*ret =*/ pthread_cond_timedwait( &m_cond, &lock->m_lock, &ts );
		}
#endif
	}

	int ret_val = m_val;
	m_val = 0;
#ifdef _WIN32
	ResetEvent( m_event );
#endif
	return ret_val;
}

///////////////////////
////	Thread

Thread::Thread()
	: m_th( INVALID_THD )
	, m_thread_state( THREAD_STATE_INIT )
	, m_stop_flag( 0 )
{
}

Thread::~Thread()
{
	if( m_th != INVALID_THD ) {
		PR_ERR( "runner %p thread is still running", this );
	}
}

bool Thread::is_thread_handle_valid() const
{
	return m_th != INVALID_THD;
}

int Thread::Start()
{
	if( is_thread_handle_valid() ) {
		PR_ERR( "thread already running" );
		return -1;
	}

	m_thread_state = THREAD_STATE_RUNNING;
	m_stop_flag = 0;

#ifdef _WIN32
	m_th = (HANDLE)_beginthreadex( NULL, 0, s_wrap_thread_start_routine, this, 0, NULL );
#else
	int ret = pthread_create( &m_th, NULL, s_wrap_thread_start_routine, this );
	if( ret != 0 )
		m_th = INVALID_THD;
#endif

	if( m_th == INVALID_THD ) {
		m_thread_state = THREAD_STATE_INIT;
 		PR_ERR( "create thread failed" );
		return -1;
	}
	return 0;
}

void Thread::SignalStop()
{
	m_stop_flag = 1;
	return;
}

int Thread::WaitStop()
{
	if( m_th == INVALID_THD )
		return 0;

	SignalStop();

#ifdef _WIN32
	WaitForSingleObject( m_th, INFINITE );
	CloseHandle( m_th );
#else
	void * tmp = NULL;
	pthread_join( m_th, &tmp );
#endif

	m_th = INVALID_THD;
	m_thread_state = THREAD_STATE_STOPPED;

	return 0;
}

int Thread::SetState( int state )
{
	if( state < THREAD_STATE_RUNNING ) {
		PR_ERR( "wrong state %d, user should only set running state", state );
		return -1;
	}
	m_thread_state = state;
	return 0;
}

#ifdef _WIN32
unsigned int __stdcall Thread::s_wrap_thread_start_routine( void * arg )
{
	Thread * th = (Thread *)arg;
	int ret = th->run_thread();
	th->m_thread_state = THREAD_STATE_ZOMBIE;
	_endthreadex( 0 );
	return (DWORD)ret;
}
#else
void * Thread::s_wrap_thread_start_routine( void * arg )
{
	Thread * th = (Thread *)arg;
	int ret = th->run_thread();
	th->m_thread_state = THREAD_STATE_ZOMBIE;
	return (void *)(intptr_t)ret;
}
#endif

////////////////
////	ThreadObj

ThreadObj::ThreadObj( thread_func_t func, void * arg )
	: m_func( func )
	, m_arg( arg )
{

}

ThreadObj::~ThreadObj()
{

}

int ThreadObj::SetFunc( thread_func_t func, void * arg )
{
	if( is_thread_handle_valid() ) {
		PR_ERR( "thread already running" );
		return -1;
	}

	m_func = func;
	m_arg = arg;
	return 0;
}

int ThreadObj::run_thread()
{
	if( m_func == NULL )
		return -1;
	return (*m_func)( this, m_arg );
}

////////////////
END_NS_DGN

