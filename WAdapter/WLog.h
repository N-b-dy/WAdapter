#ifndef _W_LOG_H
#define _W_LOG_H

#include <inttypes.h>
#include <stdarg.h>
#include "Arduino.h"

typedef void (*printfunction)(Print*);

#define LOG_LEVEL_SILENT  0
#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_NOTICE  4
#define LOG_LEVEL_TRACE   5
#define LOG_LEVEL_VERBOSE 6

#define CR "\n"
#define LOGGING_VERSION 1_0_3

/**
 * Logging is a helper class to output informations over
 * RS232. If you know log4j or log4net, this logging class
 * is more or less similar ;-) <br>
 * Different loglevels can be used to extend or reduce output
 * All methods are able to handle any number of output parameters.
 * All methods print out a formated string (like printf).<br>
 * To reduce output and program size, reduce loglevel.
 * 
 * Output format string can contain below wildcards. Every wildcard
 * must be start with percent sign (\%)
 * 
 * ---- Wildcards
 * 
 * %s	replace with an string (char*)
 * %c	replace with an character
 * %d	replace with an integer value
 * %l	replace with an long value
 * %x	replace and convert integer value into hex
 * %X	like %x but combine with 0x123AB
 * %b	replace and convert integer value into binary
 * %B	like %x but combine with 0b10100011
 * %t	replace and convert boolean value into "t" or "f"
 * %T	like %t but convert into "true" or "false"
 * 
 * ---- Loglevels
 * 
 * 0 - LOG_LEVEL_SILENT     no output
 * 1 - LOG_LEVEL_FATAL      fatal errors
 * 2 - LOG_LEVEL_ERROR      all errors
 * 3 - LOG_LEVEL_WARNING    errors and warnings
 * 4 - LOG_LEVEL_NOTICE     errors, warnings and notices
 * 5 - LOG_LEVEL_TRACE      errors, warnings, notices, traces
 * 6 - LOG_LEVEL_VERBOSE    all
 */

class WLog {
public:
	WLog(int level, Print *output, bool showLevel = true) {
		setLevel(level);
		setShowLevel(showLevel);
		_logOutput = output;
	}

	void setLevel(int level) {
		_level = constrain(level, LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE);
	}

	void setShowLevel(bool showLevel) {
		_showLevel = showLevel;
	}

	void setPrefix(printfunction f) {
		_prefix = f;
	}

	void setSuffix(printfunction f) {
		_suffix = f;
	}

	template<class T, typename ... Args> void fatal(T msg, Args ... args) {
		printLevel(LOG_LEVEL_FATAL, msg, args...);
	}

	template<class T, typename ... Args> void error(T msg, Args ... args) {
		printLevel(LOG_LEVEL_ERROR, msg, args...);
	}

	template<class T, typename ... Args> void warning(T msg, Args ...args) {
		printLevel(LOG_LEVEL_WARNING, msg, args...);
	}

	template<class T, typename ... Args> void notice(T msg, Args ...args) {
		printLevel(LOG_LEVEL_NOTICE, msg, args...);
	}

	template<class T, typename ... Args> void trace(T msg, Args ... args) {
		printLevel(LOG_LEVEL_TRACE, msg, args...);
	}

	template<class T, typename ... Args> void verbose(T msg, Args ... args) {
		printLevel(LOG_LEVEL_VERBOSE, msg, args...);
	}

private:
	void print(const char *format, va_list args) {
		for (; *format != 0; ++format) {
			if (*format == '%') {
				++format;
				printFormat(*format, &args);
			} else {
				_logOutput->print(*format);
			}
		}
		_logOutput->println();
	}

	void print(const __FlashStringHelper *format, va_list args) {
		PGM_P p = reinterpret_cast<PGM_P>(format);
		char c = pgm_read_byte(p++);
		for(;c != 0; c = pgm_read_byte(p++)) {
			if (c == '%') {
				c = pgm_read_byte(p++);
				printFormat(c, &args);
			} else {
				_logOutput->print(c);
			}
		}
		_logOutput->println();
	}

	void printFormat(const char format, va_list *args) {
		if (format == '%') {
			_logOutput->print(format);
		} else if (format == 's') {
			register char *s = (char *)va_arg(*args, int);
			_logOutput->print(s);
		} else if (format == 'S') {
			register __FlashStringHelper *s = (__FlashStringHelper *)va_arg(*args, int);
			_logOutput->print(s);
		} else if (format == 'd' || format == 'i') {
			_logOutput->print(va_arg(*args, int), DEC);
		} else if (format == 'D' || format == 'F') {
			_logOutput->print(va_arg(*args, double));
		} else if (format == 'x') {
			_logOutput->print(va_arg(*args, int), HEX);
		} else if (format == 'X') {
			_logOutput->print("0x");
			_logOutput->print(va_arg(*args, int), HEX);
		} else if (format == 'b') {
			_logOutput->print(va_arg(*args, int), BIN);
		} else if (format == 'B') {
			_logOutput->print("0b");
			_logOutput->print(va_arg(*args, int), BIN);
		} else if (format == 'l') {
			_logOutput->print(va_arg(*args, long), DEC);
		} else if (format == 'c') {
			_logOutput->print((char) va_arg(*args, int));
		} else if (format == 't') {
			if (va_arg(*args, int) == 1) {
				_logOutput->print("T");
			} else {
				_logOutput->print("F");
			}
		} else if (format == 'T') {
			if (va_arg(*args, int) == 1) {
				_logOutput->print(F("true"));
			} else {
				_logOutput->print(F("false"));
			}
		}
	}

	template<class T> void printLevel(int level, T msg, ...) {
		if (level > _level) {
			return;
		}

		if (_prefix != NULL) {
			_prefix(_logOutput);
		}

		if (_showLevel) {
			static const char levels[] = "FEWNTV";
			_logOutput->print(levels[level - 1]);
			_logOutput->print(": ");
		}

		va_list args;
		va_start(args, msg);
		print(msg, args);

		if (_suffix != NULL) {
			_suffix(_logOutput);
		}
	}

	int _level;
	bool _showLevel;
	Print *_logOutput;

	printfunction _prefix = NULL;
	printfunction _suffix = NULL;

};

#endif

