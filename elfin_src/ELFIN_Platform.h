/**
 * @file    ELFIN_Platform.h
 * @brief
 */

#ifndef __LLRP_READER__LLRPREADER_PLATFORM_H__
#define __LLRP_READER__LLRPREADER_PLATFORM_H__

#include <string>
#include <stdio.h>
#include <stdint.h>

#include <ltkcpp.h>

#include "ELFIN_Defs.h"

namespace ELFIN {

class TagReportMap_Hash {
public:
	std::size_t operator()(LLRP::CTagReportData *k) const
	{
		LLRP::llrp_u8_t *mk = &(((LLRP::CEPC_96 *) k->getEPCParameter())->getEPC().m_aValue[0]);
		std::size_t seed = 0;
		for (int i = 0;i < 12;i++) {
			boost::hash_combine(seed, mk[i]);
		}

		return seed;
	}
};

class TagReportMap_Pred {
public:
	bool operator()(LLRP::CTagReportData *a, LLRP::CTagReportData *b) const
	{
		LLRP::llrp_u8_t *ma = &(((LLRP::CEPC_96 *) a->getEPCParameter())->getEPC().m_aValue[0]);
		LLRP::llrp_u8_t *mb = &(((LLRP::CEPC_96 *) b->getEPCParameter())->getEPC().m_aValue[0]);

		for (int i = 0;i < 12;i++) {
			if (ma[i] != mb[i]) {
				return false;
			}
		}
		return true;
	}
};

template <typename Container>
struct container_hash {
	std::size_t operator()(Container const& c) const {
		return boost::hash_range(c.begin(), c.end());
	}
};

typedef boost::unordered_map<int, LLRP::CAccessCommandOpSpec*> OPSpecsMap;
typedef boost::unordered_map<LLRP::llrp_u32_t, ELFIN::AccessOperation*> AOMap;
typedef std::pair<LLRP::llrp_u32_t, ELFIN::AccessOperation*> AOMap_t;
typedef boost::unordered_map<LLRP::llrp_u32_t, LLRP::CAccessSpec*> CAOMap;
typedef std::pair<LLRP::llrp_u32_t, LLRP::CAccessSpec*> CAOMap_t;
typedef boost::unordered_map<LLRP::llrp_u32_t, ELFIN::ReaderOperation*> ROMap;
typedef std::pair<LLRP::llrp_u32_t, ReaderOperation *> ROMap_t;
typedef boost::unordered_map<LLRP::llrp_u32_t, LLRP::CROSpec *> CROMap;
typedef std::pair<LLRP::llrp_u32_t, LLRP::CROSpec *> CROMap_t;
typedef boost::unordered_set<LLRP::CTagReportData *, TagReportMap_Hash, TagReportMap_Pred> TagReportSet;
typedef std::pair<enum LLRP::EStatusCode, LLRP::llrp_utf8v_t> Result_t;


/** @class Utils
 * @brief Some utils that are used in LLRP Wrapper. This includes printing log, getting timestamp, etc.
 */
class Utils{
public:
	/// Prints log with given LogType format
	static int LOG(enum ELogType logType, const char *fmt, ...);
	/// Set the log level of LLRP Wrapper
	static void setLogLevel(enum ELogType __pLogtype);
	/// Parse given CMessage and print it to screen in XML format
	static void printXMLMessage (LLRP::CMessage * pMessage);
	/// Returns current UTC timestamp in LLRP::CUTCTimeStamp type
	static LLRP::CUTCTimestamp* getCurrentUTCTimestamp();
	/// Returns current UTC time in millisecond
	static uint64_t getCurrentTimeMilliseconds();
	/// Converts given char array to LLRP::llrp_utf8v_t format and return it
	static LLRP::llrp_utf8v_t getUTF8Str (const char *pStr);
	/**@{*/
	/// Method for performance test. Not used currently.
	static void startTimeCount(int _tagCount, int _antCount);
	static void stopTimeCount();
	static void printCountedTime(const char *str);
	static void resetTimeCount();
	/**@}*/

};

#include "ReaderHandler/ReaderCapabilities.h"

}


#include "LLRPReader_Configurations.h"

#endif /* __LLRP_READER__LLRPREADER_PLATFORM_H__ */
