#include "Python.h"

#ifdef __APPLE__
#if defined(HAVE_GETTIMEOFDAY) && defined(HAVE_FTIME)
  /*
   * _PyTime_gettimeofday falls back to ftime when getttimeofday fails because the latter
   * might fail on some platforms. This fallback is unwanted on MacOSX because
   * that makes it impossible to use a binary build on OSX 10.4 on earlier
   * releases of the OS. Therefore claim we don't support ftime.
   */
# undef HAVE_FTIME
#endif
#endif

#ifdef HAVE_FTIME
#include <sys/timeb.h>
#if !defined(MS_WINDOWS) && !defined(PYOS_OS2)
extern int ftime(struct timeb *);
#endif /* MS_WINDOWS */
#endif /* HAVE_FTIME */

void
_PyTime_gettimeofday(_PyTime_timeval *tp)
{
    /* There are three ways to get the time:
      (1) gettimeofday() -- resolution in microseconds
      (2) ftime() -- resolution in milliseconds
      (3) time() -- resolution in seconds
      In all cases the return value in a timeval struct.
      Since on some systems (e.g. SCO ODT 3.0) gettimeofday() may
      fail, so we fall back on ftime() or time().
      Note: clock resolution does not imply clock accuracy! */

#if (defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY) \
     || defined(HAVE_FTIME))
    int err;
#endif
#ifdef HAVE_CLOCK_GETTIME
    struct timespec ts;
#endif
#ifdef HAVE_FTIME
    struct timeb t;
#endif

    /* test clock_gettime(CLOCK_REALTIME) */
#ifdef HAVE_CLOCK_GETTIME
    err  = clock_gettime(CLOCK_REALTIME, &ts);
    if (err == 0) {
#if 0 
       if (info) {
            struct timespec res;
            info->implementation = "clock_gettime(CLOCK_REALTIME)";
            info->monotonic = 0;
            info->adjustable = 1;
            if (clock_getres(CLOCK_REALTIME, &res) == 0)
                info->resolution = res.tv_sec + res.tv_nsec * 1e-9;
            else
                info->resolution = 1e-9;
        }
#endif
        tp->tv_sec = ts.tv_sec;
        tp->tv_usec = ts.tv_nsec / 1000;
        return;
    }
#endif

     /* test gettimeofday() */
#ifdef HAVE_GETTIMEOFDAY
#ifdef GETTIMEOFDAY_NO_TZ
    if (gettimeofday(tp) == 0)
        return;
#else /* !GETTIMEOFDAY_NO_TZ */
    if (gettimeofday(tp, (struct timezone *)NULL) == 0)
        return;
#endif /* !GETTIMEOFDAY_NO_TZ */
#endif /* !HAVE_GETTIMEOFDAY */

#ifdef HAVE_FTIME
    ftime(&t);
    tp->tv_sec = t.time;
    tp->tv_usec = t.millitm * 1000;
#if 0
    if (info) {
        info->implementation = "ftime()";
        info->resolution = 1e-3;
        info->monotonic = 0;
        info->adjustable = 1;
    }
#endif
#else /* !HAVE_FTIME */
    tp->tv_sec = time(NULL);
    tp->tv_usec = 0;
#endif /* !HAVE_FTIME */
    return;
}
