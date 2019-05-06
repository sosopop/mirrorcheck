#ifndef MIRRORACCEL_THREAD_H_
#define MIRRORACCEL_THREAD_H_

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

#if defined(WIN32)
#define MA_STDCALL __stdcall
#define ma_mutex_t CRITICAL_SECTION
#define ma_thread_t HANDLE
#define ma_thread_t_null (HANDLE)0
#if !defined(_WIN32_WINNT) || !defined(_WIN32_WINNT_VISTA) || \
    (_WIN32_WINNT < _WIN32_WINNT_VISTA)
#define ma_mutex_init(m) InitializeCriticalSection(m)
#else
#define ma_mutex_init(m) InitializeCriticalSectionEx(m, 0, 1)
#endif
#define ma_mutex_acquire(m) EnterCriticalSection(m)
#define ma_mutex_release(m) LeaveCriticalSection(m)
#define ma_mutex_destroy(m) DeleteCriticalSection(m)
#else
#define MA_STDCALL
#define ma_mutex_t pthread_mutex_t
#define ma_thread_t pthread_t *
#define ma_thread_t_null (pthread_t *)0
#define ma_mutex_init(m) pthread_mutex_init(m, NULL)
#define ma_mutex_acquire(m) pthread_mutex_lock(m)
#define ma_mutex_release(m) pthread_mutex_unlock(m)
#define ma_mutex_destroy(m) pthread_mutex_destroy(m)
#endif

ma_thread_t ma_thread_create(
    unsigned int(MA_STDCALL *func)(void *),
    void *arg);

void ma_thread_destroy(ma_thread_t hnd);

int ma_thread_join(ma_thread_t *hnd);

#endif