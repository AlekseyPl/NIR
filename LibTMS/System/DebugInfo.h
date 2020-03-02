#ifndef SYSTEM_DEBUGINFO_H_
#define SYSTEM_DEBUGINFO_H_

#include <stddef.h>
#include <stdint.h>
#include <cstdio>
#include <cstdarg>

namespace System {
class Command;
class HostAdapter;

/**
 * Посылает отладочную информацию на хост.
 */
class DebugInfo {
public:
	struct Config {
		int debugLevel;
		int defaultDebugLevel;
		bool enableStdout;
		explicit Config(int debugLevel = DebugInfo::all, int defaultDebugLevel = DebugInfo::info, bool enableStdout =
				true);
	};

	enum DebugLevel {
		all, info, warn, error, attention, none
	};

	static DebugInfo & Locate();

	/**
	 *  Отправка текстовой информации в формате printf
	 * @param format как в printf
	 */
	virtual void SendText(const char *format, ...);
	virtual void SendText(DebugLevel level, const char *format, ...);
	/**
	 * Отправка текстовой информации в формате printf (аналог vprintf)
	 * @param format
	 * @param ap
	 */
	virtual void SendTextVA(const char *format, std::va_list ap);
	virtual void SendTextVA(DebugLevel level, const char *format, std::va_list ap);
	/**
	 *  Отправка текстовой информации в формате printf
	 * @param format как в printf
	 */
	virtual void SendTextDirect(const char *format, ...);
	virtual void SendTextDirect(DebugLevel level, const char *format, ...);
	/**
	 * Отправка текстовой информации в формате printf (аналог vprintf)
	 * @param format
	 * @param ap
	 */
	virtual void SendTextVADirect(const char *format, std::va_list ap);
	virtual void SendTextVADirect(DebugLevel level, const char *format, std::va_list ap);

	virtual int GetDebugLevel() {
		return config.debugLevel;
	}

	void EnableStdout() {
		config.enableStdout = true;
	}
	void DisableStdout() {
		config.enableStdout = false;
	}
protected:

	virtual ~DebugInfo() {
	}
	DebugInfo();
	HostAdapter * adapter;
	Config config;
	static const int maxTextLen = 127;
};

}

#endif // SYSTEM_DEBUGINFO_H_
