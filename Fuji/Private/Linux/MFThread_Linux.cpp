#include "Fuji.h"
#include "MFThread.h"


// interface functions

MFThread MFThread_CreateThread(const char *pName, MFThreadEntryPoint pEntryPoint, void *pUserData, int priority, uint32 stackSize)
{
	return NULL;
}

void MFThread_ExitThread(int exitCode)
{
}

void MFThread_TerminateThread(MFThread thread)
{
}

int MFThread_GetExitCode(MFThread thread)
{
	return 0;
}

void MFThread_DestroyThread(MFThread thread)
{
}


MFMutex MFThread_CreateMutex(const char *pName)
{
	return 0;
}

void MFThread_DestroyMutex(MFMutex mutex)
{
}

void MFThread_LockMutex(MFMutex mutex)
{
}

void MFThread_ReleaseMutex(MFMutex mutex)
{
}

MFSemaphore MFThread_CreateSemaphore(const char *pName, int maxCount, int startCount)
{
	return NULL;
}

void MFThread_DestroySemaphore(MFSemaphore semaphore)
{
}

uint32 MFThread_WaitSemaphore(MFSemaphore semaphore, uint32 timeout)
{
	return 0;
}

void MFThread_SignalSemaphore(MFSemaphore semaphore)
{
}
