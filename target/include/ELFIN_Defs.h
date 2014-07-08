/*
 * elfin_defs.h
 *
 *  Created on: Jul 8, 2014
 *      Author: iot-team
 */

#ifndef ELFIN_DEFS_H_
#define ELFIN_DEFS_H_

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>


namespace ELFIN {
class AccessOperation;
class ReaderOperation;
class AbstractAntennaOperation;
class StubAntenna;
class StubGPIPort;
class StubGPOPort;
class StubTag;
}

namespace ELFIN
{
#define FALSE       0
#define TRUE        1

// ALL > TRACE > DEBUG > INFO > WARN > ERROR > FATAL > OFF
enum ELogType {
	LOGTYPE_OFF = 0,
	LOGTYPE_FATAL = 1,
	LOGTYPE_ERROR = 2,
	LOGTYPE_WARN = 3,
	LOGTYPE_INFO = 4,
	LOGTYPE_DEBUG = 5,
	LOGTYPE_TRACE = 6,
	LOGTYPE_ALL = 7
};


enum EGPITriggerType {
	GPI_StartTrigger = 1,
	GPI_StopTrigger = 2
};

enum EMemoryBankType {
	MB_Reserved = 0,
	MB_EPC = 1,
	MB_TID = 2,
	MB_User = 3
};

#define KEEPALIVE_FAIL_LIMIT 3

#define EVENT_NOTI_KIND_COUNT 12

#define CCONNECTION_BUFFER_SIZE 256u * 1024u


/* This may be supported only in C99. If this shows compile error,
 * substitute READER_LOG to ELFIN::ReaderLogger::LOG in whole source files
 */
#define READER_LOG(...) ELFIN::Utils::LOG(__VA_ARGS__)
#define READER_PRINTXMLMSG(pMsg) ELFIN::Utils::printXMLMessage(pMsg)




// <int, int, int, int> => <GPIPortNum, GPIEvent, ROSpecID, trigType>
typedef std::vector<boost::tuple<int, int, int, int> > GPIEventRegistry;
typedef boost::unordered_map<int, ELFIN::AbstractAntennaOperation*> AbsAntennaOpsMap;
typedef boost::unordered_map<int, StubAntenna *> AntennaMap;
typedef boost::unordered_map<int, StubGPIPort *> GPIPortMap;
typedef boost::unordered_map<int, StubGPOPort *> GPOPortMap;
typedef std::vector<uint8_t> MemoryBank;
typedef boost::unordered_map<MemoryBank, ELFIN::StubTag*> AntennaFoVMap;
typedef std::vector<ELFIN::StubTag *> TagVector;

};  /* namespace ELFIN */


#endif /* ELFIN_DEFS_H_ */
