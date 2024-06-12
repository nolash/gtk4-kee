#ifndef DEBUG_H_
#define DEBUG_H_

// provides:
#include <llog.h>
#include <rerr.h>

/**
 * \brief Debug levels for simple log output.
 */
/// Critical error, should terminate application.
#define DEBUG_CRITICAL LLOG_CRITICAL
/// Error, anomalous condition that should not occur.
#define DEBUG_ERROR LLOG_ERROR
/// Warning, condition that may contribute to affecting the state of the program adversely.
#define DEBUG_WARNING LLOG_WARNING
/// Info, events that an end-user may be interested in during normal operation.
#define DEBUG_INFO LLOG_INFO
/// Debug, events that a developer may be intereted in during normal operation.
#define DEBUG_DEBUG LLOG_DEBUG
/// Trace, mostly temporary loglines used for debugging concrete issues during development.
#define DEBUG_TRACE LLOG_GURU

/**
 *
 * \brief Implementer logs constant string according to given log level.
 *
 * \param level Debug level
 * \param s String to log
 */
//void debug_log(enum debugLevel level, const char *s);
void debug_log(int level, const char *s);
int debug_logerr(enum lloglvl_e, int err, char *s);

#endif
