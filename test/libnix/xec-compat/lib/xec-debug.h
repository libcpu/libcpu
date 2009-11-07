#ifndef __xec_debug_h
#define __xec_debug_h

#include <stdio.h>
#include "xec-base.h"

typedef enum _xec_log_severity
  {
    XEC_LOG_FATAL,
    XEC_LOG_ERROR,
    XEC_LOG_WARNING,
    XEC_LOG_INFO,
    XEC_LOG_DEBUG
  } xec_log_severity_t;

/* Flags for XEC_LOG */
#define XEC_LOG_NONE      0
#define XEC_LOG_CORE      1
#define XEC_LOG_DONTEXIT  2
#define XEC_LOG_NEWLINE   4
#define XEC_LOG_ERREXIT   8
#define XEC_LOG_CLEANEXIT 16

/* Flags for xec_log_set_flags */
#define XEC_LOG_TIMESTAMP 1
#define XEC_LOG_SOURCEINFO 2

#ifdef NDEBUG
#ifdef REALLY_NO_DEBUG
#define XEC_LOG(cookie, severity, flags, format, ...) do { } while (0)
#else
#define XEC_LOG(cookie, severity, flags, format, ...) \
( (severity) == XEC_LOG_FATAL ? xec_log (cookie, severity, flags, __func__, __FILE__, __LINE__, format, __VA_ARGS__) : (void)0)
#endif
#else
#define XEC_LOG(cookie, severity, flags, format, ...) \
xec_log (cookie, severity, flags, __func__, __FILE__, __LINE__, format, __VA_ARGS__)
#endif

#define __XEC_ABNORMAL_END(cookie, reason, forcecore) \
do {                                                  \
  XEC_LOG (cookie,                                    \
           XEC_LOG_FATAL,                             \
           (forcecore) ? XEC_LOG_CORE : 0,            \
           "%s",                                      \
           (reason));                                 \
} while (0)

#define XEC_BUGCHECK(cookie, id)  __XEC_ABNORMAL_END (cookie, "BUG CHECK " #id, true)
#define XEC_ERROR_NYI(cookie)     __XEC_ABNORMAL_END (cookie, "NOT YET IMPLEMENTED", false)
#define XEC_WARN_NYI(cookie)      XEC_LOG (cookie, XEC_LOG_WARNING, 0, "NOT YET IMPLEMENTED", 0)

#ifndef NDEBUG
#ifdef REALLY_NO_DEBUG
#define XEC_ASSERT(cookie, condition) do { } while (0)
#else
#define XEC_ASSERT(cookie, condition) \
do { if (!(condition)) XEC_LOG (cookie, XEC_LOG_FATAL, XEC_LOG_CORE, "Assertion failed: %s", #condition); } while (0)
#endif
#define XEC_ASSERT0(condition)                XEC_ASSERT (NULL, condition)

#define XEC_ASSERT2(cookie, condition, reason) \
do { if (!condition) XEC_LOG (cookie, XEC_LOG_FATAL, XEC_LOG_CORE, "Assertion failed: %s, reason: %s", #condition, reason); } while (0)
#else
#define XEC_ASSERT(cookie, condition)
#define XEC_ASSERT0(condition)
#define XEC_ASSERT2(cookie, condition, reason)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void
xec_abort
  (
    bool coredump
  );

void *
xec_log_register
  (
    char const *tag
  );

void
xec_log_unregister
  (
    void *cookie
  );

void
xec_log_disable_all
  (
    void
  );

bool
xec_log_enable
  (
    char const *tag
  );

bool
xec_log_disable
  (
    char const *tag
  );

bool
xec_log_is_enabled
  (
    char const *tag
  );

bool
xec_log_token_is_enabled
  (
    void *p
  );

void
xec_log_set_min_severity
  (
    char const         *tag,
    xec_log_severity_t  severity
  );

xec_log_severity_t
xec_log_get_min_severity
  (
    char const *tag
  );

bool
xec_log_to_file
  (
    char const *tag,
    char const *name
  );

bool
xec_log_to_stderr
  (
    char const *tag
  );

FILE *
xec_log_token_get_file
  (
    void *p
  );

FILE *
xec_log_get_file
  (
    char const *tag
  );

unsigned
xec_log_get_flags
  (
    void
  );

void
xec_log_set_flags
  (
    unsigned flags
  );

void
xec_log_cleanup
  (
    void
  );

void
xec_log_parse_options
  (
    char const *optstring
  );

void
xec_log
  (
    void               *cookie,
    xec_log_severity_t  severity,
    unsigned            flags,
    char const         *function,
    char const         *filename,
    unsigned            line,
    char const         *format,
    ...
  );

void
xec_logv
  (
    void               *cookie,
    xec_log_severity_t  severity,
    unsigned            flags,
    char const         *function,
    char const         *filename,
    unsigned            line,
    char const         *format,
    va_list             ap
  );

#ifdef __cplusplus
}
#endif

#endif  /* !__xec_debug_h */
