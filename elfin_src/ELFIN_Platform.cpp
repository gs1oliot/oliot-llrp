/**
 * @file    ELFIN_Platform.cpp
 * @brief
 */

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <ctime>
#include <sys/time.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "ELFIN_Platform.h"

int _pLogType;

int ELFIN::Utils::LOG(enum ELogType pLogType, const char *fmt, ...) {
	int result = 0;
	// ALL > TRACE > DEBUG > INFO > WARN > ERROR > FATAL > OFF
	if (pLogType > _pLogType)
		return 0;
	std::time_t rawtime;
	std::tm* timeinfo;
	char buffer [80];
	std::time(&rawtime);
	timeinfo = std::localtime(&rawtime);

	std::strftime(buffer,80,"[%m/%d-%H:%M:%S",timeinfo);
	printf ("%s", buffer);

	struct timeval  tv;
	gettimeofday(&tv, NULL);
	printf(":%06ld]", tv.tv_usec);


	printf ("LLRP_");
	switch (pLogType) {
	case ELFIN::LOGTYPE_FATAL:
		printf ("FATAL: ");
		break;
	case ELFIN::LOGTYPE_ERROR:
		printf ("ERROR: ");
		break;
	case ELFIN::LOGTYPE_WARN:
		printf ("WARN: ");
		break;
	case ELFIN::LOGTYPE_INFO:
		printf ("INFO: ");
		break;
	case ELFIN::LOGTYPE_DEBUG:
		printf ("DEBUG: ");
		break;
	case ELFIN::LOGTYPE_TRACE:
		printf ("TRACE: ");
		break;
	default:
		return 0;
	}


	switch (pLogType) {
	case ELFIN::LOGTYPE_FATAL:
	case ELFIN::LOGTYPE_ERROR:
	case ELFIN::LOGTYPE_WARN:
	case ELFIN::LOGTYPE_INFO:
	case ELFIN::LOGTYPE_DEBUG:
	case ELFIN::LOGTYPE_TRACE:
		va_list args;
		va_start(args, fmt);
		result = vprintf(fmt,args);
		va_end(args);
		break;
	default:
		return 0;
	}

	return result;
}

void ELFIN::Utils::setLogLevel(enum ELogType __pLogtype) {
	_pLogType = __pLogtype;
}

void
ELFIN::Utils::printXMLMessage (LLRP::CMessage * pMessage)
{
//#define NO_XMLLOG
#ifndef NO_XMLLOG
	if (_pLogType < LOGTYPE_INFO)
		return;
	else {
		char                        aBuf[100*1024];

		/*
		 * Convert the message to an XML string.
		 * This fills the buffer with either the XML string
		 * or an error message. The return value could
		 * be checked.
		 */

		pMessage->toXMLString(aBuf, sizeof aBuf);

		/*
		 * Print the XML Text to the standard output.
		 */
		READER_LOG(LOGTYPE_INFO, "%s", aBuf);
	}
#endif
}

/*
 * ELFIN_Platform.cpp
 *
 *  Created on: Jul 18, 2013
 *      Author: shheo
 */

LLRP::CUTCTimestamp* ELFIN::Utils::getCurrentUTCTimestamp() {
	LLRP::CUTCTimestamp *pTmStamp = new LLRP::CUTCTimestamp();
	uint64_t microseconds = getCurrentTimeMilliseconds() * 1000;
	pTmStamp->setMicroseconds(microseconds);

	return pTmStamp;
}

uint64_t ELFIN::Utils::getCurrentTimeMilliseconds() {
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	uint64_t time_in_mill =
			(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond

	/*
	time_t timer;
	time(&timer);
	uint64_t time_milliseconds = timer * 1000;
	 */
	return time_in_mill;
}

LLRP::llrp_utf8v_t ELFIN::Utils::getUTF8Str(const char *pStr) {
	LLRP::llrp_utf8v_t pUTF8Str(strlen(pStr));
	for (int i = 0;i < pUTF8Str.m_nValue;i++) {
		pUTF8Str.m_pValue[i] = (LLRP::llrp_utf8_t) pStr[i];
	}
	return pUTF8Str;
}

uint64_t countedTime = 0;
uint64_t countStartTime = 0;
int counting = 0;
int tagCount = 0;
int antCount = 0;
char timeStringBuffer[100];

void ELFIN::Utils::startTimeCount(int _tagCount, int _antCount) {
	tagCount = _tagCount;
	antCount = _antCount;
	countStartTime = getCurrentTimeMilliseconds();
	counting = 1;
	timeStringBuffer[0] = 0;
}

void ELFIN::Utils::stopTimeCount() {
	if (counting != 0) {
		countedTime += getCurrentTimeMilliseconds() - countStartTime;
		countStartTime = 0;
		counting = 0;
	}
}

void ELFIN::Utils::printCountedTime(const char *str) {
	if (counting == 1) {
		countedTime += getCurrentTimeMilliseconds() - countStartTime;
		countStartTime = getCurrentTimeMilliseconds();
	}
	char buffer[50];
	sprintf (buffer, "%" PRIu64 " ", countedTime);
	if (strlen(timeStringBuffer) + strlen(buffer) < 90) {
		strcat (timeStringBuffer, buffer);
	}
	else if (strlen(timeStringBuffer) + strlen(buffer) < 100) {
		strcat (timeStringBuffer, "....");
	}
	else {

	}
}

void ELFIN::Utils::resetTimeCount() {
	printf ("Time: %d %d %s\n", tagCount, antCount, timeStringBuffer);
	timeStringBuffer[0] = 0;
	countedTime = 0;
	countStartTime = 0;
	counting = 0;
}

