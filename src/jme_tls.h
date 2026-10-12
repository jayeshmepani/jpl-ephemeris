#ifndef JME_TLS_H
#define JME_TLS_H

/* Thread-local storage for per-thread scratch and caches (no shared mutable state). */
#if defined(_MSC_VER)
#define JME_THREAD_LOCAL __declspec(thread)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define JME_THREAD_LOCAL _Thread_local
#else
#define JME_THREAD_LOCAL __thread
#endif

#endif
