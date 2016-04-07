#include "CMutexExclusiveLock.h"

CMutexLock::CMutexLock()
:
	m_eLockState(LOCK_STATE_UNLOCKED)
{
	m_key = CreateSemaphore(NULL, 1, 1, NULL);
}//constructor

CMutexLock::~CMutexLock()
{
	CloseHandle(m_key);
}//~CMutex

bool CMutexLock::lock()
{
	m_eLockState = LOCK_STATE_LOCKED;
	if (!WaitForSingleObject(m_key, INFINITE))
		return true;
	return false;
}//lock

bool CMutexLock::unlock()
{
	m_eLockState = LOCK_STATE_UNLOCKED;
	if (ReleaseSemaphore(m_key, 1, NULL))
		return true;
	return false;
}//unlock

CMutexExlusiveLock::CMutexExlusiveLock(CMutexLock& _mutex)
:
	m_mutexLock(_mutex)
{
	m_mutexLock.lock();
}//Constructor

CMutexExlusiveLock::~CMutexExlusiveLock()
{
	m_mutexLock.unlock();
}//Destructor