#ifndef _DEBUG_H
#define _DEBUG_H

/**
 * \brief Debug levels for simple log output.
 */
enum debugLevel {
	/// Critical error, should terminate application.
	DEBUG_CRITICAL,
	/// Error, anomalous condition that should not occur.
	DEBUG_ERROR,
	/// Warning, condition that may contribute to affecting the state of the program adversely.
	DEBUG_WARNING,
	/// Info, events that an end-user may be interested in during normal operation.
	DEBUG_INFO,
	/// Debug, events that a developer may be intereted in during normal operation.
	DEBUG_DEBUG,
	/// Trace, mostly temporary loglines used for debugging concrete issues during development.
	DEBUG_TRACE,
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * \brief Implementer logs constant string according to given log level.
 *
 * \param level Debug level
 * \param s String to log
 */
void debug_log(enum debugLevel level, const char *s);

#ifdef __cplusplus
}
#endif

#endif
