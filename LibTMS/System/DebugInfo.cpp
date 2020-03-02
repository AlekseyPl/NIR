/*
 * DebugInfo.cpp
 *
 *  Created on: Jan 20, 2016
 *      Author: rachicky
 */

#include "DebugInfo.h"
#include <iostream>

namespace System {
DebugInfo::Config::Config(int debugLevel, int defaultDebugLevel, bool enableStdout) :
		debugLevel(debugLevel), defaultDebugLevel(defaultDebugLevel), enableStdout(enableStdout) {

}


DebugInfo& DebugInfo::Locate()
{
	static System::DebugInfo obj;
	return obj;
}

DebugInfo::DebugInfo() :
      config(Config()) {

}

void DebugInfo::SendText(const char *format, ...) {
	std::va_list ap;
	va_start(ap, format);
	SendTextVA(static_cast<DebugLevel>(config.defaultDebugLevel), format, ap);
	va_end(ap);
}

void DebugInfo::SendTextVA(const char *format, std::va_list ap) {
	SendTextVA(static_cast<DebugLevel>(config.defaultDebugLevel), format, ap);
}

void DebugInfo::SendTextVA(DebugLevel level, const char *format, std::va_list ap) {

	char buf[maxTextLen];
	std::vsnprintf(buf, maxTextLen, format, ap);

	if (config.enableStdout) {
		std::cout << buf << std::endl << std::flush;
	}


}

void DebugInfo::SendTextDirect(const char *format, ...) {
	std::va_list ap;
	va_start(ap, format);
	SendTextVA(static_cast<DebugLevel>(config.defaultDebugLevel), format, ap);
	va_end(ap);
}

void DebugInfo::SendTextVADirect(const char *format, std::va_list ap) {
	SendTextVA(static_cast<DebugLevel>(config.defaultDebugLevel), format, ap);
}

void DebugInfo::SendTextDirect(DebugLevel lvl, const char *format, ...) {
	std::va_list ap;
	va_start(ap, format);
	SendTextVA(lvl, format, ap);
	va_end(ap);
}

void DebugInfo::SendTextVADirect(DebugLevel lvl, const char *format, std::va_list ap) {
	SendTextVA(lvl, format, ap);
}


void DebugInfo::SendText(DebugLevel lvl, const char *format, ...) {
	std::va_list ap;
	va_start(ap, format);
	SendTextVA(lvl, format, ap);
	va_end(ap);
}

} /* namespace System */
