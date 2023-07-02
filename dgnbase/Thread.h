// Thread.h : thread
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

#ifndef INCLUDED_DGN_THREAD_H
#define INCLUDED_DGN_THREAD_H

#include <dgn/dgn.h>

#ifdef _WIN32
#include <windows.h>

#else
#include <pthread.h>

#endif

BEGIN_NS_DGN
////////////////

// <1> mutex

class CondVal;

class DGN_LIB_API Mutex
{
public:
	Mutex()
	{
#ifdef _WIN32
		InitializeCriticalSection( &m_lock );
#else
		pthread_mutex_init( &m_lock, NULL );
#endif
	}

	~Mutex()
	{
#ifdef _WIN32
		DeleteCriticalSection( &m_lock );
#else
		pthread_mutex_destroy( &m_lock );
#endif
	}

	Mutex( const Mutex & lock ) = delete;
	Mutex & operator = ( const Mutex & lock ) = delete;

	void Lock()
	{
#ifdef _WIN32
		EnterCriticalSection( &m_lock );
#else
		pthread_mutex_lock( &m_lock );
#endif
	}

	void UnLock()
	{
#ifdef _WIN32
		LeaveCriticalSection( &m_lock );
#else
		pthread_mutex_unlock( &m_lock );
#endif
	}

protected:
#ifdef _WIN32
	CRITICAL_SECTION m_lock;
#else
	friend class CondVal;
	pthread_mutex_t m_lock;
#endif
};

class DGN_LIB_API MutexGuard
{
public:
	MutexGuard( Mutex * mutex ) : m_mutex( mutex ) { m_mutex->Lock(); }
	~MutexGuard() { Detach(); }

	MutexGuard( const MutexGuard & lockguard ) = delete;
	MutexGuard & operator = ( const MutexGuard & lockguard ) = delete;

	void Detach() { if( m_mutex != NULL ) { m_mutex->UnLock(); m_mutex = NULL; } return; }

protected:
	Mutex * m_mutex;
};

// <2> CondVal

// value 0 means no signal value, > 0 means has signal value
// only the last signal value is valid, only one wait can get signal value
// signal value keep valid until some one wait and get it

class DGN_LIB_API CondVal
{
public:
	CondVal() : m_val(0)
	{
#ifdef _WIN32
		m_event = CreateEvent( NULL, TRUE, FALSE, NULL );
#else
		pthread_cond_init( &m_cond, NULL );
#endif
	}
	~CondVal()
	{
#ifdef _WIN32
		CloseHandle( m_event ), m_event = NULL;
#else
		pthread_cond_destroy( &m_cond );
#endif
	}

	CondVal( const CondVal & val ) = delete;
	CondVal & operator = ( const CondVal & val ) = delete;

	// value == 0 means remove value, value > 0 means set value
	void Signal( int val = 1 );

	// Mutex must be locked before call Wait, timeout_ms < 0 means forever
	// return 0 if timeout, > 0 success wait and get the value that signal before
	int Wait( Mutex * lock, int timeout_ms );

protected:
	volatile int m_val;
#ifdef _WIN32
	HANDLE m_event;
#else
	pthread_cond_t m_cond;
#endif
};

// <3> thread 

// outside the thread : create / start / signalstop / waitstop , ( getstate is not good style )
// inside the thread func : do work / setstate / getstate / check hasstopflag / return

// thread state, should be mofified by thread func only
enum {
	THREAD_STATE_UNKNOWN = 0, // should not exist
	THREAD_STATE_INIT, // init, not running
	THREAD_STATE_ZOMBIE, // zombie, thread stopped, before WaitStop
	THREAD_STATE_STOPPED, // thread stopped and afterWaitStop, almost the same as INIT

	// following are all running state, can set by user,
	THREAD_STATE_RUNNING = 10, // running at thread
	THREAD_STATE_USER = 20, // user custom state should begin from here
};

class DGN_LIB_API Thread
{
public:
	Thread();
	virtual ~Thread();

	virtual int Start();
	virtual void SignalStop();  // outside signal stop
	virtual int WaitStop(); // should call by outside thread, simple loop check

public: // should be call inside thread function, make public for ThreadObj's 3rd func use
	volatile int GetState() const { return m_thread_state; }
	volatile bool HasStopFlag() const { return m_stop_flag != 0; }

	int SetState( int state );

protected:
	virtual int run_thread() = 0; // main thread run, should check stop flag

#ifdef _WIN32
	static unsigned int __stdcall s_wrap_thread_start_routine( void * arg );
#else
	static void * s_wrap_thread_start_routine( void * arg );
#endif

protected:
	bool is_thread_handle_valid() const;

protected:
#ifdef _WIN32
	HANDLE m_th;
#else
	pthread_t m_th;
#endif

private:
	volatile int m_thread_state; // thread state, thread func inside to modify
	volatile int m_stop_flag;
};

// <4> thread obj, for composing instead of inherit

typedef int (* thread_func_t)( Thread * th, void * arg );

class DGN_LIB_API ThreadObj : public Thread
{
public:
	ThreadObj( thread_func_t func = NULL, void * arg = NULL );
	virtual ~ThreadObj();

	int SetFunc( thread_func_t func, void * arg = NULL );

protected:
	virtual int run_thread();

protected:
	thread_func_t m_func;
	void * m_arg;
};

template< typename T >
class ThreadObjTP : public Thread
{
public:
	typedef int (T::*func_t)( Thread * th, void * arg );
	ThreadObjTP( func_t func = NULL, T * obj = NULL, void * arg = NULL );
	virtual ~ThreadObjTP();

	int SetFunc( func_t func, T * obj, void * arg = NULL );

protected:
	virtual int run_thread();

protected:
	func_t m_func;
	T * m_obj;
	void * m_arg;
};

template< typename T >
ThreadObjTP<T>::ThreadObjTP( func_t func, T * obj, void * arg )
	: m_func( func ), m_obj( obj ), m_arg( arg )
{

}

template< typename T >
ThreadObjTP<T>::~ThreadObjTP()
{

}

template< typename T >
int ThreadObjTP<T>::SetFunc( func_t func, T * obj, void * arg )
{
	if( is_thread_handle_valid() ) {
		return -1;
	}

	m_func = func;
	m_obj = obj;
	m_arg = arg;	
	return 0;
}

template< typename T >
int ThreadObjTP<T>::run_thread()
{
	if( m_func == NULL || m_obj == NULL )
		return -1;
	return (m_obj->*m_func)( this, m_arg );
}

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_THREAD_H

