#pragma once

#include <windows.h>

#define SEM_ID HANDLE

typedef enum
{
	LOCK_STATE_UNKNOWN = -1,
	LOCK_STATE_UNLOCKED,
	LOCK_STATE_LOCKED
}LOCK_STATE_ETYPE;

class CMutexLock
{
public:
	CMutexLock();
	virtual ~CMutexLock();

	bool lock();
	bool unlock();
protected:
	LOCK_STATE_ETYPE m_eLockState;
private:
	SEM_ID m_key;
};

class CMutexExlusiveLock
{
public:
	CMutexExlusiveLock(CMutexLock &);
	virtual ~CMutexExlusiveLock();

private:
	CMutexLock& m_mutexLock;
};
